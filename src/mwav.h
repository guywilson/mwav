#include <stdint.h>

#ifndef __INCL_MWAV
#define __INCL_MWAV

#define WAV_SAMPLE_RATE             44100
#define WAV_BITS_PER_SAMPLE         16
#define WAV_NUM_CHANNELS            1
#define WAV_MAX_AMPLITUDE           8192            // The volume of the output WAV
#define WAV_BEEP_FREQ               880             // Equivalent to the note A
#define WAV_BYTES_PER_SEC           ((WAV_SAMPLE_RATE * WAV_BITS_PER_SAMPLE * WAV_NUM_CHANNELS) / 8)

#define FULL_CIRCLE_DEGREES         360
#define PI                          3.1415926535
#define DEGREE_TO_RADIANS           (PI / 180.0)

#define MORSE_DIT_DURATION          0.1

/*
** The following are defined by the International Telecommunication Union (ITU)
** and should not be changed...
*/
#define ITU_DAH_DURATION            (MORSE_DIT_DURATION * 3.0)
#define ITU_SPCE_DURATION           MORSE_DIT_DURATION
#define ITU_CHAR_SEPARATOR          (MORSE_DIT_DURATION * 3.0)
#define ITU_WORD_SEPARATOR          (MORSE_DIT_DURATION * 7.0)

const char * morseCodes[] = {
    ".-",       // A
    "-...",     // B
    "-.-.",     // C
    "-..",      // D
    ".",        // E
    "..-.",     // F
    "--.",      // G
    "....",     // H
    "..",       // I
    ".---",     // J
    "-.-",      // K
    ".-..",     // L
    "--",       // M
    "-.",       // N
    "---",      // O
    ".--.",     // P
    "--.-",     // Q
    ".-.",      // R
    "...",      // S
    "-",        // T
    "..-",      // U
    "...-",     // V
    ".--",      // W
    "-..-",     // X
    "-.--",     // Y
    "--..",     // Z
    ".----",    // 1
    "..---",    // 2
    "...--",    // 3
    "....-",    // 4
    ".....",    // 5
    "-....",    // 6
    "--...",    // 7
    "---..",    // 8
    "----.",    // 9
    "-----",    // 0
};

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

#endif
