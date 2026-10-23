#ifndef __CLI_H__
#define __CLI_H__

#include <pico/stdio.h>

#define CLI_INPUT_BUFFER_SIZE   1024

bool CLI_read();
void CLI_execute();

#endif
