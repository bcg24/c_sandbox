#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <portaudio.h>
#include <SDL2/SDL.h>
#define SAMPLE_RATE (44100)
#define PI (355.0/113.0)
#define NUM_SECONDS (1)
#define NL (26)
#define NUM_NOTE (108)

/*
* Fun improvements TODO:
* Octave shifting
* Waveform visualizer
* Waveform editor
* RT STFT
* EQ ^
*/

typedef struct {
    float *pfmg;                            // pointer to fm_global
    volatile short key_states[NL];          // volatile: "don't cache, always re-read"
    volatile short m_active[NL];            // Stores the active subset midi_nums -- precursor to shifting
                                            // For now, stays constant, but on e.g. Shift/CapsLk keystroke, in/decrements all elements by 12
    float phases[NL];                       // one phase per KEY, not per MIDI note
                                            // maintained by callback -- only gets advanced while respective key is active
                                            // resets to 0 on key release
    unsigned long sample_count;             // global sample counter
} SynthState;

static int paSineCallback(const void*, void*, unsigned long,
                          const PaStreamCallbackTimeInfo*,
                          PaStreamCallbackFlags, void*);

void idx_freq(float *note_freq);

float note_freq(int m);

void init_mAct(short *m_active);

void shift_m(short *m_active, short dir);

main(){

    int running = 1;
    unsigned long k = 0, k_start[NL];
    float fm_global[NUM_NOTE];          // computed once at startup, read-only after
                                        // Lets just start from midi 0 
    SDL_Event event;
    PaStream *stream;
    SynthState data;
    PaError err;
    PaStreamParameters outputParameters;

    printf("Mapping keyboard...\n");
    init_mAct(data.m_active);
    for(short *pks = &(data.key_states[0]); pks < &(data.key_states[NL]); pks++) // initialize with zeros
        *pks = 0;

    printf("Indexing frequencies...");
    idx_freq(fm_global);

    printf("Passing pointers...\n");
    data.pfmg = fm_global;



    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow("Key Test", 
                                       SDL_WINDOWPOS_CENTERED, 
                                       SDL_WINDOWPOS_CENTERED, 
                                       200, 200, 0);
    if (!win) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

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
                         64,
                         paClipOff,
                         paSineCallback,
                         &data);

    if( err != paNoError ) goto error;
    printf("Stream opened.\n");

    printf("Starting stream...\n");
    err = Pa_StartStream( stream );
    if( err != paNoError ) goto error;
    printf("Stream started...\n");

    while(running){

        while (SDL_PollEvent(&event)) {
            const char *key_name = SDL_GetKeyName(event.key.keysym.sym);
            if (key_name[0] >= 'A' && key_name[0] <= 'Z') {
                int lex_idx = key_name[0] - 'A';
                if(event.type == SDL_KEYDOWN && (data.key_states[lex_idx] != 1)){
                    data.key_states[lex_idx] = 1;
                    break;
                } else if(event.type == SDL_KEYUP && (data.key_states[lex_idx] != 0)){
                    data.key_states[lex_idx] = 0;
                    break;
                } else if(event.type == SDL_QUIT){
                    running = 0;
                    break;
                }  
            }
        }
        SDL_Delay(1);

        k++;
        
    }

    printf("Stopping stream...\n");
    err = Pa_StopStream( stream );
    if( err != paNoError ) goto error;

    printf("Closing stream...\n");
    err = Pa_CloseStream( stream );
    if( err != paNoError ) goto error;

    printf("Terminating stream...\n");
    err = Pa_Terminate();
    if( err != paNoError ) goto error;

    printf("Closing SDL window...\n");
    SDL_DestroyWindow(win);
    SDL_Quit();
    printf("Done.\n");
    return(EXIT_SUCCESS);

    error:
        printf("PortAudio error: %s\n", Pa_GetErrorText(err) );
        return(EXIT_FAILURE);
}

/*
* Function to create an array of frequencies where
* the index corresponds to the frequency's MIDI number
*/
void idx_freq(float *fm){
    for(int i = 0; i < NUM_NOTE; i++)
        fm[i] = note_freq(i);
}

