/*
 * synth_wasapi.c - Real-time synthesizer using WASAPI Exclusive Mode
 * 
 * Compile with MSVC:
 *   cl synth_wasapi.c /link ole32.lib
 * 
 * Compile with MinGW:
 *   gcc synth_wasapi.c -o synth.exe -lole32 -lm
 */

#define COBJMACROS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

// GUIDs - define them ourselves to avoid linking issues
const CLSID CLSID_MMDeviceEnumerator = {0xbcde0395, 0xe52f, 0x467c, {0x8e,0x3d,0xc4,0x57,0x92,0x91,0x69,0x2e}};
const IID IID_IMMDeviceEnumerator    = {0xa95664d2, 0x9614, 0x4f35, {0xa7,0x46,0xde,0x8d,0xb6,0x36,0x17,0xe6}};
const IID IID_IAudioClient          = {0x1cb9ad4c, 0xdbfa, 0x4c32, {0xb1,0x78,0xc2,0xf5,0x68,0xa7,0x03,0xb2}};
const IID IID_IAudioRenderClient    = {0xf294acfc, 0x3146, 0x4483, {0xa7,0xbf,0xad,0xdc,0xa7,0xc2,0x60,0xe2}};

#define PI 3.14159265358979323846
#define NL (26)
#define NUM_NOTE (108)

typedef enum {
    WAVE_SINE,
    WAVE_SQUARE,
    WAVE_SAW,
    WAVE_TRIANGLE,
    WAVE_KICK,
    WAVE_SNARE,
    WAVE_HIHAT
} WaveType;

typedef struct {
    float *pfmg;
    volatile short key_states[NL];
    short prev_key_states[NL];              // for drum trigger edge detection
    volatile short m_active[NL];
    float phases[NL];
    float envelopes[NL];
    float drum_time[NL];                    // time since trigger (in samples) for drums
    volatile WaveType waveform;
    int sample_rate;
    float attack_rate;
    float release_rate;
    unsigned int noise_seed;                // for noise generation
} SynthState;

// Function prototypes
void idx_freq(float *fm);
float note_freq(int m);
void init_mAct(volatile short *m_active);
void shift_m(volatile short *m_active, short dir);
float generate_sample(float phase, WaveType waveform);
float generate_drum(WaveType drum, float t, float base_freq, float *phase,
                    int sample_rate, unsigned int *noise_seed);
float noise(unsigned int *seed);
int is_drum(WaveType w);
const char* waveform_name(WaveType w);
void fill_buffer(SynthState *data, float *buffer, UINT32 frames);

