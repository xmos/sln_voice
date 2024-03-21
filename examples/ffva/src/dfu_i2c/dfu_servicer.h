// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.
#pragma once

#define DFU_CONTROLLER_SERVICER_RESID   (240)
#define NUM_RESOURCES_DFU_SERVICER      (1) // DFU servicer

/**
 * @brief DFU servicer read command handler
 *
 * Handles read commands dedicated to the DFU servicer resource
 *
 * @param res_info          Resource info of the current command
 * @param cmd               Command ID of this command
 * @param payload           Pointer to the payload that contains the write data
 * @param payload_len       Length in bytes of the write command payload
 * @return control_ret_t    CONTROL_SUCCESS if command handled successfully,
 *                          otherwise control_ret_t error status indicating the error.
 */
control_ret_t dfu_servicer_read_cmd(control_resource_info_t *res_info, control_cmd_t cmd, uint8_t *payload, size_t payload_len);

/**
 * @brief DFU servicer write command handler
 *
 * Handles write commands dedicated to the DFU servicer resource
 *
 * @param res_info          Resource info of the current command
 * @param cmd               Command ID of this command
 * @param payload           Pointer to the payload that contains the write data
 * @param payload_len       Length in bytes of the write command payload
 * @return control_ret_t    CONTROL_SUCCESS if command handled successfully,
 *                          otherwise control_ret_t error status indicating the error.
 */
control_ret_t dfu_servicer_write_cmd(control_resource_info_t *res_info, control_cmd_t cmd, const uint8_t *payload, size_t payload_len);