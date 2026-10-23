#include "cli.h"

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include "spi.h"

static inline char* nextToken() {
    return strtok(NULL, " ");
}

static char inputBuffer[CLI_INPUT_BUFFER_SIZE];
static size_t inputIndex = 0;

static inline void push(char c){
    inputBuffer[inputIndex] = c;
    inputIndex++;
}

static inline void pop(){
    if (inputIndex > 0) {
        inputIndex--;
        putchar(0x08);
        putchar(' ');
        putchar(0x08);
    }
}

bool CLI_read(){
    char c = getchar_timeout_us(100);

    if (c >= 'A' && c <= 'Z'){
        c = c - 'A' + 'a';
    }
    if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == ' '){
        putchar(c);
        push(c);
    }

    if (c == 0x7F){
        pop();
    }

    // if (c != 0xFE){
    //     printf("Press: 0x%02X\n", c);
    // }

    if (inputIndex == CLI_INPUT_BUFFER_SIZE || c == '\n' || c == '\r'){
        putchar('\n');
        inputBuffer[inputIndex] = 0;
        inputIndex = 0;
        return true;
    }
    return false;
}

static inline int32_t CLI_convertToInt(const char* str){
    int32_t value = 0;
    uint8_t base = 10;
    if (strlen(str) > 2 && str[1] == 'x'){
        str+=2;
        base = 16;
    } else if (strlen(str) > 2 && str[1] == 'b'){
        str+=2;
        base = 2;
    }
    char *end;
    return strtol(str, &end, base);
}

static int CLI_readSPI(){
    char *token = nextToken();
    if (!token) return -1;
    uint16_t address = CLI_convertToInt(token);
    token = nextToken();
    size_t size = 1;
    if (token){
        size = CLI_convertToInt(token);
        if (size == 0) return -2;
    }

    uint32_t readValues[512];
    SPI_read(address, readValues, size);
    for (size_t i = 0; i < size; i++){
        printf("%03i: 0x%08X\n", i, readValues[i]);
    }
    return 0;
}

static int CLI_writeSPI(){
    char *token = nextToken();
    if (!token) return 1;
    uint16_t address = CLI_convertToInt(token);

    uint32_t sendValues[512];
    int i = 0;
    token = nextToken();
    while(token){
        uint32_t readInt = CLI_convertToInt(token);
        sendValues[i] = readInt;
        i++;
        token = nextToken();
    }

    SPI_write(address, sendValues, i);
    return 0;
}


void CLI_execute(){
    char *token = strtok(inputBuffer, " ");
    int error = 0;

    if (
        strncmp(token, "read", 4) == 0 || strncmp(token, "r", 1) == 0
    ) {
        error = CLI_readSPI();
    } else if (
        strncmp(token, "write", 5) == 0 || strncmp(token, "w", 1) == 0
    ) {
        error = CLI_writeSPI();
    } else {
        error = 1;
    }

    if (error == -2) {
        printf("Invalid read count\n");
    } else if (error){
        printf("Invalid command\n");
    }
}

