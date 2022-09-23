#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <errno.h>

#define WAV_SAMPLE_RATE             44100
#define WAV_BITS_PER_SAMPLE         16
#define WAV_NUM_CHANNELS            1
#define WAV_MAX_AMPLITUDE           8192            // The volume of the output WAV
#define WAV_BEEP_FREQ               880             // Equivalent to the note A
#define WAV_BYTES_PER_SEC           ((WAV_SAMPLE_RATE * WAV_BITS_PER_SAMPLE * WAV_NUM_CHANNELS) / 8)

#define FULL_CIRCLE_DEGREES         360
#define PI                          3.1415926535
#define DEGREE_TO_RADIANS           (PI / 180.0)

#define MORSE_DIT_DURATION          0.25

/*
** The following are defined by the International Telecommunication Union (ITU)
** and should not be changed...
*/
#define ITU_DAH_DURATION            (MORSE_DIT_DURATION * 3.0)
#define ITU_SPCE_DURATION           MORSE_DIT_DURATION
#define ITU_CHAR_SEPARATOR          (MORSE_DIT_DURATION * 3.0)
#define ITU_WORD_SEPARATOR          (MORSE_DIT_DURATION * 7.0)

typedef struct __attribute__((__packed__)) {
    char            id[4];
    uint32_t        length;
    int16_t         tag;
    uint16_t        numChannels;
    uint32_t        sampleRate;
    uint32_t        avgBytesPerSec;
    uint16_t        blockAlign;
    uint16_t        bitsPerSample;
}
RF_FMT_CHUNK;

typedef struct __attribute__((__packed__)) {
    char            id[4];
    uint32_t        length;
}
RF_DATA_CHUNK;

typedef struct __attribute__((__packed__)) {
    char            id[4];
    uint32_t        fileSize;
    char            type[4];

    RF_FMT_CHUNK    formatChunk;
    RF_DATA_CHUNK   dataChunk;
}
WAV;

void printUsage()
{
    printf("Usage:\n");
}

int main(int argc, char ** argv)
{
    WAV         wave;
    double      sampleIntervalDegrees;
    double      angle;
    uint32_t    sampleNum;
    uint32_t    numSampleBytes;
    uint32_t    fileLength;
    uint32_t    numSamples;
    uint16_t    frequency;
    uint16_t    wavLength;
    int16_t *   samples;
    FILE *      fptr;
    char *      pszFilename;
    int         i;

    if (argc > 1) {
        for (i = 1;i < argc;i++) {
            if (argv[i][0] == '-') {
                if (argv[i][1] == 'l') {
                    wavLength = (uint16_t)atoi(&argv[i+1][0]);
                }
                else if (argv[i][1] == 'o') {
                    pszFilename = strdup(&argv[i+1][0]);
                }
                else {
                    printf("Invalid argument: %s\n\n", &argv[i][0]);
                    printUsage();
                    exit(-1);
                }
            }
        }
    }
    else {
        printUsage();
        exit(-1);
    }

    frequency = WAV_BEEP_FREQ;
    numSamples = WAV_SAMPLE_RATE * wavLength;

    memcpy(wave.id, "RIFF", 4);
    memcpy(wave.type, "WAVE", 4);

    memcpy(wave.formatChunk.id, "fmt ", 4);
    wave.formatChunk.length = 16;
    wave.formatChunk.tag = 1;
    wave.formatChunk.numChannels = WAV_NUM_CHANNELS;
    wave.formatChunk.sampleRate = WAV_SAMPLE_RATE;
    wave.formatChunk.avgBytesPerSec = WAV_BYTES_PER_SEC;
    wave.formatChunk.blockAlign = 2;
    wave.formatChunk.bitsPerSample = WAV_BITS_PER_SAMPLE;

    memcpy(wave.dataChunk.id, "data", 4);

    numSampleBytes = numSamples * (WAV_BITS_PER_SAMPLE / 8);
    samples = (int16_t *)malloc(numSampleBytes);

    if (samples == NULL) {
        fprintf(stderr, "Failed to allocate %u sample data bytes\n", numSampleBytes);
        exit(-1);
    }

    wave.dataChunk.length = numSampleBytes;

    fileLength = sizeof(WAV) + numSampleBytes;

    wave.fileSize = fileLength - 8;;

    sampleIntervalDegrees = (double)(FULL_CIRCLE_DEGREES * frequency) / (double)WAV_SAMPLE_RATE;

    angle = 0.0;

    for (sampleNum = 0;sampleNum < numSamples;sampleNum++) {
        samples[sampleNum] = (int16_t)(sin(angle * DEGREE_TO_RADIANS) * WAV_MAX_AMPLITUDE);

        angle += sampleIntervalDegrees;

        if (angle > 360.0) {
            angle = angle - 360.0;
        }
    }

    fptr = fopen(pszFilename, "wb");

    if (fptr == NULL) {
        fprintf(stderr, "Failed to open wav file %s for writing: %s\n", pszFilename, strerror(errno));
        exit(-1);
    }

    fwrite(&wave, 1, sizeof(WAV), fptr);
    fwrite(samples, (WAV_BITS_PER_SAMPLE / 8), numSamples, fptr);

    fclose(fptr);

    free(samples);
    free(pszFilename);

    return 0;
}