/*
* Function to convert MIDI number to frequency
*/
float
note_freq(int m){
    if(m == 0.0)   // We'll put a zero sentinel value for the innactive letters
                    // i.e. letters that don't have a MIDI number
        return(0.0);      // This should make sinf(phase[i]) = sinf(0) = 0
                        // This should make it have no contribution to the 
                        // waveform even if those keys are pressed
    float fm  =  pow(2.0, (float)(m - 69)/12)*440;
    return(fm);
}

/* This routine will be called by the PortAudio engine when audio is needed.
 * It may called at interrupt level on some machines so don't do anything
 * that could mess up the system like calling malloc() or free().
*/ 
static int paSineCallback(const void *inputBuffer, void *outputBuffer, 
                          unsigned long framesPerBuffer,               
                          const PaStreamCallbackTimeInfo* timeInfo,    
                          PaStreamCallbackFlags statusFlags,           
                          void *userData)
{
    SynthState *data = (SynthState*)userData;
    float *out = (float*)outputBuffer;
    (void)inputBuffer;
    
    for (unsigned int i = 0; i < framesPerBuffer; i++) {
        float sample = 0.0f;
        int active_count = 0;
        
        // Sum contributions from all active keys
        for (int k = 0; k < NL; k++) {
            if (data->key_states[k]) {  // if the key state is active
                short midi = data->m_active[k];
                float freq = data->pfmg[midi];
                
                if (freq != 0.0f) {
                    sample += sinf(data->phases[k]);
                    
                    // Advance phase
                    data->phases[k] += (2.0f * PI * freq) / SAMPLE_RATE;
                    
                    // Wrap to avoid float overflow
                    if (data->phases[k] >= 2.0f * PI)
                        data->phases[k] -= 2.0f * PI;
                    
                    active_count++;
                }
            } else {
                // Don't reset - let phase free-run
                // This way, re-pressing a key continues smoothly
            }
        }
        
        // Normalize to prevent clipping
        if (active_count > 0)
            sample /= (float)active_count;
        
        *out++ = sample;  // left
        *out++ = sample;  // right (mono for now)
    }
    
    return 0;
}

/*
* The key states are organized lexicographically. 
* Put the MIDI number indicated on the right-hand
* side of the assingment operator at the corresponding
* lexicographical index of the character constant indicated
* on the lefthand sie of the assignment operator.
* When result of the MIDI numbers frequency transformation 
* is taken in an element-wise vector product with the key_states,
* the resulting vector can be taken as an argument to the
* Fourier construction

* P = 73    // C#_(n+1)
* L = 72    // C_(n+1)
* K = 71    // B_n
* I = 70    // A#_n
* J = 69    // A_n
* T, U = 68 // G#_n
* F = 67    // G_n
* R = 66    // F#_n
* D = 65    // F_n
* S = 64    // E_n
* W = 63    // D#_n
* A = 62    // D_n
* Q = 61    // C#_n
* Where n is the octave (initialized at n = 4)
*
* We want to intialize the array of active
* MIDI numbers with all zeros, except for the 
*
* Unmapped keys get 0 (sentinel), which produces
* zero frequency and thus no audio contribution.
* Default octave n = 4, so A_n = A4 = MIDI 69.
*/
void init_mAct(short *m_active) {
    // Zero everything first
    for (int i = 0; i < NL; i++)
        m_active[i] = 0;
    
    // White keys (bottom row)
    m_active['A' - 'A'] = 62;   // D4
    m_active['S' - 'A'] = 64;   // E4
    m_active['D' - 'A'] = 65;   // F4
    m_active['F' - 'A'] = 67;   // G4
    m_active['J' - 'A'] = 69;   // A4
    m_active['K' - 'A'] = 71;   // B4
    m_active['L' - 'A'] = 72;   // C5
    
    // Black keys (top row)
    m_active['Q' - 'A'] = 61;   // C#4
    m_active['W' - 'A'] = 63;   // D#4
    m_active['R' - 'A'] = 66;   // F#4
    m_active['T' - 'A'] = 68;   // G#4
    m_active['U' - 'A'] = 68;   // G#4
    m_active['I' - 'A'] = 70;   // A#4
    m_active['P' - 'A'] = 73;   // C#5
}


/*
* Function to in/decrement the active MIDI
* numbers by 12. Skips if value is 0
*/
void shift_m(short *m_active, short dir){
    for (int i = 0; i < NL; i++) {
        if (m_active[i] != 0)
            m_active[i] += dir * 12;
    }
}