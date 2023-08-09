
#include "fileio.h"

int file_open(file_t *fp, const char* name, const char *mode) {
#if TEST_WAV_XSCOPE
    fp->xscope_file = xscope_open_file(name, (char*)mode); 
#else
    if(!strcmp(mode, "rb")) {
        fp->file = open(name, O_RDONLY);
        if(fp->file == -1) {return -1;}
    }
    else if(!strcmp(mode, "wb")) {
        fp->file = open(name, O_WRONLY|O_CREAT, 0644);
        if(fp->file == -1) {return -1;}
    }
    else {
        assert((0) && "invalid file open mode specified. Only 'rb' and 'wb' modes supported");
    }
#endif
    return 0;
}

void file_seek(file_t *fp, long int offset, int origin) {
#if TEST_WAV_XSCOPE
    xscope_fseek(&fp->xscope_file, offset, origin);
#else
    lseek(fp->file, offset, origin);
#endif
}

int get_current_file_offset(file_t *fp) {
#if TEST_WAV_XSCOPE
    int current_offset = xscope_ftell(&fp->xscope_file);
#else
    int current_offset = lseek(fp->file, 0, SEEK_CUR); 
#endif
    return current_offset;
}

int get_file_size(file_t *fp) {
#if TEST_WAV_XSCOPE
    //find the current offset in the file
    int current_offset = xscope_ftell(&fp->xscope_file);
    //go to the end
    xscope_fseek(&fp->xscope_file, 0, SEEK_END);
    //get offset which will be file size
    int size = xscope_ftell(&fp->xscope_file);
    //return back to the original offset
    xscope_fseek(&fp->xscope_file, current_offset, SEEK_SET);
#else
    //find the current offset in the file
    int current_offset = lseek(fp->file, 0, SEEK_CUR);
    //get file size
    int size = lseek(fp->file, 0, SEEK_END);
    //go back to original offset
    lseek(fp->file, current_offset, SEEK_SET);
#endif
    return size;
}

void file_read(file_t *fp, void *buf, size_t count) {
#if TEST_WAV_XSCOPE
    xscope_fread(&fp->xscope_file, (uint8_t*)buf, count);
#else
    read(fp->file, buf, count);
#endif
}

void file_write(file_t *fp, void *buf, size_t count) {
#if TEST_WAV_XSCOPE
    xscope_fwrite(&fp->xscope_file, (uint8_t*)buf, count);
#else
    write(fp->file, buf, count);
#endif
}

void file_close(file_t *fp) {
#if !TEST_WAV_XSCOPE
    close(fp->file);
#else
    //files are closed by a single call to xscope_close_all_files()
#endif
}

void shutdown_session() {
    //Needed for XSCOPE_ID_HOST_QUIT in xscope_close_all_files()
#if TEST_WAV_XSCOPE
    xscope_close_all_files();
#endif
}
