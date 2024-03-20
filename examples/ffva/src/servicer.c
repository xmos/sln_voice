// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.
#define DEBUG_UNIT SERVICER_TASK
#ifndef DEBUG_PRINT_ENABLE_SERVICER_TASK
    #define DEBUG_PRINT_ENABLE_SERVICER_TASK 1
#endif
#include "debug_print.h"
#include <stdio.h>
#include <platform.h>
#include "platform/platform_conf.h"
#include "device_control_i2c.h"
#include "servicer.h"
//#include "control_init.h"


#if appconfI2C_CTRL_ENABLED && ON_TILE(I2C_CTRL_TILE_NO)
static device_control_t device_control_i2c_ctx_s;
device_control_t *device_control_i2c_ctx = (device_control_t *) &device_control_i2c_ctx_s;
device_control_t *device_control_ctxs[APP_CONTROL_TRANSPORT_COUNT] = {
        (device_control_t *) &device_control_i2c_ctx_s,
};
#endif
static void i2c_slave_start(void)
{
#if appconfI2C_CTRL_ENABLED && ON_TILE(I2C_CTRL_TILE_NO)
    #include "print.h"
    printintln(222);

    rtos_i2c_slave_start(i2c_slave_ctx,
                         device_control_i2c_ctx,
                         (rtos_i2c_slave_start_cb_t) device_control_i2c_start_cb,
                         (rtos_i2c_slave_rx_cb_t) device_control_i2c_rx_cb,
                         (rtos_i2c_slave_tx_start_cb_t) device_control_i2c_tx_start_cb,
                         (rtos_i2c_slave_tx_done_cb_t) NULL,
                         NULL,
                         NULL,
                         appconfI2C_INTERRUPT_CORE,
                         appconfI2C_TASK_PRIORITY);
#endif
}