int key_pressed(int vk) {
    return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

int main() {
    HRESULT hr;
    IMMDeviceEnumerator *enumerator = NULL;
    IMMDevice *device = NULL;
    IAudioClient *audioClient = NULL;
    IAudioRenderClient *renderClient = NULL;
    WAVEFORMATEX *mixFormat = NULL;
    WAVEFORMATEX waveFormat = {0};
    UINT32 bufferFrameCount;
    REFERENCE_TIME requestedDuration = 100000;  // 10ms in 100ns units
    REFERENCE_TIME actualDuration;
    HANDLE bufferEvent = NULL;
    
    float fm_global[NUM_NOTE];
    SynthState data = {0};
    int running = 1;
    
    // Edge detection state
    int prev_waveform_keys[7] = {0};
    int prev_shift_left = 0, prev_shift_right = 0;

    printf("=== WASAPI Exclusive Mode Synth ===\n\n");

    // Initialize COM
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED); 
    if (FAILED(hr)) {
        printf("Failed to initialize COM: 0x%lx\n", hr);
        return 1;
    }

    // https://learn.microsoft.com/en-us/windows/win32/coreaudio/enumerating-audio-devices

    // Create device enumerator
    hr = CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,      // https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-cocreateinstance
                          &IID_IMMDeviceEnumerator, (void**)&enumerator);   // create a device enumerator. A device enumerator is an object with an IMMDeviceEnumerator interface
    if (FAILED(hr)) {                                                       // The CLSID associated with the data and code that will be used to create the object.
        printf("Failed to create device enumerator: 0x%lx\n", hr);
        goto cleanup;
    }

    // Get default audio endpoint
    hr = IMMDeviceEnumerator_GetDefaultAudioEndpoint(enumerator, eRender, eConsole, &device); // https://learn.microsoft.com/en-us/windows/win32/api/mmdeviceapi/nf-mmdeviceapi-immdeviceenumerator-getdefaultaudioendpoint
    if (FAILED(hr)) {
        printf("Failed to get default audio endpoint: 0x%lx\n", hr);
        goto cleanup;
    }

    // Activate audio client
    hr = IMMDevice_Activate(device, &IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&audioClient);
    if (FAILED(hr)) {
        printf("Failed to activate audio client: 0x%lx\n", hr);
        goto cleanup;
    }

    // Get mix format to see what the device supports
    hr = IAudioClient_GetMixFormat(audioClient, &mixFormat);    
    if (FAILED(hr)) {
        printf("Failed to get mix format: 0x%lx\n", hr);
        goto cleanup;
    }

    // Reduces
    // https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-getdeviceperiod

    printf("Device mix format:\n");
    printf("  Sample Rate: %lu Hz\n", mixFormat->nSamplesPerSec);
    printf("  Channels: %d\n", mixFormat->nChannels);
    printf("  Bits per sample: %d\n", mixFormat->wBitsPerSample);

    // Set up our desired format - float32 stereo
    waveFormat.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
    waveFormat.nChannels = 2;
    waveFormat.nSamplesPerSec = mixFormat->nSamplesPerSec;  // Use device's native rate
    waveFormat.wBitsPerSample = 32;
    waveFormat.nBlockAlign = waveFormat.nChannels * waveFormat.wBitsPerSample / 8;
    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
    waveFormat.cbSize = 0;

    // Create event for buffer notifications
    bufferEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!bufferEvent) {
        printf("Failed to create buffer event\n");
        goto cleanup;
    }

    // Try exclusive mode first
    hr = IAudioClient_Initialize(audioClient,                               //https://learn.microsoft.com/en-us/windows/desktop/api/Audioclient/nf-audioclient-iaudioclient-initialize
                                  AUDCLNT_SHAREMODE_EXCLUSIVE,
                                  AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
                                  requestedDuration,
                                  requestedDuration,
                                  &waveFormat,
                                  NULL);
    
    if (hr == AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED) {
        // Get the aligned buffer size
        hr = IAudioClient_GetBufferSize(audioClient, &bufferFrameCount);
        if (FAILED(hr)) goto cleanup;
        
        // Release and recreate
        IAudioClient_Release(audioClient);
        hr = IMMDevice_Activate(device, &IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&audioClient);
        if (FAILED(hr)) goto cleanup;
        
        requestedDuration = (REFERENCE_TIME)((10000000.0 * bufferFrameCount / waveFormat.nSamplesPerSec) + 0.5);
        
        hr = IAudioClient_Initialize(audioClient,
                                      AUDCLNT_SHAREMODE_EXCLUSIVE,
                                      AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
                                      requestedDuration,
                                      requestedDuration,
                                      &waveFormat,
                                      NULL);
    }
    
    if (hr == AUDCLNT_E_UNSUPPORTED_FORMAT) {
        printf("Exclusive mode not supported with this format, trying shared mode...\n");
        
        // Fall back to shared mode
        IAudioClient_Release(audioClient);
        hr = IMMDevice_Activate(device, &IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&audioClient);
        if (FAILED(hr)) goto cleanup;
        
        hr = IAudioClient_Initialize(audioClient,                               //https://learn.microsoft.com/en-us/windows/desktop/api/Audioclient/nf-audioclient-iaudioclient-initialize
                                      AUDCLNT_SHAREMODE_SHARED,
                                      AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
                                      requestedDuration,
                                      0,
                                      mixFormat,
                                      NULL);
        
        // Update our wave format to match
        waveFormat = *mixFormat;
    }
    
    if (FAILED(hr)) {
        printf("Failed to initialize audio client: 0x%lx\n", hr);
        goto cleanup;
    }

    // Set the event handle
    hr = IAudioClient_SetEventHandle(audioClient, bufferEvent);
    if (FAILED(hr)) {
        printf("Failed to set event handle: 0x%lx\n", hr);
        goto cleanup;
    }

    // Get buffer size
    hr = IAudioClient_GetBufferSize(audioClient, &bufferFrameCount);    //https://learn.microsoft.com/en-us/windows/desktop/api/Audioclient/nf-audioclient-iaudioclient-getbuffersize
    if (FAILED(hr)) {
        printf("Failed to get buffer size: 0x%lx\n", hr);
        goto cleanup;
    }

    // Get render client
    hr = IAudioClient_GetService(audioClient, &IID_IAudioRenderClient, (void**)&renderClient);
    if (FAILED(hr)) {
        printf("Failed to get render client: 0x%lx\n", hr);
        goto cleanup;
    }

    // Calculate actual latency
    actualDuration = (REFERENCE_TIME)((10000000.0 * bufferFrameCount / waveFormat.nSamplesPerSec) + 0.5);
    printf("\nAudio initialized:\n");
    printf("  Buffer size: %u frames\n", bufferFrameCount);
    printf("  Latency: %.2f ms\n", (double)actualDuration / 10000.0);
    printf("  Sample rate: %lu Hz\n", waveFormat.nSamplesPerSec);

    // Initialize synth state
    data.sample_rate = waveFormat.nSamplesPerSec;
    data.attack_rate = 1.0f / (data.sample_rate * 10 / 1000.0f);   // 10ms attack
    data.release_rate = 1.0f / (data.sample_rate * 50 / 1000.0f);  // 50ms release
    data.pfmg = fm_global;
    data.waveform = WAVE_SINE;
    data.noise_seed = 12345;  // seed for noise generator

    printf("\nMapping keyboard...\n");
    init_mAct(data.m_active);

    printf("Indexing frequencies...\n");
    idx_freq(fm_global);

    printf("Waveform: %s\n", waveform_name(data.waveform));
    printf("\nControls:\n");
    printf("  ASDF JKL    - White keys (D4-C5)\n");
    printf("  QW R TU I P - Black keys\n");
    printf("  1-4         - Synth waveforms (Sine/Square/Saw/Triangle)\n");
    printf("  5-7         - Drums (Kick/Snare/Hi-Hat)\n");
    printf("  L/R Shift   - Shift octave down/up\n");
    printf("  ESC         - Quit\n\n");

    // Start the stream
    hr = IAudioClient_Start(audioClient);
    if (FAILED(hr)) {
        printf("Failed to start audio client: 0x%lx\n", hr);
        goto cleanup;
    }

    printf("Playing... Press ESC to quit.\n\n");

    // Main loop
    while (running) {
        // Wait for buffer event or timeout
        DWORD waitResult = WaitForSingleObject(bufferEvent, 100);
        
        if (waitResult == WAIT_OBJECT_0) {
            UINT32 padding = 0;
            BYTE *bufferData;
            UINT32 framesToWrite;
            
            // For exclusive mode, we write the whole buffer
            // For shared mode, we need to check padding
            hr = IAudioClient_GetCurrentPadding(audioClient, &padding);
            if (FAILED(hr)) break;
            
            framesToWrite = bufferFrameCount - padding;
            
            if (framesToWrite > 0) {
                hr = IAudioRenderClient_GetBuffer(renderClient, framesToWrite, &bufferData);    // https://learn.microsoft.com/en-us/windows/desktop/api/Audioclient/nf-audioclient-iaudiorenderclient-getbuffer
                if (SUCCEEDED(hr)) {
                    fill_buffer(&data, (float*)bufferData, framesToWrite);
                    IAudioRenderClient_ReleaseBuffer(renderClient, framesToWrite, 0);   //https://learn.microsoft.com/en-us/windows/desktop/api/Audioclient/nf-audioclient-iaudiorenderclient-releasebuffer
                }
            }
        }
        
        // Poll keyboard
        if (key_pressed(VK_ESCAPE)) {
            running = 0;
            break;
        }

        // Note keys
        for (int i = 0; i < NL; i++) {
            data.key_states[i] = key_pressed('A' + i) ? 1 : 0;
        }

        // Waveform selection (1-4 synth, 5-7 drums)
        int waveform_keys[7] = {
            key_pressed('1'), key_pressed('2'),
            key_pressed('3'), key_pressed('4'),
            key_pressed('5'), key_pressed('6'),
            key_pressed('7')
        };
        
        for (int i = 0; i < 7; i++) {
            if (waveform_keys[i] && !prev_waveform_keys[i]) {
                data.waveform = (WaveType)i;
                printf("Waveform: %s\n", waveform_name(data.waveform));
            }
            prev_waveform_keys[i] = waveform_keys[i];
        }

        // Octave shifting
        int shift_left = key_pressed(VK_RSHIFT);
        int shift_right = key_pressed(VK_LSHIFT);

        if (shift_left && !prev_shift_left) {
            if (data.m_active['J' - 'A'] - 12 >= 21) {
                shift_m(data.m_active, -1);
                printf("Octave down: A = %.1f Hz\n", fm_global[data.m_active['J' - 'A']]);
            } else {
                printf("Already at lowest octave\n");
            }
        }
        if (shift_right && !prev_shift_right) {
            if (data.m_active['P' - 'A'] + 12 < NUM_NOTE) {
                shift_m(data.m_active, 1);
                printf("Octave up: A = %.1f Hz\n", fm_global[data.m_active['J' - 'A']]);
            } else {
                printf("Already at highest octave\n");
            }
        }
        prev_shift_left = shift_left;
        prev_shift_right = shift_right;
    }

    // Stop the stream
    IAudioClient_Stop(audioClient);

