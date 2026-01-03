#include <stdio.h>
#include "portaudio.h"

int main() {
    Pa_Initialize();
    
    int numDevices = Pa_GetDeviceCount();
    printf("Number of devices: %d\n", numDevices);
    
    for (int i = 0; i < numDevices; i++) {
        const PaDeviceInfo *info = Pa_GetDeviceInfo(i);
        const PaHostApiInfo *hostInfo = Pa_GetHostApiInfo(info->hostApi);
        printf("Device %d: %s (%s)\n", i, info->name, hostInfo->name);
        printf("  Max inputs: %d, Max outputs: %d\n", 
               info->maxInputChannels, info->maxOutputChannels);
    }
    
    Pa_Terminate();
    return 0;
}