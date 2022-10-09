#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <ctype.h>

#include "mwav.h"

#define MAX_MESSAGE_LEN                 4096

int16_t *   _ditSamples;
int16_t *   _dahSamples;
int16_t *   _spaceSamples;


void printUsage()
{
    printf("Usage:\n");
    printf("\tmwav -o <output WAV file>\n\n");
}

void createSineWave(int16_t * samples, uint32_t numSamples)
{
    double          sampleIntervalDegrees;
    double          angle;
    uint32_t        sampleNum;

    sampleIntervalDegrees = (double)(FULL_CIRCLE_DEGREES * (double)WAV_BEEP_FREQ) / (double)WAV_SAMPLE_RATE;

    angle = 0.0;

    for (sampleNum = 0;sampleNum < numSamples;sampleNum++) {
        samples[sampleNum] = (int16_t)(sin(angle * (double)DEGREE_TO_RADIANS) * (double)WAV_MAX_AMPLITUDE);

        angle += sampleIntervalDegrees;

        if (angle > 360.0) {
            angle = angle - 360.0;
        }
    }
}

uint32_t buildDit()
{
    uint32_t        numSamples;
    uint32_t        numSampleBytes;

    numSamples = (uint32_t)((double)WAV_SAMPLE_RATE * (double)MORSE_DIT_DURATION);
    numSampleBytes = numSamples * (WAV_BITS_PER_SAMPLE / 8);

    _ditSamples = (int16_t *)malloc(numSampleBytes);

    if (_ditSamples == NULL) {
        fprintf(stderr, "Failed to allocate %u sample data bytes\n", numSampleBytes);
        exit(-1);
    }

    createSineWave(_ditSamples, numSamples);

    return numSamples;
}

uint32_t buildDah()
{
    uint32_t        numSamples;
    uint32_t        numSampleBytes;

    numSamples = (uint32_t)((double)WAV_SAMPLE_RATE * (double)ITU_DAH_DURATION);
    numSampleBytes = numSamples * (WAV_BITS_PER_SAMPLE / 8);

    _dahSamples = (int16_t *)malloc(numSampleBytes);

    if (_dahSamples == NULL) {
        fprintf(stderr, "Failed to allocate %u sample data bytes\n", numSampleBytes);
        exit(-1);
    }

    createSineWave(_dahSamples, numSamples);

    return numSamples;
}

uint32_t buildSpace()
{
    uint32_t        numSamples;
    uint32_t        numSampleBytes;

    numSamples = (uint32_t)((double)WAV_SAMPLE_RATE * (double)ITU_SPCE_DURATION);
    numSampleBytes = numSamples * (WAV_BITS_PER_SAMPLE / 8);

    _spaceSamples = (int16_t *)malloc(numSampleBytes);

    if (_spaceSamples == NULL) {
        fprintf(stderr, "Failed to allocate %u sample data bytes\n", numSampleBytes);
        exit(-1);
    }

    memset(_spaceSamples, 0, numSampleBytes);

    return numSamples;
}

const char * getMorseChar(char c)
{
    char            ch;
    const char *    morse;

    ch = toupper(c);

    if (isalpha(ch)) {
        morse = morseCodes[ch - 'A'];
    }
    else if (isdigit(ch)) {
        if (ch == '0') {
            morse = morseCodes[35];
        }
        else {
            morse = morseCodes[26 + (ch - '1')];
        }
    }
    else {
        return NULL;
    }

    return morse;
}

void unitTest(int testNum)
{
    char        c;
    int         i;

    switch (testNum) {
        case 1:
            c = 'A';

            for (i = 0;i < 26;i++) {
                printf("'%c' = '%s'\n", c, getMorseChar(c));
                c++;
            }
            break;
    }
}

