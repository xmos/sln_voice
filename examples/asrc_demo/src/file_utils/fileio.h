#ifndef FILEIO_H
#define FILEIO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#if TEST_WAV_XSCOPE
#include "xscope_io_device.h"
#endif
#include <fcntl.h>
#include <unistd.h>

typedef union {
    int file;
#if TEST_WAV_XSCOPE
    xscope_file_t xscope_file;
#endif
}file_t;


int file_open(file_t *fp, const char* name, const char *mode);
void file_read(file_t *fp, void *buf, size_t count);
void file_write(file_t *fp, void *buf, size_t count);
void file_seek(file_t *fp, long int offset, int origin);
void file_close(file_t *fp);
void shutdown_session(); //Needed for XSCOPE_ID_HOST_QUIT in xscope_close_all_files()

int get_current_file_offset(file_t *fp);
int get_file_size(file_t *fp);
#endif
