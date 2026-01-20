#define COBJMACROS
#define WIN32_LEAN_AND_MEAN
#define PI 3.14159265358979323846

#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <blas.h>

// __uuidof only works with MSVC
const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);    // GUID value attached as attributes to the MMDeviceEnumerator class object
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);      // GUID value attached as attributes to the IMMDeviceEnumerator interface
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);
// IID - interface identifier, is a type of GUID

int key_pressed(int vk) {
    return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

main(){
    HRESULT hr;
    /*
    Generally, the multi media interfaces let you discover 
    enpoint devices and query their capability.
    */
    
    /*
    First we initialize a null pointer to the device enumerator
    sub-interface. This will eventually point us to a list of all 
    the availible devices
    */
    IMMDeviceEnumerator *pEnumerator = NULL;        // Pointer to the IMMDeviceEnumerator interface of a device-enumerator object.
                                                    // Through the IMMDeviceEnumerator interface, the client can obtain references 
                                                    // to the other interfaces in the MMDevice API. 
    IMMDevice *pEndpoint = NULL;                    // Pointer to a representation an audio device, initiallized with null
    IAudioClient *pAudioClient = NULL;              //
    IAudioCaptureClient *pCaptureClient = NULL;     // Where we get the actual data from
    WAVEFORMATEX *pDeviceFormat = NULL;             // Actually a pointer to a WAVEFORMEXTENSIBLE structure -- stores format
    UINT32 bufferFrameCount;                        //
    REFERENCE_TIME requestedDuration = 300000;      //
    HANDLE bufferEvent = NULL;
    enum{frameCount = 1600};
    float samples[frameCount], overflow[frameCount], *pSample, *pRead, *pWrite;

    // Initialize COM
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED); 
    if (FAILED(hr)) {
        fprintf(stderr, "Failed to initialize COM: 0x%lx\n", hr);
        return(EXIT_FAILURE);
    }

    // Create device enumerator
    hr = CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,      // https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-cocreateinstance
                          &IID_IMMDeviceEnumerator, (void**)&pEnumerator);  // Create a device enumerator. A device enumerator is an object with an IMMDeviceEnumerator interface
    if (FAILED(hr)) {                                                       // The CLSID associated with the data and code that will be used to create the object.
        fprintf(stderr, "Failed to create device enumerator: 0x%lx\n", hr);          // Pass the null pointer to enumerator, casting as type pointer to pointer to void
        goto cleanup;
    }

    // Get default audio endpoint for the rendering function, e.g. headphones or speakers
    hr = IMMDeviceEnumerator_GetDefaultAudioEndpoint(pEnumerator, eRender, eMultimedia, &pEndpoint); // https://learn.microsoft.com/en-us/windows/win32/api/mmdeviceapi/nf-mmdeviceapi-immdeviceenumerator-getdefaultaudioendpoint
    /*
    Changed from eConsole to eMultimedia:
    "
    eConsole
    Value: 0
    Games, system notification sounds, and voice commands.
    eMultimedia
    Music, movies, narration, and live music recording.
    "

    The default device covered all eRoles in Vista, but this might've changed in Windows 11

    On the right-hand side of the assignment operator, we're implicitly accessing the lpVtbl 
    array of function pointers which was initialized by CoCreateInstance, through the pEnumerator 
    pointer. We're then getting the function pointer to GetDefaultAudioEndpoint, and passing it 
    the pEnumerator, eRender, eMultimedia, &pEndpoint arguments. It passes back a HRESULT and 
    populates the pEndpoint pointer with address of the IMMDevice interface of the endpoint 
    object for the default audio endpoint device. This is the COM calling convention.

    Through this method, the caller obtains a counted reference to the interface. The caller is 
    responsible for releasing the interface, when it is no longer needed, by calling the 
    interface's Release method. If the GetDefaultAudioEndpoint call fails, *pEndpoint is NULL.
    */
    if (FAILED(hr)) {
        printf(stderr, "Failed to get default audio endpoint: 0x%lx\n", hr);
        goto cleanup;
    }

    // Activate audio client
    hr = IMMDevice_Activate(pEndpoint, &IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&pAudioClient);    // The Activate method creates a COM object with the specified interface.
    /*
    So again, what we're saying here is go into the lpVtbl in pEndpoint and get the function pointer of
    IMMDevice_Activate. We pass it the interface identifier (IID) IID_IAudioClient that we defined as
    a constant. in pEndpoint that we got from 
    the GetDefault method, presumably an
    
    */
    if (FAILED(hr)) {
        printf(stderr, "Failed to activate audio client: 0x%lx\n", hr);
        goto cleanup;
    }

    /*
    Get mix format to see what the device supports
    Writes the address of a WAVEFORMATEX structure into pDeviceFormat
    and allocates storage for the structure. The caller is responsible 
    for freeing the storage, when it is no longer needed, by calling 
    the CoTaskMemFree function

    The client can call this method before calling the IAudioClient::Initialize 
    method. When creating a shared-mode stream for an audio endpoint 
    device, the Initialize method always accepts the stream format 
    obtained from a GetMixFormat call on the same device.

    The mix format is the format that the audio engine uses internally 
    for digital processing of shared-mode streams. This format is not 
    necessarily a format that the audio endpoint device supports. 
    */

    hr = IAudioClient_GetMixFormat(pAudioClient, &pDeviceFormat);
    if (FAILED(hr)) {
        printf("Failed to get mix format: 0x%lx\n", hr);
        goto cleanup;
    }

    printf("Capturing in device mix format:\n");
    printf("  Sample Rate: %lu Hz\n", pDeviceFormat->nSamplesPerSec);
    printf("  Channels: %d\n", pDeviceFormat->nChannels);
    printf("  Bits per sample: %d\n", pDeviceFormat->wBitsPerSample);

    /*
    Loopback capture:
    Initialize IAudioClient on the default render enpoint (headphones or speakers)
    with flag AUDCLNT_STREAMFLAGS_LOOPBACK 

    During stream initialization, the client can, as an option, enable event-driven 
    buffering. To do so, the client calls the IAudioClient::Initialize method with 
    the AUDCLNT_STREAMFLAGS_EVENTCALLBACK flag set. 
    */

    hr = IAudioClient_Initialize(pAudioClient,
                                AUDCLNT_SHAREMODE_SHARED,
                                AUDCLNT_STREAMFLAGS_LOOPBACK,
                                requestedDuration,
                                0,
                                &pDeviceFormat,  // Use what we're given
                                NULL);

    // Create event for buffer notifications
    bufferEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!bufferEvent) {
        printf("Failed to create buffer event\n");
        goto cleanup;
    }                               

    if (hr == AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED) {
        // Get the aligned buffer size
        hr = IAudioClient_GetBufferSize(pAudioClient, &bufferFrameCount);
        if (FAILED(hr)) goto cleanup;
        
        // Release and recreate
        IAudioClient_Release(pAudioClient);
        hr = IMMDevice_Activate(pEndpoint, &IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&pAudioClient);
        if (FAILED(hr)) goto cleanup;

        requestedDuration = (REFERENCE_TIME)((10000000.0 * bufferFrameCount / pDeviceFormat.nSamplesPerSec) + 0.5);

        hr = IAudioClient_Initialize(pAudioClient,
                                        AUDCLNT_SHAREMODE_SHARED,
                                        AUDCLNT_STREAMFLAGS_LOOPBACK,
                                        requestedDuration,
                                        0,
                                        pDeviceFormat,
                                        NULL);  
    }

    /*
    After enabling event-driven buffering, and before calling the 
    IAudioClient::Start method to start the stream, the client 
    must call SetEventHandle to register the event handle that the 
    system will signal each time a buffer becomes ready to be processed 
    by the client

    The SetEventHandle method sets the event handle that the system signals 
    when an audio buffer is ready to be processed by the client. Requires 
    prior initialization of the IAudioClient interface.
    */
    hr = IAudioClient_SetEventHandle(pAudioClient, bufferEvent);
    if (FAILED(hr)) {
        printf("Failed to set event handle: 0x%lx\n", hr);
        goto cleanup;
    }

    hr = IAudioClient_GetService(pAudioClient, &IID_IAudioCaptureClient, (void**)&pCaptureClient);
        if (FAILED(hr)) {
        printf("Failed to get capture client: 0x%lx\n", hr);
        goto cleanup;
    }
                                    
    // Start the stream
    hr = IAudioClient_Start(pAudioClient);
    if (FAILED(hr)) {
        printf("Failed to start audio client: 0x%lx\n", hr);
        goto cleanup;
    }

    printf("Playing... Press ESC to quit.\n\n");

    // Initialize the pointers
    pSample = samples;
    pRead = pWrite = overflow;

    while(running){
        /*
        Waits until the specified object is in the signaled state or the time-out interval elapses.
        */
        DWORD waitResult = WaitForSingleObject(bufferEvent, 100);   // waits for the event instead of continuous polling

        if(waitResult == WAIT_OBJECT_0){
            UINT32 numFramesAvailable;
            BYTE *pData;    // points to samples
            DWORD flags;    // tells us if the device was muted or data loss

            hr = IAudioCaptureClient_GetBuffer(&pCaptureClient, &pData, 
                                                &numFramesAvailable, &flags, NULL, NULL);

            /*
            During each ReleaseBuffer call, the caller reports the number of audio frames that it read from the buffer. 
            This number must be either the (nonzero) packet size or 0. If the number is 0, then the next GetBuffer call 
            will present the caller with the same packet as in the previous GetBuffer call.

            If we pass back the same variable, we're saying that we read everything we go
            */   

            /*
            The algorithm is as follows:
            1. On the initial pass, the sample array shouldn't fill up
            1a. If the array does fill up for some reason, we hop to 5.
            2. The FFT won't calculate until the array is full
            3. So we wait until the next iteration
            4. Fill up the remaining space in the sample array
            5. Calculate the FFT
            6. And take what won't fit and put it in the overflow buffer
            7. On the next pass, put what will fit from the overflow ring buffer in the sample array
            8. If any of the current audio buffer will fit in the remaining space in the sample array, then 
            put it in there
            9. Everything remaining that won't fit goes into the overflow buffer
            10. The sample array is full so we can calculate the FFT
            11. Go back to 7
            */
           
            
            int numSampRem = &samples[frameCount] - pSample;
            int readToEnd = &overflow[frameCount] - pRead;
            int startToWrite = pWrite - overflow;
            int numUnrdOvflw = (pWrite < pRead) ? (readToEnd + startToWrite) : (pWrite - pRead);

            // === PHASE 1: Drain overflow buffer into sample array ===
            if (pWrite != pRead) {  // Overflow has unread data
                if (pWrite < pRead) {  // Write pointer has wrapped
                    if (numSampRem <= readToEnd) {
                        // All we need fits before end of overflow buffer
                        memcpy(pSample, pRead, numSampRem * sizeof(float));
                        pRead += numSampRem;
                        pSample += numSampRem;
                    } else if (numSampRem <= numUnrdOvflw) {
                        // Need to wrap: copy to end, then from start
                        memcpy(pSample, pRead, readToEnd * sizeof(float));
                        pSample += readToEnd;
                        pRead = overflow;
                        memcpy(pSample, pRead, (numSampRem - readToEnd) * sizeof(float));
                        pRead += (numSampRem - readToEnd);
                        pSample += (numSampRem - readToEnd);
                    } else {
                        // Sample array can hold all overflow data
                        memcpy(pSample, pRead, readToEnd * sizeof(float));
                        pSample += readToEnd;
                        pRead = overflow;
                        memcpy(pSample, pRead, startToWrite * sizeof(float));
                        pSample += startToWrite;
                        pRead = pWrite;  // Buffer now empty
                    }
                } else {  // Write pointer hasn't wrapped (pWrite >= pRead)
                    if (numSampRem <= numUnrdOvflw) {
                        memcpy(pSample, pRead, numSampRem * sizeof(float));
                        pRead += numSampRem;
                        pSample += numSampRem;
                    } else {
                        memcpy(pSample, pRead, numUnrdOvflw * sizeof(float));
                        pSample += numUnrdOvflw;
                        pRead = pWrite;  // Buffer now empty
                    }
                }
            }

            // === PHASE 2: Handle incoming pData ===
            numSampRem = &samples[frameCount] - pSample;  // Recalculate after overflow drain
            int writeToEnd = &overflow[frameCount] - pWrite;

            if (numSampRem <= numFramesAvailable) {
                // Incoming data will fill (or overfill) sample array
                memcpy(pSample, pData, numSampRem * sizeof(float));
                pSample = samples;  // Reset for FFT, sample array now full
                
                // TODO: Trigger FFT here
                
                
                // Put remainder into overflow buffer
                int remainder = numFramesAvailable - numSampRem;
                if (remainder > 0) {
                    if (remainder >= writeToEnd) {
                        // Overflow write needs to wrap
                        memcpy(pWrite, &pData[numSampRem], writeToEnd * sizeof(float));
                        pWrite = overflow;
                        memcpy(pWrite, &pData[numSampRem + writeToEnd], (remainder - writeToEnd) * sizeof(float));
                        pWrite += (remainder - writeToEnd);
                    } else {
                        memcpy(pWrite, &pData[numSampRem], remainder * sizeof(float));
                        pWrite += remainder;
                    }
                }
            } else {
                // All incoming data fits in sample array
                memcpy(pSample, pData, numFramesAvailable * sizeof(float));
                pSample += numFramesAvailable;
            }
            if (SUCCEEDED(hr) && numFramesAvailable > 0) {
                IAudioCaptureClient_ReleaseBuffer(pCaptureClient, numFramesAvailable);
            }
        }

        // Poll keyboard
        if (key_pressed(VK_ESCAPE)) {
            running = 0;
            break;
        }
    }

cleanup:
    if (bufferEvent) CloseHandle(bufferEvent);
    if (pCaptureClient) IAudioCaptureClient_Release(pCaptureClient);
    if (pAudioClient) IAudioClient_Release(pAudioClient);
    if (pDeviceFormat) CoTaskMemFree(pDeviceFormat);
    if (pEndpoint) IMMDevice_Release(pEndpoint);
    if (pEnumerator) IMMDeviceEnumerator_Release(pEnumerator);
    CoUninitialize();

    printf("Done.\n");
    return 0;
}

double complex
cmp_exp(int k, int n){
    double complex z = 1.0 + 2.0*I;
    double complex w = 3.0 - 4.0*I;
    double complex product = z * w;  // Just works
    double re = creal(product);
    double im = cimag(product);

    double complex twiddle = cexp(-I * 2 * M_PI * k * n / N);
}

