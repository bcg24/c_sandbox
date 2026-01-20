#pragma comment(lib, "user32.lib")   // For GetAsyncKeyState
#pragma comment(lib, "ole32.lib")    // For CoInitializeEx, CoCreateInstance, CoUninitialize, CoTaskMemFree

#define WIN32_LEAN_AND_MEAN
#define BINS (16)
#define PI (3.14159265358979323846)

#include <stdio.h>
#include <stdlib.h>
#include <complex>
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <cblas.h>
#include <math.h>

using dcomplex = std::complex<double>;

// __uuidof works in MSVC C++
// No need to declare GUIDs manually

int key_pressed(int vk) {
    return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

void DFT(dcomplex *X, float *x, dcomplex *pW, int frameCount);
void twiddleTable(dcomplex *pW, int nSamplesPerSec, int frameCount);
void printHistogramHorizontal(double *bins, int numBins, int maxWidth);

int main(){
    HRESULT hr;
    
    IMMDeviceEnumerator *pEnumerator = NULL;
    IMMDevice *pEndpoint = NULL;
    IAudioClient *pAudioClient = NULL;
    IAudioCaptureClient *pCaptureClient = NULL;
    WAVEFORMATEX *pDeviceFormat = NULL;
    UINT32 bufferFrameCount;
    REFERENCE_TIME requestedDuration = 300000;
    HANDLE bufferEvent = NULL;
    constexpr int frameCount = BINS * 100;
    float samples[frameCount], overflow[frameCount], *pSample, *pRead, *pWrite;
    dcomplex *W = new dcomplex[frameCount * frameCount];  // Heap allocation - too big for stack

    // Initialize COM
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED); 
    if (FAILED(hr)) {
        fprintf(stderr, "Failed to initialize COM: 0x%lx\n", hr);
        return EXIT_FAILURE;
    }

    // Create device enumerator - C++ style COM calls
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
                          __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    if (FAILED(hr)) {
        fprintf(stderr, "Failed to create device enumerator: 0x%lx\n", hr);
        goto cleanup;
    }

    // C++ COM uses -> method calls, not C macros
    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &pEndpoint);
    if (FAILED(hr)) {
        fprintf(stderr, "Failed to get default audio endpoint: 0x%lx\n", hr);
        goto cleanup;
    }

    hr = pEndpoint->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pAudioClient);
    if (FAILED(hr)) {
        fprintf(stderr, "Failed to activate audio client: 0x%lx\n", hr);
        goto cleanup;
    }

    hr = pAudioClient->GetMixFormat(&pDeviceFormat);
    if (FAILED(hr)) {
        printf("Failed to get mix format: 0x%lx\n", hr);
        goto cleanup;
    }

    printf("Capturing in device mix format:\n");
    printf("  Sample Rate: %lu Hz\n", pDeviceFormat->nSamplesPerSec);
    printf("  Channels: %d\n", pDeviceFormat->nChannels);
    printf("  Bits per sample: %d\n", pDeviceFormat->wBitsPerSample);

    hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
                                AUDCLNT_STREAMFLAGS_LOOPBACK | AUDCLNT_STREAMFLAGS_EVENTCALLBACK,  // Add this flag
                                requestedDuration,
                                0,
                                pDeviceFormat,
                                NULL);

    bufferEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!bufferEvent) {
        printf("Failed to create buffer event\n");
        goto cleanup;
    }                               

    if (hr == AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED) {
        hr = pAudioClient->GetBufferSize(&bufferFrameCount);
        if (FAILED(hr)) goto cleanup;
        
        pAudioClient->Release();
        hr = pEndpoint->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pAudioClient);
        if (FAILED(hr)) goto cleanup;

        requestedDuration = (REFERENCE_TIME)((10000000.0 * bufferFrameCount / pDeviceFormat->nSamplesPerSec) + 0.5);

        hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
                              AUDCLNT_STREAMFLAGS_LOOPBACK | AUDCLNT_STREAMFLAGS_EVENTCALLBACK,  // Here too
                              requestedDuration,
                              0,
                              pDeviceFormat,
                              NULL);
    }

    hr = pAudioClient->SetEventHandle(bufferEvent);
    if (FAILED(hr)) {
        printf("Failed to set event handle: 0x%lx\n", hr);
        goto cleanup;
    }

    hr = pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&pCaptureClient);
    if (FAILED(hr)) {
        printf("Failed to get capture client: 0x%lx\n", hr);
        goto cleanup;
    }

    twiddleTable(W, pDeviceFormat->nSamplesPerSec, frameCount);
                                    
    hr = pAudioClient->Start();
    if (FAILED(hr)) {
        printf("Failed to start audio client: 0x%lx\n", hr);
        goto cleanup;
    }

    printf("Playing... Press ESC to quit.\n\n");

    pSample = samples;
    pRead = pWrite = overflow;
    
    {
        int running = 1;
        while(running){
            DWORD waitResult = WaitForSingleObject(bufferEvent, 100);

            if(waitResult == WAIT_OBJECT_0){
                UINT32 numFramesAvailable;
                BYTE *pData;
                DWORD flags;

                hr = pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL);

                int numSampRem = &samples[frameCount] - pSample;
                int readToEnd = &overflow[frameCount] - pRead;
                int startToWrite = pWrite - overflow;
                int numUnrdOvflw = (pWrite < pRead) ? (readToEnd + startToWrite) : (pWrite - pRead);

                // === PHASE 1: Drain overflow buffer into sample array ===
                if (pWrite != pRead) {
                    if (pWrite < pRead) {
                        if (numSampRem <= readToEnd) {
                            memcpy(pSample, pRead, numSampRem * sizeof(float));
                            pRead += numSampRem;
                            pSample += numSampRem;
                        } else if (numSampRem <= numUnrdOvflw) {
                            memcpy(pSample, pRead, readToEnd * sizeof(float));
                            pSample += readToEnd;
                            pRead = overflow;
                            memcpy(pSample, pRead, (numSampRem - readToEnd) * sizeof(float));
                            pRead += (numSampRem - readToEnd);
                            pSample += (numSampRem - readToEnd);
                        } else {
                            memcpy(pSample, pRead, readToEnd * sizeof(float));
                            pSample += readToEnd;
                            pRead = overflow;
                            memcpy(pSample, pRead, startToWrite * sizeof(float));
                            pSample += startToWrite;
                            pRead = pWrite;
                        }
                    } else {
                        if (numSampRem <= numUnrdOvflw) {
                            memcpy(pSample, pRead, numSampRem * sizeof(float));
                            pRead += numSampRem;
                            pSample += numSampRem;
                        } else {
                            memcpy(pSample, pRead, numUnrdOvflw * sizeof(float));
                            pSample += numUnrdOvflw;
                            pRead = pWrite;
                        }
                    }
                }

                // === PHASE 2: Handle incoming pData ===
                numSampRem = &samples[frameCount] - pSample;
                int writeToEnd = &overflow[frameCount] - pWrite;

                if (numSampRem <= (int)numFramesAvailable) {
                    memcpy(pSample, pData, numSampRem * sizeof(float));
                    pSample = samples;
                    
                    // Trigger FFT here
                    dcomplex *X = new dcomplex[frameCount];
                    DFT(X, samples, W, frameCount);
                    int n = frameCount / BINS;
                    double sumX_bin[BINS/2];
                    for (int ii = 0; ii < BINS/2; ii++) {
                        if(ii == 0){
                            sumX_bin[ii] = 0;
                            continue;
                        }
                        sumX_bin[ii] = cblas_dzasum(n, reinterpret_cast<double*>(&X[ii * n]), 1);
                    }
                    delete[] X;

                    printHistogramHorizontal(sumX_bin, BINS/2, 15);
                    
                    int remainder = numFramesAvailable - numSampRem;
                    if (remainder > 0) {
                        if (remainder >= writeToEnd) {
                            memcpy(pWrite, &pData[numSampRem * sizeof(float)], writeToEnd * sizeof(float));
                            pWrite = overflow;
                            memcpy(pWrite, &pData[(numSampRem + writeToEnd) * sizeof(float)], (remainder - writeToEnd) * sizeof(float));
                            pWrite += (remainder - writeToEnd);
                        } else {
                            memcpy(pWrite, &pData[numSampRem * sizeof(float)], remainder * sizeof(float));
                            pWrite += remainder;
                        }
                    }
                } else {
                    memcpy(pSample, pData, numFramesAvailable * sizeof(float));
                    pSample += numFramesAvailable;
                }
                if (SUCCEEDED(hr) && numFramesAvailable > 0) {
                    pCaptureClient->ReleaseBuffer(numFramesAvailable);
                }
            }

            if (key_pressed(VK_ESCAPE)) {
                running = 0;
                break;
            }
        }
    }