// Generic Servicer task.
void servicer_task(void *args) {
    device_control_servicer_t servicer_ctx;

    servicer_t *servicer = (servicer_t*)args;
    xassert(servicer != NULL);

    for(int i=0; i<servicer->num_resources; i++) {
        servicer->res_info[i].control_pkt_queue.queue_wr_index = 0;
    }
    
    control_resid_t *resources = (control_resid_t*)pvPortMalloc(servicer->num_resources * sizeof(control_resid_t));
    for(int i=0; i<servicer->num_resources; i++)
    {
        resources[i] = servicer->res_info[i].resource;
    }

    if(appconfI2C_CTRL_ENABLED > 0)
    {
        control_ret_t dc_ret;
        debug_printf("Calling device_control_servicer_register(), servicer ID %d, on tile %d, core %d.\n", servicer->id, THIS_XCORE_TILE, rtos_core_id_get());

        dc_ret = device_control_servicer_register(&servicer_ctx,
                                            device_control_ctxs,
                                            appconfI2C_CTRL_ENABLED,
                                            resources, servicer->num_resources);
        debug_printf("Out of device_control_servicer_register(), servicer ID %d, on tile %d. servicer_ctx address = 0x%x\n", servicer->id, THIS_XCORE_TILE, &servicer_ctx);
    }

    // Start I2C and SPI slave.
    // This ends up calling device_control_resources_register() which has a strange non-yielding implementation,
    // where it waits for servicers to register with the device control context. That's why, control_start_io_tasks()
    // is not called from platform_start(). Additionally, if there is only one xcore core dedicated for all RTOS tasks,
    // this design will not work, since device_control_resources_register() will not yield and the servicers wouldn't get
    // scheduled so they could register with the device control leading to an eventual timeout error from device_control_resources_register().
    printintln(333);
    if(1)//servicer->start_io == (int32_t)1)
    {
        i2c_slave_start();
    }
    vPortFree(resources);
    printintln(350);

    // The first call to device_control_servicer_cmd_recv triggers the device_control_i2c_start_cb() through I2S slave ISR call somehow. device_control_i2c_start_cb() and device_control_servicer_cmd_recv() have to be in separate logical cores
    // and be scheduled simultaneously for this to work.
    if(APP_CONTROL_TRANSPORT_COUNT > 0)
    {
        for(;;){
            device_control_servicer_cmd_recv(&servicer_ctx, read_cmd, write_cmd, servicer, RTOS_OSAL_WAIT_FOREVER);
        }
    }
    else
    {
        for(;;){
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

//-----------------Servicer read write callback functions-----------------------//
DEVICE_CONTROL_CALLBACK_ATTR
control_ret_t read_cmd(control_resid_t resid, control_cmd_t cmd, uint8_t *payload, size_t payload_len, void *app_data)
{
    control_ret_t ret = CONTROL_SUCCESS;
    servicer_t *servicer = (servicer_t*)app_data;

    // For read commands, payload[0] is reserved from status. So payload_len is one more than the payload_len stored in the resource command map
    payload_len -= 1;
    uint8_t *payload_ptr = &payload[1]; //Excluding the status byte, which is updated later.

    debug_printf("Servicer ID %d on tile %d received READ command %02x for resid %02x\n\t",servicer->id, THIS_XCORE_TILE, cmd, resid);
    debug_printf("The command is requesting %d bytes\n\t", payload_len);


    control_resource_info_t *current_res_info = get_res_info(resid, servicer);
    xassert(current_res_info != NULL); // This should never happen
    control_cmd_info_t *current_cmd_info;
    ret = validate_cmd(&current_cmd_info, current_res_info, cmd, payload_ptr, payload_len);
    if(ret != CONTROL_SUCCESS)
    {
        payload[0] = ret; // Update status in byte 0
        return ret;
    }
    // Check if command is for the servicer itself
    if(current_res_info->resource == servicer->res_info[0].resource)
    {
        ret = servicer_read_cmd(current_res_info, cmd, payload_ptr, payload_len);
        payload[0] = ret;
        return ret;
    }

    int32_t cmd_handled = 0;
    // If special command then handle through special command handler
    /*ret = special_read_cmd_handler(current_res_info, cmd, payload_ptr, payload_len, &cmd_handled);
    if(cmd_handled) // Command handled by the special command handler
    {
        payload[0] = ret;
        return ret;
    }*/

    // Read from one of the underlying resources
    ret = servicer_read_from_resource(current_res_info, cmd, payload_ptr, payload_len);
    payload[0] = ret;
    return ret;
}

DEVICE_CONTROL_CALLBACK_ATTR
control_ret_t write_cmd(control_resid_t resid, control_cmd_t cmd, const uint8_t *payload, size_t payload_len, void *app_data)
{
    control_ret_t ret = CONTROL_SUCCESS;
    servicer_t *servicer = (servicer_t*)app_data;
    //debug_printf("Device control WRITE. Servicer ID %d\n\t", servicer->id);

    debug_printf("Servicer ID %d on tile %d received WRITE command %02x for resid %02x\n\t", servicer->id, THIS_XCORE_TILE, cmd, resid);
    debug_printf("The command has %d bytes\n\t", payload_len);

    control_resource_info_t *current_res_info = get_res_info(resid, servicer);
    xassert(current_res_info != NULL);
    control_cmd_info_t *current_cmd_info;
    ret = validate_cmd(&current_cmd_info, current_res_info, cmd, payload, payload_len);
    if(ret != CONTROL_SUCCESS)
    {
        return ret;
    }
    // Check if command is for the servicer itself
    if(current_res_info->resource == servicer->res_info[0].resource)
    {
        ret = servicer_write_cmd(current_res_info, cmd, payload, payload_len);
        return ret;
    }

    // If special command then handle through special command handler
    /*int32_t cmd_handled = 0;
    ret = special_write_cmd_handler(current_res_info, cmd, payload, payload_len, &cmd_handled);
    if(cmd_handled) // Command handled by the special command handler
    {
        return ret;
    }*/

    // Command is for one of the underlying resources
    ret = servicer_write_to_resource(current_res_info, cmd, payload, payload_len);
    return ret;
}

// Initialise packet payload pointers to point to valid memory.


//-----------------Servicer helper functions-----------------------//
// Return a pointer to the control_cmd_info_t structure for a given command ID. Return NULL if command not found in the
// command map for the resource.
control_cmd_info_t* get_cmd_info(uint8_t cmd_id, const control_resource_info_t *res_info)
{
    for(int i=0; i<res_info->command_map.num_commands; i++)
    {
        if(res_info->command_map.commands[i].cmd_id == cmd_id)
        {
            return &res_info->command_map.commands[i];
        }
    }
    return NULL;
}

// Return a pointer to the servicer's control_resource_info_t structure for a given resource ID.
// Return NULL if the resource ID is not found in the list of resources serviced by the servicer.
control_resource_info_t* get_res_info(control_resid_t resource, const servicer_t *servicer)
{
    for(int res=0; res<servicer->num_resources; res++)
    {
        // Get the cmd_info for the current command
        if(servicer->res_info[res].resource == resource)
        {
            return &servicer->res_info[res];
        }
    }
    return NULL;
}

// Validate the command from the host against that commands information in the stored command_map
control_ret_t validate_cmd(control_cmd_info_t **cmd_info,
                            control_resource_info_t *res_info,
                            control_cmd_t cmd,
                            const uint8_t *payload,
                            size_t payload_len)
{
    control_ret_t ret = CONTROL_SUCCESS;
    *cmd_info = get_cmd_info(CONTROL_CMD_CLEAR_READ(cmd), res_info);
    if(*cmd_info == NULL)
    {
        return SERVICER_WRONG_COMMAND_ID;
    }

    // Validate non special command length
    // Don't do payload check for special commands since for the last filter chunk, host might request less than the payload length specified in the cmd_map

    if(payload_len != (*cmd_info)->bytes_per_val * (*cmd_info)->num_vals)
    {
        return SERVICER_WRONG_COMMAND_LEN;
    }
    // Payload validation happens either on the host or in the specific command handlers. No generic payload validation is done in the servicer.

    return ret;
}

// Process write commands directed to the servicer resource
control_ret_t servicer_write_cmd(control_resource_info_t *res_info, control_cmd_t cmd, const uint8_t *payload, size_t payload_len)
{
    // Handle write commands directed to a servicer resource
    switch(res_info->resource)
    {
        case DFU_CONTROLLER_SERVICER_RESID:
            return dfu_servicer_write_cmd(res_info, cmd, payload, payload_len);
        break;
    }
    return CONTROL_SUCCESS;
}

// Process read commands directed to the servicer resource
control_ret_t servicer_read_cmd(control_resource_info_t *res_info, control_cmd_t cmd, uint8_t *payload, size_t payload_len)
{
    control_ret_t ret = CONTROL_SUCCESS;
     // Handle read commands directed to a servicer resource
    switch(res_info->resource)
    {
        case DFU_CONTROLLER_SERVICER_RESID:
            ret = dfu_servicer_read_cmd(res_info, cmd, payload, payload_len);
            break;
    }
    return ret;
}

// Forward read command to underlying resource
control_ret_t servicer_read_from_resource(control_resource_info_t *res_info, control_cmd_t cmd, uint8_t *payload, size_t payload_len)
{
    // Process read cmd from underlying resource

    // Check if the read command is already in the queue
    control_pkt_t* cmd_pkt = queue_check_packet(&res_info->control_pkt_queue, cmd);
    // If cmd already in queue
    if(cmd_pkt != NULL)
    {
        switch(cmd_pkt->pkt_status)
        {
            case PKT_DONE:
                debug_printf("Read cmd and status DONE found in queue\n");
                memcpy(payload, cmd_pkt->payload, payload_len);
                cmd_pkt->pkt_status = PKT_FREE;
                return CONTROL_SUCCESS;
            case PKT_WAIT:
                debug_printf("Read cmd with status PKT_WAIT found in queue\n");
                return SERVICER_COMMAND_RETRY;
            default:
                xassert(0);
        }
    }

    // Otherwise add command to queue
    control_ret_t ret = queue_write_command_to_free_packet(&res_info->control_pkt_queue, res_info->resource, cmd, payload, payload_len);
    if (ret == CONTROL_SUCCESS) // Since we've just added the command to queue return retry for host to query for response later
    {
        return SERVICER_COMMAND_RETRY;
    }
    return ret;
}

// Forward write command to underlying resource
control_ret_t servicer_write_to_resource(control_resource_info_t *res_info, control_cmd_t cmd, const uint8_t *payload, size_t payload_len)
{
    control_ret_t ret = queue_write_command_to_free_packet(&res_info->control_pkt_queue, res_info->resource, cmd, payload, payload_len);
    return ret;
}