cleanup:
    if (bufferEvent) CloseHandle(bufferEvent);
    if (renderClient) IAudioRenderClient_Release(renderClient);
    if (audioClient) IAudioClient_Release(audioClient);
    if (mixFormat) CoTaskMemFree(mixFormat);
    if (device) IMMDevice_Release(device);
    if (enumerator) IMMDeviceEnumerator_Release(enumerator);
    CoUninitialize();

    printf("Done.\n");
    return 0;
}

void fill_buffer(SynthState *data, float *buffer, UINT32 frames) {
    WaveType waveform = data->waveform;
    int drum_mode = is_drum(waveform);
    
    for (UINT32 i = 0; i < frames; i++) {
        float sample = 0.0f;
        int voice_count = 0;

        for (int k = 0; k < NL; k++) {
            short midi = data->m_active[k];
            float freq = data->pfmg[midi];

            if (freq == 0.0f)
                continue;

            if (drum_mode) {
                // Drum mode: trigger-based
                // Check for rising edge (new key press)
                if (data->key_states[k] && !data->prev_key_states[k]) {
                    // Retrigger: reset time and phase
                    data->drum_time[k] = 0.0f;
                    data->phases[k] = 0.0f;
                    data->envelopes[k] = 1.0f;  // mark as active
                }
                data->prev_key_states[k] = data->key_states[k];
                
                // Generate drum sound if active
                if (data->envelopes[k] > 0.0f) {
                    float t = data->drum_time[k] / data->sample_rate;
                    float drum_sample = generate_drum(waveform, t, freq, 
                                                      &data->phases[k],
                                                      data->sample_rate,
                                                      &data->noise_seed);
                    sample += drum_sample;
                    voice_count++;
                    
                    data->drum_time[k] += 1.0f;
                    
                    // Deactivate after drum sound has decayed (500ms max)
                    if (t > 0.5f)
                        data->envelopes[k] = 0.0f;
                }
            } else {
                // Synth mode: gate-based with envelope
                if (data->key_states[k]) {
                    data->envelopes[k] += data->attack_rate;
                    if (data->envelopes[k] > 1.0f)
                        data->envelopes[k] = 1.0f;
                } else {
                    data->envelopes[k] -= data->release_rate;
                    if (data->envelopes[k] < 0.0f)
                        data->envelopes[k] = 0.0f;
                }

                if (data->envelopes[k] > 0.0f) {
                    sample += data->envelopes[k] * generate_sample(data->phases[k], waveform);
                    voice_count++;

                    data->phases[k] += (2.0f * PI * freq) / data->sample_rate;
                    if (data->phases[k] >= 2.0f * PI)
                        data->phases[k] -= 2.0f * PI;
                }
            }
        }

        if (voice_count > 0)
            sample /= (float)voice_count;

        // Stereo output
        *buffer++ = sample;  // left
        *buffer++ = sample;  // right
    }
}