int main(int argc, char ** argv)
{
    WAV             wave;
    uint32_t        numSampleBytes = 0U;
    uint32_t        fileLength;
    uint32_t        numDitSamples;
    uint32_t        numDahSamples;
    uint32_t        numSpaceSamples;
    FILE *          fptr;
    char *          pszFilename;
    char            szMessage[MAX_MESSAGE_LEN];
    const char *    pszMorse;
    char            morseCh;
    char            ch;
    int             i;
    int             j;
    size_t          messageLen;
    size_t          morseLen;
    size_t          sampleSize;

    //unitTest(1);

    if (argc > 1) {
        for (i = 1;i < argc;i++) {
            if (argv[i][0] == '-') {
                if (argv[i][1] == 'o') {
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

    printf("Enter message: ");
    fgets(szMessage, (MAX_MESSAGE_LEN - 1), stdin);

    messageLen = strlen(szMessage);

    if (messageLen == 0) {
        return 0;
    }

    numDitSamples = buildDit();
    numDahSamples = buildDah();
    numSpaceSamples = buildSpace();

    fptr = fopen(pszFilename, "wb");

    if (fptr == NULL) {
        fprintf(stderr, "Failed to open wav file %s for writing: %s\n", pszFilename, strerror(errno));
        exit(-1);
    }

    fwrite(&wave, 1, sizeof(WAV), fptr);

    sampleSize = (WAV_BITS_PER_SAMPLE / 8);

    for (i = 0;i < messageLen;i++) {
        ch = szMessage[i];

        if (ch == ' ') {
            /*
            ** Each word is separated by 7 spaces...
            */
            fwrite(_spaceSamples, sampleSize, numSpaceSamples, fptr);
            numSampleBytes += numSpaceSamples * sampleSize;
            fwrite(_spaceSamples, sampleSize, numSpaceSamples, fptr);
            numSampleBytes += numSpaceSamples * sampleSize;
            fwrite(_spaceSamples, sampleSize, numSpaceSamples, fptr);
            numSampleBytes += numSpaceSamples * sampleSize;
            fwrite(_spaceSamples, sampleSize, numSpaceSamples, fptr);
            numSampleBytes += numSpaceSamples * sampleSize;
            fwrite(_spaceSamples, sampleSize, numSpaceSamples, fptr);
            numSampleBytes += numSpaceSamples * sampleSize;
            fwrite(_spaceSamples, sampleSize, numSpaceSamples, fptr);
            numSampleBytes += numSpaceSamples * sampleSize;
            fwrite(_spaceSamples, sampleSize, numSpaceSamples, fptr);
            numSampleBytes += numSpaceSamples * sampleSize;
        }
        else {
            pszMorse = getMorseChar(ch);

            if (pszMorse != NULL) {
                morseLen = strlen(pszMorse);

                for (j = 0;j < morseLen;j++) {
                    morseCh = pszMorse[j];

                    if (morseCh == '.') {
                        fwrite(_ditSamples, sampleSize, numDitSamples, fptr);
                        numSampleBytes += numDitSamples * sampleSize;
                    }
                    else if (morseCh == '-') {
                        fwrite(_dahSamples, sampleSize, numDahSamples, fptr);
                        numSampleBytes += numDahSamples * sampleSize;
                    }

                    /*
                    ** If this isn't the last dit/dah in the morse sequence
                    ** add the space separator between...
                    */
                    if (j < (morseLen - 1)) {
                        fwrite(_spaceSamples, sampleSize, numSpaceSamples, fptr);
                        numSampleBytes += numSpaceSamples * sampleSize;
                    }
                }

                /*
                ** Each character is separated by 3 spaces...
                */
                fwrite(_spaceSamples, sampleSize, numSpaceSamples, fptr);
                numSampleBytes += numSpaceSamples * sampleSize;
                fwrite(_spaceSamples, sampleSize, numSpaceSamples, fptr);
                numSampleBytes += numSpaceSamples * sampleSize;
                fwrite(_spaceSamples, sampleSize, numSpaceSamples, fptr);
                numSampleBytes += numSpaceSamples * sampleSize;
            }
        }
    }

    wave.dataChunk.length = numSampleBytes;

    fileLength = sizeof(WAV) + numSampleBytes;

    wave.fileSize = fileLength - 8;;

    /*
    ** Rewind to the beginning of the file and
    ** rewrite the WAV structure as we now have
    ** the length fields...
    */
    rewind(fptr);

    fwrite(&wave, 1, sizeof(WAV), fptr);

    fclose(fptr);

    free(_ditSamples);
    free(_dahSamples);
    free(_spaceSamples);
    free(pszFilename);

    return 0;
}
