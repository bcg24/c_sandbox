#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <portaudio.h>
#include <SDL2/SDL.h>
#define SAMPLE_RATE (44100)
#define PI (355.0/113.0)
#define NL (26)
#define NUM_NOTE (108)
// Envelope timing (in samples)
#define ATTACK_MS   1
#define RELEASE_MS  1
#define ATTACK_RATE  (1.0f / (SAMPLE_RATE * ATTACK_MS / 1000.0f))
#define RELEASE_RATE (1.0f / (SAMPLE_RATE * RELEASE_MS / 1000.0f))

/*
* Fun improvements TODO:
* Octave shifting
* Waveform visualizer
* Waveform editor
* RT STFT
* EQ ^
*/

typedef enum {
    WAVE_SINE,
    WAVE_SQUARE,
    WAVE_SAW,
    WAVE_TRIANGLE
} WaveType;

typedef struct {
    float *pfmg;                            // pointer to fm_global
    volatile short key_states[NL];          // volatile: "don't cache, always re-read"
    volatile short m_active[NL];            // Stores the active subset midi_nums -- precursor to shifting
                                            // For now, stays constant, but on e.g. Shift/CapsLk keystroke, in/decrements all elements by 12
    float phases[NL];                       // one phase per KEY, not per MIDI note
                                            // maintained by callback -- only gets advanced while respective key is active
                                            // free-runs to avoid clicks
    float envelopes[NL];                    // envelope level 0.0 to 1.0 per key
    volatile WaveType waveform;             // current waveform type
    unsigned long sample_count;             // global sample counter
} SynthState;

static int paSineCallback(const void*, void*, unsigned long,
                          const PaStreamCallbackTimeInfo*,
                          PaStreamCallbackFlags, void*);

void idx_freq(float *note_freq);

float note_freq(int m);

void init_mAct(short *m_active);

void shift_m(short *m_active, short dir);

float generate_sample(float phase, WaveType waveform);

const char* waveform_name(WaveType w);

int main(){

    int running = 1;
    float fm_global[NUM_NOTE];          // computed once at startup, read-only after
                                        // Lets just start from midi 0 
    SDL_Event event;
    PaStream *stream;
    SynthState data = {0};              // zero-initialize everything
    PaError err;
    PaStreamParameters outputParameters;

    printf("Mapping keyboard...\n");
    init_mAct(data.m_active);
    
    // key_states already zeroed by = {0}

    printf("Indexing frequencies...\n");
    idx_freq(fm_global);

    printf("Passing pointers...\n");
    data.pfmg = fm_global;
    data.waveform = WAVE_SINE;          // default waveform

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow("Synth - Press 1-4 for waveforms", 
                                       SDL_WINDOWPOS_CENTERED, 
                                       SDL_WINDOWPOS_CENTERED, 
                                       400, 100, 0);
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
                         8,                // small buffer for low latency
                         paClipOff,
                         paSineCallback,
                         &data);

    if( err != paNoError ) goto error;
    printf("Stream opened.\n");

    printf("Starting stream...\n");
    err = Pa_StartStream( stream );
    if( err != paNoError ) goto error;
    printf("Stream started...\n");
    printf("Waveform: %s\n", waveform_name(data.waveform));
    printf("\nControls:\n");
    printf("  ASDF JKL    - White keys (D4-C5)\n");
    printf("  QW R TU I P - Black keys\n");
    printf("  1-4         - Change waveform\n");
    printf("  Left/Right  - Shift octave down/up\n");
    printf("  Close window to quit\n\n");

    while(running){

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
                break;
            }
            
            if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                SDL_Keycode key = event.key.keysym.sym;
                
                // Check for waveform selection (1-4 keys) and octave shift (arrows)
                if (event.type == SDL_KEYDOWN) {
                    switch (key) {
                        case SDLK_1:
                            data.waveform = WAVE_SINE;
                            printf("Waveform: %s\n", waveform_name(data.waveform));
                            continue;
                        case SDLK_2:
                            data.waveform = WAVE_SQUARE;
                            printf("Waveform: %s\n", waveform_name(data.waveform));
                            continue;
                        case SDLK_3:
                            data.waveform = WAVE_SAW;
                            printf("Waveform: %s\n", waveform_name(data.waveform));
                            continue;
                        case SDLK_4:
                            data.waveform = WAVE_TRIANGLE;
                            printf("Waveform: %s\n", waveform_name(data.waveform));
                            continue;
                        case SDLK_LSHIFT:
                            // Shift down half an octave (check lower bound)
                            if (data.m_active['J' - 'A'] - 12 >= 21) {  // A0 = MIDI 21
                                shift_m(data.m_active, -1);
                                printf("Octave down: A = %.1f Hz\n", fm_global[data.m_active['J' - 'A']]);
                            } else {
                                printf("Already at lowest octave\n");
                            }
                            continue;
                        case SDLK_RSHIFT:
                            // Shift up an half an octave (check upper bound)
                            if (data.m_active['P' - 'A'] + 12 < NUM_NOTE) {  // Stay in array bounds
                                shift_m(data.m_active, 1);
                                printf("Octave up: A = %.1f Hz\n", fm_global[data.m_active['J' - 'A']]);
                            } else {
                                printf("Already at highest octave\n");
                            }
                            continue;
                    }
                }
                
                // Handle note keys
                const char *key_name = SDL_GetKeyName(key);
                if (key_name[0] >= 'A' && key_name[0] <= 'Z') {
                    int lex_idx = key_name[0] - 'A';
                    if (event.type == SDL_KEYDOWN && (data.key_states[lex_idx] != 1)) {
                        data.key_states[lex_idx] = 1;
                    } else if (event.type == SDL_KEYUP && (data.key_states[lex_idx] != 0)) {
                        data.key_states[lex_idx] = 0;
                    }
                }
            }
        }
    }

    printf("Stopping stream...\n");
    err = Pa_StopStream( stream );
    if( err != paNoError ) goto error;

    printf("Closing stream...\n");
    err = Pa_CloseStream( stream );
    if( err != paNoError ) goto error;

    printf("Terminating PortAudio...\n");
    err = Pa_Terminate();
    if( err != paNoError ) goto error;

    printf("Closing SDL window...\n");
    SDL_DestroyWindow(win);
    SDL_Quit();
    printf("Done.\n");
    return(EXIT_SUCCESS);

    error:
        printf("PortAudio error: %s\n", Pa_GetErrorText(err) );
        Pa_Terminate();
        SDL_Quit();
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
    if(m == 0)      // Zero sentinel value for inactive letters
        return(0.0f);
    float fm = powf(2.0f, (float)(m - 69) / 12.0f) * 440.0f;
    return(fm);
}

