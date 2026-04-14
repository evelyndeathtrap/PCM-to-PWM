#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <espeak/speak_lib.h>
#include <nvml.h>

nvmlReturn_t result;
    nvmlDevice_t device;
    unsigned int device_count;

/* The callback function where we receive PCM data */
int SynthCallback(short *wav, int numsamples, espeak_EVENT *events) {
    if (wav == NULL) return 0; // End of synthesis

    /* 'wav' contains the 16-bit signed PCM samples.
       'numsamples' is the number of samples in this buffer.
       For now, we'll just write them to stdout or a file.
    */
    for (int i = 0; i < numsamples; i++) {
     result = nvmlDeviceSetFanSpeed_v2(device, 0, wav[i]);
usleep(20);
result = nvmlDeviceSetFanSpeed_v2(device, 0, 0);
usleep(20);
    }
    return 0; // Continue synthesis
}

int main(int argc, char *argv[]) {
result = nvmlInit();
    if (NVML_SUCCESS != result) {
        printf("Failed to initialize NVML: %s\n", nvmlErrorString(result));
        return 1;
    }

    /* 2. Get the handle for the first GPU (index 0) */
    result = nvmlDeviceGetHandleByIndex(0, &device);
    if (NVML_SUCCESS != result) {
        printf("Failed to get handle for device 0: %s\n", nvmlErrorString(result));
    }

    /* 3. Set the Fan Control Policy to Manual */
    // Note: nvmlDeviceSetDefaultFanSpeed_v2 or similar might be required
    // depending on your driver version. This is the standard override:
    result = nvmlDeviceSetFanControlPolicy(device, 0, NVML_FAN_POLICY_MANUAL);
    
    if (NVML_SUCCESS != result) {
        printf("Manual control not supported or failed: %s\n", nvmlErrorString(result));
        printf("Check if you have sufficient permissions (sudo).\n");
    } else {
        /* 4. Set the actual speed */
    }
    if (argc < 2) {
        fprintf(stderr, "Usage: %s \"text to speak\"\n", argv[0]);
        return 1;
    }

    /* 1. Initialize espeak */
    // AUDIO_OUTPUT_RETRIEVAL allows us to get PCM via callback instead of playing it
    int samplerate = espeak_Initialize(AUDIO_OUTPUT_RETRIEVAL, 500, NULL, 0);
    
    if (samplerate == -1) {
        fprintf(stderr, "Failed to initialize eSpeak.\n");
        return 1;
    }

    /* 2. Set the callback */
    espeak_SetSynthCallback(SynthCallback);

    /* 3. Synthesize the text from argv[1] */
    unsigned int unique_identifier;
    espeak_Synth(argv[1], strlen(argv[1]) + 1, 0, POS_CHARACTER, 0, 
                 espeakCHARS_AUTO, &unique_identifier, NULL);

    /* 4. Wait for synthesis to complete */
    espeak_Synchronize();

    /* 5. Cleanup */
    espeak_Terminate();

    return 0;
}