void idx_freq(float *fm) {
    for (int i = 0; i < NUM_NOTE; i++)
        fm[i] = note_freq(i);
}

float note_freq(int m) {
    if (m == 0)
        return 0.0f;
    return powf(2.0f, (float)(m - 69) / 12.0f) * 440.0f;
}

// Fast noise generator (xorshift)
float noise(unsigned int *seed) {
    *seed ^= *seed << 13;
    *seed ^= *seed >> 17;
    *seed ^= *seed << 5;
    return (float)(*seed) / (float)UINT_MAX * 2.0f - 1.0f;
}

// Check if waveform is a drum type
int is_drum(WaveType w) {
    return w == WAVE_KICK || w == WAVE_SNARE || w == WAVE_HIHAT;
}

float generate_sample(float phase, WaveType waveform) {
    switch (waveform) {
        case WAVE_SINE:
            return sinf(phase);
        case WAVE_SQUARE:
            return (phase < PI) ? 1.0f : -1.0f;
        case WAVE_SAW:
            return (phase / PI) - 1.0f;
        case WAVE_TRIANGLE:
            if (phase < PI)
                return (2.0f * phase / PI) - 1.0f;
            else
                return 3.0f - (2.0f * phase / PI);
        default:
            return 0.0f;
    }
}

/*
 * Generate drum sample
 * t = time in seconds since trigger
 * base_freq = frequency from key mapping (used to vary pitch)
 */
