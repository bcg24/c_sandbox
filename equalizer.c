#define COBJMACROS
#define WIN32_LEAN_AND_MEAN

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

#define PI 3.14159265358979323846

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
    IAudioCaptureClient *pCaptureClient = NULL;

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
    "eConsole
    Value: 0
    Games, system notification sounds, and voice commands.
    eMultimedia
    Music, movies, narration, and live music recording."

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
    Loopback capture:
    Initialize IAudioClient on the default render enpoint (headphones or speakers)
    with flag AUDCLNT_STREAMFLAGS_LOOPBACK 
    */
    hr = IAudioClient_Initialize(pAudioClient,                               //https://learn.microsoft.com/en-us/windows/desktop/api/pAudioClient/nf-pAudioClient-iaudioclient-initialize
                                    AUDCLNT_SHAREMODE_SHARED,
                                    AUDCLNT_STREAMFLAGS_LOOPBACK,
                                    requestedDuration,    // The buffer capacity as a time value
                                    0,                    // Can only be > 0 in exclusive mode
                                    mixFormat,            // Shares with the audio engine in shared mode
                                    NULL);      
                                    
    /*
    We need to calculate the bit twidling from the buffer size
    */
    hr = IAudioClient_GetBufferSize(pAudioClient, &bufferFrameCount);
    if (FAILED(hr)) goto cleanup;
    /*
    To get the size of the allocated buffer, which might be different 
    from the requested size, the client calls the IAudioClient::GetBufferSize method
    */

    hr = IAudioClient_GetService(pAudioClient, &IID_IAudioCaptureClient, (void**)&pCaptureClient);
    if (FAILED(hr)) goto cleanup;
                                    

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

