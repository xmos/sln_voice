// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stdio.h>
#include <stdlib.h>

#include <xcore/thread.h>
#include <xcore/lock.h>

#include "asr_override.h"
#include "flash.h"

typedef struct read_ext_async_thread_data {
  lock_t lock;
  void *dest;
  const void *src;
  size_t n;
} read_ext_async_thread_data_t;

#define IS_FLASH(a)                    \
   (((uintptr_t)a >= XS1_SWMEM_BASE) && \
   (((uintptr_t)a <= (XS1_SWMEM_BASE - 1 + XS1_SWMEM_SIZE))))

// stack memory for the read_ext_async_thread function
#define READ_EXT_ASYNC_THREAD_STACKWORDS  (512)
__attribute__((aligned(8))) static char
    read_ext_async_stack[READ_EXT_ASYNC_THREAD_STACKWORDS*sizeof(uint32_t)];

void read_ext_async_thread(void* context) {
    read_ext_async_thread_data_t* td = (read_ext_async_thread_data_t*)context;
    
    // acquire the lock
    lock_acquire(td->lock);

    // depending on src address location, perform the flash read or memory copy
    if IS_FLASH(td->src) {
        flash_read_wrapper((unsigned *)td->dest, (unsigned)td->src, td->n);
    } else {
        memcpy(td->dest, td->src, td->n);
    }

    // release the lock
    lock_release(td->lock);  
}

void asr_read_ext(void *dest, const void *src, size_t n) {
    if IS_FLASH(src) {
        flash_read_wrapper((unsigned *)dest, (unsigned)src, n);
    } else {
        memcpy(dest, src, n);
    }
}

int asr_read_ext_async(void *dest, const void *src, size_t n) {
    // allocate a new hw lock
    lock_t lock = lock_alloc();

    // set the thread data
    read_ext_async_thread_data_t td;
    td.lock = lock;
    td.dest = dest;
    td.src = src;
    td.n = n;
    
    // run read_ext_async_thread in a new thread
    run_async(read_ext_async_thread, &td,
        stack_base(read_ext_async_stack, READ_EXT_ASYNC_THREAD_STACKWORDS));

    // return the lock as a handle so the caller can call asr_read_ext_wait later
    return (int)lock;
}

void asr_read_ext_wait(int handle) {
    lock_t lock = (lock_t) handle;
    // wait on lock
    //   reading thread will release when it is complete
    lock_acquire(lock);
    // release and free the lock
    lock_release(lock);    
    lock_free(lock);
    return;
}
