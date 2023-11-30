// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stdio.h>
#include <stdlib.h>

#include <xcore/parallel.h>

#include "flash.h"

DECLARE_JOB(process_file, (void))
void process_file();

void tile0_main() {
    flash_setup();

    PAR_JOBS(
        PJOB(process_file, ())
    );

    flash_teardown();
}
