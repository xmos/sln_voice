// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xcore/lock.h>
#include <xcore/thread.h>
#include <xcore/assert.h>

#include "device_memory.h"
#include "device_memory_impl.h"
#include "flash.h"

typedef struct read_ext_async_thread_data {
  lock_t lock;
  void *dest;
  const void *src;
  size_t n;
} read_ext_async_thread_data_t;

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


__attribute__((fptrgroup("devmem_malloc_fptr_grp")))
void * devmem_malloc_local(size_t size) {
    return malloc(size);
}

__attribute__((fptrgroup("devmem_free_fptr_grp")))
void devmem_free_local(void *ptr) {
    free(ptr);
}

__attribute__((fptrgroup("devmem_read_ext_fptr_grp")))
void devmem_read_ext_local(void *dest, const void *src, size_t n) {
    if IS_FLASH(src) {
        flash_read_wrapper((unsigned *)dest, (unsigned)(src-XS1_SWMEM_BASE), n);
    } else {
        memcpy(dest, src, n);
    }
}

__attribute__((fptrgroup("devmem_read_ext_async_fptr_grp")))
int devmem_read_ext_async_local(void *dest, const void *src, size_t n) {
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

__attribute__((fptrgroup("devmem_read_ext_wait_fptr_grp")))
void devmem_read_ext_wait_local(int handle) {
    lock_t lock = (lock_t) handle;
    // wait on lock
    //   reading thread will release when it is complete
    lock_acquire(lock);
    // release and free the lock
    lock_release(lock);    
    lock_free(lock);
    return;
}

void devmem_init(devmem_manager_t *devmem_ctx) {
    xassert(devmem_ctx);    
    devmem_ctx->malloc = devmem_malloc_local;
    devmem_ctx->free = devmem_free_local;
    devmem_ctx->read_ext = devmem_read_ext_local;
    devmem_ctx->read_ext_async = devmem_read_ext_async_local;
    devmem_ctx->read_ext_wait = devmem_read_ext_wait_local;
}
