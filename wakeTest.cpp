#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <windows.h>

#define ROUNDS 64

// WAKE key schedule

void key_schedule(uint32_t key, uint32_t schedule[ROUNDS]) {
    schedule[0] = key;
    for (int i = 1; i < ROUNDS; i++) {
        schedule[i] = (schedule[i - 1] + 0x6DC597F) * 0x5851F42D;
    }
}

// Is it possible to split these into two seperate 32-bit blocks to utilize 64 bit architecture? 

void add_padding(unsigned char** data, size_t* data_len) {
    size_t original_len = *data_len;
    size_t new_len = (*data_len + 3) & ~3; // Round up to the nearest 4 bytes
    if (new_len != original_len) {
        unsigned char* new_data = (unsigned char*)malloc(new_len);
        if (new_data == NULL) {
            // Handle memory allocation error
            return;
        }
        memset(new_data, 0, new_len);
        memcpy(new_data, *data, original_len);
        *data = new_data;
        *data_len = new_len;
    }
}
// WAKE encryption 
void wake_encrypt(uint32_t schedule[ROUNDS], uint32_t* data, size_t data_len) {
    for (size_t i = 0; i < data_len; i++) {
        for (int j = 0; j < ROUNDS; j++) {
            data[i] += schedule[j];
            data[i] = (data[i] << 3) | (data[i] >> 29);
        }
    }
}

// WAKE decryption 
void wake_decrypt(uint32_t schedule[ROUNDS], uint32_t* data, size_t data_len) {
    for (size_t i = 0; i < data_len; i++) {
        for (int j = ROUNDS - 1; j >= 0; j--) {
            data[i] = (data[i] >> 3) | (data[i] << 29);
            data[i] -= schedule[j];
        }
    }
}

// Padding removal 

void remove_padding(unsigned char** data, size_t* data_len) {
    // find the last non-zero byte
    int i = *data_len - 1;
    while (i >= 0 && (*data)[i] == 0) {
        i--;
    }

    // Calculate the new length without padding
    size_t new_len = i + 1;
    if (new_len != *data_len) {
        // Create a new buffer without padding
        unsigned char* new_data = (unsigned char*)malloc(new_len);
        if (new_data == NULL) {
            // Handle memory allocation error
            return;
        }
        memcpy(new_data, *data, new_len);
        *data = new_data;
        *data_len = new_len;
    }
}

