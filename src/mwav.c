#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <errno.h>

#include "mwav.h"

int16_t *   _ditSamples;
int16_t *   _dahSamples;
int16_t *   _spaceSamples;

void printUsage()
{
    printf("Usage:\n");
}

uint32_t buildDit()
{
    double          sampleIntervalDegrees;
    double          angle;
    uint32_t        numSamples;
    uint32_t        numSampleBytes;
    uint32_t        sampleNum;

    numSamples = (uint32_t)((double)WAV_SAMPLE_RATE * (double)MORSE_DIT_DURATION);
    numSampleBytes = numSamples * (WAV_BITS_PER_SAMPLE / 8);

    _ditSamples = (int16_t *)malloc(numSampleBytes);

    if (_ditSamples == NULL) {
        fprintf(stderr, "Failed to allocate %u sample data bytes\n", numSampleBytes);
        exit(-1);
    }

    sampleIntervalDegrees = (double)(FULL_CIRCLE_DEGREES * (double)WAV_BEEP_FREQ) / (double)WAV_SAMPLE_RATE;

    angle = 0.0;

    for (sampleNum = 0;sampleNum < numSamples;sampleNum++) {
        _ditSamples[sampleNum] = (int16_t)(sin(angle * (double)DEGREE_TO_RADIANS) * (double)WAV_MAX_AMPLITUDE);

        angle += sampleIntervalDegrees;

        if (angle > 360.0) {
            angle = angle - 360.0;
        }
    }

    return numSampleBytes;
}

uint32_t buildDah()
{
    double          sampleIntervalDegrees;
    double          angle;
    uint32_t        numSamples;
    uint32_t        numSampleBytes;
    uint32_t        sampleNum;

    numSamples = (uint32_t)((double)WAV_SAMPLE_RATE * (double)ITU_DAH_DURATION);
    numSampleBytes = numSamples * (WAV_BITS_PER_SAMPLE / 8);

    _dahSamples = (int16_t *)malloc(numSampleBytes);

    if (_dahSamples == NULL) {
        fprintf(stderr, "Failed to allocate %u sample data bytes\n", numSampleBytes);
        exit(-1);
    }

    sampleIntervalDegrees = (double)(FULL_CIRCLE_DEGREES * (double)WAV_BEEP_FREQ) / (double)WAV_SAMPLE_RATE;

    angle = 0.0;

    for (sampleNum = 0;sampleNum < numSamples;sampleNum++) {
        _dahSamples[sampleNum] = (int16_t)(sin(angle * (double)DEGREE_TO_RADIANS) * (double)WAV_MAX_AMPLITUDE);

        angle += sampleIntervalDegrees;

        if (angle > 360.0) {
            angle = angle - 360.0;
        }
    }

    return numSampleBytes;
}

uint32_t buildSpace()
{
    uint32_t        numSamples;
    uint32_t        numSampleBytes;
    uint32_t        sampleNum;

    numSamples = (uint32_t)((double)WAV_SAMPLE_RATE * (double)ITU_SPCE_DURATION);
    numSampleBytes = numSamples * (WAV_BITS_PER_SAMPLE / 8);

    _spaceSamples = (int16_t *)malloc(numSampleBytes);

    if (_spaceSamples == NULL) {
        fprintf(stderr, "Failed to allocate %u sample data bytes\n", numSampleBytes);
        exit(-1);
    }

    memset(_spaceSamples, 0, numSampleBytes);
    
    return numSampleBytes;
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
        samples[sampleNum] = (int16_t)(sin(angle * (double)DEGREE_TO_RADIANS) * (double)WAV_MAX_AMPLITUDE);

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