/*
* Generate sample from phase based on waveform type.
* Phase is in [0, 2π)
*/
float generate_sample(float phase, WaveType waveform) {
    switch (waveform) {
        case WAVE_SINE:
            return sinf(phase);
        
        case WAVE_SQUARE:
            // +1 for first half of cycle, -1 for second half
            return (phase < PI) ? 1.0f : -1.0f;
        
        case WAVE_SAW:
            // Ramp from -1 to +1 over the cycle
            return (phase / PI) - 1.0f;
        
        case WAVE_TRIANGLE:
            // Up from -1 to +1 in first half, down from +1 to -1 in second half
            if (phase < PI)
                return (2.0f * phase / PI) - 1.0f;
            else
                return 3.0f - (2.0f * phase / PI);
        
        default:
            return 0.0f;
    }
}

/*
* Return string name of waveform for display
*/
const char* waveform_name(WaveType w) {
    switch (w) {
        case WAVE_SINE:     return "Sine";
        case WAVE_SQUARE:   return "Square";
        case WAVE_SAW:      return "Sawtooth";
        case WAVE_TRIANGLE: return "Triangle";
        default:            return "Unknown";
    }
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
    WaveType waveform = data->waveform;     // read once to avoid tearing
    (void)inputBuffer;
    (void)timeInfo;
    (void)statusFlags;
    
    for (unsigned int i = 0; i < framesPerBuffer; i++) {
        float sample = 0.0f;
        int voice_count = 0;
        
        // Sum contributions from all keys (active or releasing)
        for (int k = 0; k < NL; k++) {
            short midi = data->m_active[k];
            float freq = data->pfmg[midi];
            
            if (freq == 0.0f)
                continue;  // unmapped key
            
            // Update envelope based on key state
            if (data->key_states[k]) {
                // Key held - ramp up
                data->envelopes[k] += ATTACK_RATE;
                if (data->envelopes[k] > 1.0f)
                    data->envelopes[k] = 1.0f;
            } else {
                // Key released - ramp down
                data->envelopes[k] -= RELEASE_RATE;
                if (data->envelopes[k] < 0.0f)
                    data->envelopes[k] = 0.0f;
            }
            
            // Contribute to output if envelope is non-zero
            if (data->envelopes[k] > 0.0f) {
                sample += data->envelopes[k] * generate_sample(data->phases[k], waveform);
                voice_count++;
                
                // Advance phase
                data->phases[k] += (2.0f * PI * freq) / SAMPLE_RATE;
                
                // Wrap to avoid float overflow
                if (data->phases[k] >= 2.0f * PI)
                    data->phases[k] -= 2.0f * PI;
            }
        }
        
        // Normalize to prevent clipping
        if (voice_count > 0)
            sample /= (float)voice_count;
        
        *out++ = sample;  // left
        *out++ = -sample;  // right (mono for now)
    }
    
    return 0;
}

/*
* Initialize the active MIDI number array.
* Unmapped keys get 0 (sentinel), which produces
* zero frequency and thus no audio contribution.
* Default octave n = 4, so A_n = A4 = MIDI 69.
*/
void init_mAct(short *m_active) {
    // Zero everything first
    for (int i = 0; i < NL; i++)
        m_active[i] = 0;
    
    // White keys (middle row)
    m_active['A' - 'A'] = 62;   // D4
    m_active['S' - 'A'] = 64;   // E4
    m_active['D' - 'A'] = 65;   // F4
    m_active['F' - 'A'] = 67;   // G4
    m_active['J' - 'A'] = 69;   // A4 (440 Hz)
    m_active['K' - 'A'] = 71;   // B4
    m_active['L' - 'A'] = 72;   // C5
    
    // Black keys (top row)
    m_active['Q' - 'A'] = 61;   // C#4
    m_active['W' - 'A'] = 63;   // D#4
    m_active['R' - 'A'] = 66;   // F#4
    m_active['T' - 'A'] = 68;   // G#4
    m_active['U' - 'A'] = 68;   // G#4 (duplicate)
    m_active['I' - 'A'] = 70;   // A#4
    m_active['P' - 'A'] = 73;   // C#5

    // Black keys (bottom row)

    m_active['Z' - 'A'] = 63;   // D#4
    m_active['C' - 'A'] = 66;   // F#4
    m_active['V' - 'A'] = 68;   // G#4
    m_active['N' - 'A'] = 68;   // G#4 (duplicate)
    m_active['M' - 'A'] = 70;   // A#4
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