cleanup:
    delete[] W;
    if (bufferEvent) CloseHandle(bufferEvent);
    if (pCaptureClient) pCaptureClient->Release();
    if (pAudioClient) pAudioClient->Release();
    if (pDeviceFormat) CoTaskMemFree(pDeviceFormat);
    if (pEndpoint) pEndpoint->Release();
    if (pEnumerator) pEnumerator->Release();
    CoUninitialize();

    printf("Done.\n");
    return EXIT_SUCCESS;
}

void DFT(dcomplex *X, float *x, dcomplex *pW, int frameCount){
    dcomplex alpha(1.0, 0.0);
    dcomplex beta(0.0, 0.0);
    dcomplex *ix = new dcomplex[frameCount];
    
    for(int ii = 0; ii < frameCount; ii++){
        ix[ii] = dcomplex(x[ii], 0.0);
    }
    
    cblas_zgemv(CblasRowMajor, CblasNoTrans, 
                frameCount, frameCount,
                reinterpret_cast<double*>(&alpha),
                reinterpret_cast<double*>(pW), frameCount,
                reinterpret_cast<double*>(ix), 1,
                reinterpret_cast<double*>(&beta),
                reinterpret_cast<double*>(X), 1);
    
    delete[] ix;
}

void twiddleTable(dcomplex *pW, int nSamplesPerSec, int frameCount){
    for(int ii = 0; ii < frameCount; ii++){
        for(int jj = 0; jj < frameCount; jj++){
            double angle = (-2.0 * PI * ii * jj) / frameCount;
            pW[ii * frameCount + jj] = dcomplex(cos(angle), sin(angle));
        }
    }
}

void printHistogramHorizontal(double *bins, int numBins, int maxWidth = 50) {
    double maxVal = 0;
    for (int i = 0; i < numBins; i++) {
        if (bins[i] > maxVal) maxVal = bins[i];
    }
    if (maxVal == 0) maxVal = 1;
    
    for (int i = 0; i < numBins; i++) {
        int barLen = (int)((bins[i] / maxVal) * maxWidth);
        printf("%2d |", i);
        for (int j = 0; j < barLen; j++) {
            printf("#");
        }
        printf("\n");
    }
    printf("\n");
}