// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#define DEBUG_UNIT INTENT_SERVICER
#ifndef DEBUG_PRINT_ENABLE_INTENT_SERVICER
#define DEBUG_PRINT_ENABLE_INTENT_SERVICER 0
#endif
#include "debug_print.h"

#include <stdio.h>
#include <string.h>
#include <platform.h>
#include <xassert.h>

#include "platform/platform_conf.h"
#include "servicer.h"
#include "intent_servicer.h"
#include "asr.h"
#include "device_control_i2c.h"

enum e_intent_controller_servicer_resid_cmds
{
#ifndef INTENT_CONTROLLER_SERVICER_RESID_LAST_DETECTION
    INTENT_CONTROLLER_SERVICER_RESID_LAST_DETECTION = 88,
#endif
    NUM_INTENT_CONTROLLER_SERVICER_RESID_CMDS = 1
};
static control_cmd_info_t intent_controller_servicer_resid_cmd_map[] =
{
    { INTENT_CONTROLLER_SERVICER_RESID_LAST_DETECTION, 3, sizeof(uint8_t), CMD_READ_ONLY },
};

void intent_servicer_init(servicer_t *servicer)
{
    // Servicer resource info
    static control_resource_info_t intent_res_info[NUM_RESOURCES_INTENT_SERVICER];

    memset(servicer, 0, sizeof(servicer_t));
    servicer->id = INTENT_CONTROLLER_SERVICER_RESID;
    servicer->start_io = 0;
    servicer->num_resources = NUM_RESOURCES_INTENT_SERVICER;

    servicer->res_info = &intent_res_info[0];
    // Servicer resource
    servicer->res_info[0].resource = INTENT_CONTROLLER_SERVICER_RESID;
    servicer->res_info[0].command_map.num_commands = NUM_INTENT_CONTROLLER_SERVICER_RESID_CMDS;
    servicer->res_info[0].command_map.commands = intent_controller_servicer_resid_cmd_map;
}

void intent_servicer(void *args) {
    device_control_servicer_t servicer_ctx;

    servicer_t *servicer = (servicer_t*)args;
    xassert(servicer != NULL);
    control_resid_t *resources = (control_resid_t*)pvPortMalloc(servicer->num_resources * sizeof(control_resid_t));
    for(int i=0; i<servicer->num_resources; i++)
    {
        resources[i] = servicer->res_info[i].resource;
    }

    control_ret_t dc_ret;
    debug_printf("Calling device_control_servicer_register(), servicer ID %d, on tile %d, core %d.\n", servicer->id, THIS_XCORE_TILE, rtos_core_id_get());

    dc_ret = device_control_servicer_register(&servicer_ctx,
                                            device_control_ctxs,
                                            1,
                                            resources, servicer->num_resources);
    debug_printf("Out of device_control_servicer_register(), servicer ID %d, on tile %d. servicer_ctx address = 0x%x\n", servicer->id, THIS_XCORE_TILE, &servicer_ctx);

    vPortFree(resources);

    for(;;){
        device_control_servicer_cmd_recv(&servicer_ctx, read_cmd, write_cmd, servicer, RTOS_OSAL_WAIT_FOREVER);
    }
}
extern uint32_t detection_number;
extern asr_result_t last_asr_result;
control_ret_t intent_servicer_read_cmd(control_resource_info_t *res_info, control_cmd_t cmd, uint8_t *payload, size_t payload_len)
{
    control_ret_t ret = CONTROL_SUCCESS;
    uint8_t cmd_id = CONTROL_CMD_CLEAR_READ(cmd);

    memset(payload, 0, payload_len);

    debug_printf("intent_servicer_read_cmd, cmd_id: %d.\n", cmd_id);
    //asr_result_t asr_result;

    //asr_error_t asr_error = asr_get_result(NULL, &asr_result);
    printf("detection_number %d, last_asr_result.id %d\n", detection_number, last_asr_result.id);

    switch (cmd_id)
    {
    case INTENT_CONTROLLER_SERVICER_RESID_LAST_DETECTION:
    {
        debug_printf("INTENT_CONTROLLER_SERVICER_RESID_LAST_DETECTION\n");
        payload[0] = (uint8_t) last_asr_result.id;
        payload[1] = (uint8_t) detection_number;
        break;
    }

    default:
    {
        debug_printf("INTENT_CONTROLLER_SERVICER UNHANDLED COMMAND!!!\n");
        ret = CONTROL_BAD_COMMAND;
        break;
    }

    }
    return ret;
}

control_ret_t intent_servicer_write_cmd(control_resource_info_t *res_info, control_cmd_t cmd, const uint8_t *payload, size_t payload_len)
{
    control_ret_t ret = CONTROL_SUCCESS;

    uint8_t cmd_id = CONTROL_CMD_CLEAR_READ(cmd);
    debug_printf("intent_servicer_write_cmd cmd_id %d.\n", cmd_id);

    switch (cmd_id)
    {

    // Add the handling of the write commands here
    default:
        debug_printf("INTENT_CONTROLLER_SERVICER UNHANDLED COMMAND!!!\n");
        ret = CONTROL_BAD_COMMAND;
        break;
    }

    return ret;
}
