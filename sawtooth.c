#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "portaudio.h"
#define SAMPLE_RATE (44100)
#define NUM_SECONDS (1)

static int paSquareCallback(const void*, void*, unsigned long,
                          const PaStreamCallbackTimeInfo*,
                          PaStreamCallbackFlags, void*);

static int paSineCallback(const void*, void*, unsigned long,
                          const PaStreamCallbackTimeInfo*,
                          PaStreamCallbackFlags, void*);

typedef struct
{
    float left_phase;
    float right_phase;
}   
paTestData; // so instead of being type struct paTestData, its just paTestData

static paTestData data = {0.0f, 0.0f};

int
main(){
    PaStream *stream;
    PaError err;
    PaStreamParameters outputParameters;

    printf("Initializing PortAudio...\n");
    err = Pa_Initialize();
    if( err != paNoError ) goto error;
    printf("PortAudio initialized.\n");

    outputParameters.channelCount = 2;
    outputParameters.sampleFormat = paFloat32;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    for(int i = 0; i < Pa_GetDeviceCount(); i++){
        outputParameters.device = i;
        outputParameters.suggestedLatency = Pa_GetDeviceInfo(i)->defaultLowOutputLatency;
        if(Pa_GetDeviceInfo(i)->maxOutputChannels >= 2 && Pa_IsFormatSupported(NULL, &outputParameters, SAMPLE_RATE) == 0){
            printf("Using device %d: %s\n", i, Pa_GetDeviceInfo(i)->name);
            break;
        }
    }

    printf("Opening stream on device %d...\n", outputParameters.device);
    err = Pa_OpenStream( &stream,
                         NULL,              // no input
                         &outputParameters,
                         SAMPLE_RATE,
                         256,
                         paClipOff,
                         paSineCallback,
                         &data );
    if( err != paNoError ) goto error;
    printf("Stream opened.\n");

    printf("Starting stream...\n");
    err = Pa_StartStream( stream );
    if( err != paNoError ) goto error;
    printf("Stream started. Playing for %d seconds...\n", NUM_SECONDS);

    Pa_Sleep(NUM_SECONDS*1000); 

    printf("Stopping stream...\n");
    err = Pa_StopStream( stream );
    if( err != paNoError ) goto error;

    printf("Closing stream...\n");
    err = Pa_CloseStream( stream );
    if( err != paNoError ) goto error;

    printf("Terminating stream...\n");
    err = Pa_Terminate();
    if( err != paNoError ) goto error;

    printf("Done.\n");
    return(EXIT_SUCCESS);

    error:
        printf("PortAudio error: %s\n", Pa_GetErrorText(err) );
        return(EXIT_FAILURE);
}

/* This routine will be called by the PortAudio engine when audio is needed.
 * It may called at interrupt level on some machines so don't do anything
 * that could mess up the system like calling malloc() or free().
*/ 

static int paSquareCallback( const void *inputBuffer, void *outputBuffer, 
                           unsigned long framesPerBuffer,               
                           const PaStreamCallbackTimeInfo* timeInfo,    
                           PaStreamCallbackFlags statusFlags,           
                           void *userData )                             // pointer to the data you want
                                                                        // to sneak in
{
    /* Cast data passed through stream to our structure. */
    
    paTestData *data = (paTestData*)userData;                           // casts the pointer to userdata
    float *out = (float*)outputBuffer;                                  // to type pointer to paTestData
                                                                        // and rename to data
    unsigned int i;                                                     // cast pointer type to pointer
                                                                        // to float
    (void) inputBuffer; /* Prevent unused variable warning. */          // casts input buffer to void
    
    for( i=0; i<framesPerBuffer; i++ )                                  
    {
        *out++ = data->left_phase;  /* left */                          // read from member of struct via pointer, 
                                                                        // write to address via pointer, post increment pointer
                                                                        // so assumes that we allocated at least sizeof(float[framesPerBuffer])
                                                                        // to hold one stereo channel
        *out++ = data->right_phase;  /* right */
        /* Generate simple sawtooth phaser that ranges between -1.0 and 1.0. */
        if(i%2) data->left_phase += 0.01f;
        /* When signal reaches top, drop back down. */
        if( data->left_phase >= 1.0f ) {if(i%2) data->left_phase -= 2.0f;};
        /* higher pitch so we can distinguish left and right. */
        if(i%2) data->right_phase += 0.03f;
        if( data->right_phase >= 1.0f ) {if(i%2) data->right_phase -= 2.0f;};
    }
    return 0;
}

static int paSineCallback( const void *inputBuffer, void *outputBuffer, 
                           unsigned long framesPerBuffer,               
                           const PaStreamCallbackTimeInfo* timeInfo,    
                           PaStreamCallbackFlags statusFlags,           
                           void *userData )                             // pointer to the data you want
                                                                        // to sneak in
{
    /* Cast data passed through stream to our structure. */
    
    paTestData *data = (paTestData*)userData;                           // casts the pointer to userdata
    float *out = (float*)outputBuffer;                                  // to type pointer to paTestData
                                                                        // and rename to data
    unsigned int i;                                                     // cast pointer type to pointer
                                                                        // to float
    (void) inputBuffer; /* Prevent unused variable warning. */          // casts input buffer to void
    
    for( i=0; i<framesPerBuffer; i++ )                                  
    {
        *out++ = data->left_phase;  /* left */                          // read from member of struct via pointer, 
                                                                        // write to address via pointer, post increment pointer
                                                                        // so assumes that we allocated at least sizeof(float[framesPerBuffer])
                                                                        // to hold one stereo channel
        *out++ = data->right_phase;  /* right */
        /* Generate simple sawtooth phaser that ranges between -1.0 and 1.0. */
        if((i%2) == 1) data->left_phase += 0.01f;
        /* When signal reaches top, drop back down. */
        if( data->left_phase >= 1.0f ) data->left_phase -= 2.0f;
        /* higher pitch so we can distinguish left and right. */
        if((i%2) == 1) data->right_phase += 0.03f;
        if( data->right_phase >= 1.0f ) data->right_phase -= 2.0f;
    }
    return 0;
}