float generate_drum(WaveType drum, float t, float base_freq, float *phase, 
                    int sample_rate, unsigned int *noise_seed) {
    float sample = 0.0f;
    float decay;
    
    switch (drum) {
        case WAVE_KICK: {
            // Pitch envelope: starts at ~150Hz, drops to ~50Hz
            // Decay: ~200ms
            float pitch_env = expf(-t * 15.0f);  // fast pitch drop
            float freq = 50.0f + 100.0f * pitch_env;
            
            // Amplitude envelope
            decay = expf(-t * 8.0f);
            
            // Phase accumulation with variable frequency
            *phase += (2.0f * PI * freq) / sample_rate;
            if (*phase >= 2.0f * PI) *phase -= 2.0f * PI;
            
            // Add a click at the start
            float click = (t < 0.005f) ? (1.0f - t / 0.005f) * 0.5f : 0.0f;
            
            sample = (sinf(*phase) + click) * decay;
            break;
        }
        
        case WAVE_SNARE: {
            // Tone component (~200Hz) + noise
            // Decay: ~150ms
            decay = expf(-t * 12.0f);
            float noise_decay = expf(-t * 20.0f);
            
            *phase += (2.0f * PI * 200.0f) / sample_rate;
            if (*phase >= 2.0f * PI) *phase -= 2.0f * PI;
            
            float tone = sinf(*phase) * 0.4f;
            float nse = noise(noise_seed) * 0.6f * noise_decay;
            
            sample = (tone + nse) * decay;
            break;
        }
        
        case WAVE_HIHAT: {
            // Pure noise, very short decay
            // Closed hat: ~50ms, open hat could be longer
            decay = expf(-t * 40.0f);  // very fast decay
            
            // Band-limited noise (simple high-pass by differencing)
            float n1 = noise(noise_seed);
            float n2 = noise(noise_seed);
            float hp_noise = (n1 - n2) * 0.5f + n1 * 0.5f;
            
            sample = hp_noise * decay;
            break;
        }
        
        default:
            break;
    }
    
    return sample;
}

const char* waveform_name(WaveType w) {
    switch (w) {
        case WAVE_SINE:     return "Sine";
        case WAVE_SQUARE:   return "Square";
        case WAVE_SAW:      return "Sawtooth";
        case WAVE_TRIANGLE: return "Triangle";
        case WAVE_KICK:     return "Kick Drum";
        case WAVE_SNARE:    return "Snare Drum";
        case WAVE_HIHAT:    return "Hi-Hat";
        default:            return "Unknown";
    }
}

void init_mAct(volatile short *m_active) {
    for (int i = 0; i < NL; i++)
        m_active[i] = 0;

    // White keys
    m_active['A' - 'A'] = 62;   // D4
    m_active['S' - 'A'] = 64;   // E4
    m_active['D' - 'A'] = 65;   // F4
    m_active['F' - 'A'] = 67;   // G4
    m_active['J' - 'A'] = 69;   // A4 (440 Hz)
    m_active['K' - 'A'] = 71;   // B4
    m_active['L' - 'A'] = 72;   // C5

    // Black keys
    m_active['Q' - 'A'] = 61;   // C#4
    m_active['W' - 'A'] = 63;   // D#4
    m_active['R' - 'A'] = 66;   // F#4
    m_active['T' - 'A'] = 68;   // G#4
    m_active['U' - 'A'] = 68;   // G#4
    m_active['I' - 'A'] = 70;   // A#4
    m_active['P' - 'A'] = 73;   // C#5
}

void shift_m(volatile short *m_active, short dir) {
    for (int i = 0; i < NL; i++) {
        if (m_active[i] != 0)
            m_active[i] += dir * 12;
    }
}
