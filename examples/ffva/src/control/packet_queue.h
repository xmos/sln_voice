// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.
#pragma once
#include "device_control_shared.h"

/**
 * Clears the read bit on a command code
 *
 * \param[in,out] c The command code to clear the read bit on.
 */
#define CONTROL_CMD_CLEAR_READ(c) ((c) & ~0x80)

/**
 * @brief Control packet status
 *
 */
typedef enum {
    PKT_FREE,   /// Packet is free and available for the servicer to write into
    PKT_WAIT,   /// Packet contains a command written by the servicer and is waiting to be processed by the resource
    PKT_DONE    /// Packet contains a command that has been processed by the underlying resource
}pkt_status_t;

/**
 * @brief Control packet structure
 *
 */
typedef struct {
    uint8_t res_id;            /// Resource ID of the command in the packet
    uint8_t cmd_id;            /// Command ID for the command in the packet
    int32_t payload_len;       /// packet payload length in bytes
    pkt_status_t pkt_status;   /// packet status
    void *payload;             /// packet payload pointer
    uint8_t *save_payload_ptr; /// Holds a copy of the original payload ptr when the original payload_ptr gets overwritten during special command processing
    int32_t free_when_done;    /// Free packet once its done by the resource. When looking for a free packet, the pkt queue function queue_get_free_pkt() will free a done packet only if this flag is set to 1.
}control_pkt_t;

/**
 * @brief Control packet queue structure
 *
 */
typedef struct {
    control_pkt_t *pkts;      /// Array of packets that make up this queue
    int32_t depth;            /// Queue depth in number of packets
    int32_t queue_wr_index;   /// Queue current write index. This points to the packet that will be written to when the servicer attempts to add a packet to the queue.
    int32_t queue_rd_index;   /// Queue current read index. This points to the packet that the resource will read when it wants to service a command.
}control_pkt_queue_t;

/**
 * @brief Add a command to the pkt queue.
 *
 * @param control_pkt_queue     Pointer to the packet queue object
 * @param resid                 Command resource ID
 * @param cmd                   Command command ID
 * @param payload               Command payload buffer
 * @param payload_len           Payload buffer length
 * @return CONTROL_SUCCESS if command added to the queue, SERVICER_QUEUE_FULL if queue is full and command cannot be added
 */
control_ret_t queue_write_command_to_free_packet(control_pkt_queue_t *control_pkt_queue, control_resid_t resid, control_cmd_t cmd, const uint8_t *payload, size_t payload_len);

/**
 * @brief Check if a command is present in the packet queue
 *
 * @param pkt_queue         Pointer to the pkt queue objects
 * @param cmd               Command ID
 * @return Pointer to the pkt object holding the command if command is present in the queue, NULL otherwise.
 */
control_pkt_t* queue_check_packet(control_pkt_queue_t *pkt_queue, control_cmd_t cmd);

/**
 * @brief Return the packet that queue's current read pointer is pointing to
 *
 * @param pkt_queue         Pointer to the pkt queue object
 * @return                  Pointer to the packet object if read pointer is pointing to a valid packet, NULL otherwise.
 */
control_pkt_t* queue_read_packet(control_pkt_queue_t *pkt_queue);

/**
 * @brief Set the packet which is at the current_rd_index in the queue to DONE.
 * Increment the current_rd_index to point to the next packet that needs processing
 *
 * @param pkt_queue Pointer to the packet queue object
 */
void queue_set_packet_done(control_pkt_queue_t *pkt_queue);

// Return pointer to the first free pkt in the queue
/**
 * @brief Return pointer to a free packet in the control packet queue
 *
 * This function returns a free packet pointer which can then be written with a command by
 * the servicer and added to the queue.
 * Since packets are written to the queue in order, this function only looks at the packet at the queue_wr_index
 * and returns the pointer to it if that packet is free or can be freed.
 *
 * @param pkt                   free packet pointer that is returned for use by the servicer
 * @param control_pkt_queue     Pointer to the control packet queue
 * @return control_ret_t        CONTROL_SUCCESS if a free packet pointer is updated in *pkt, SERVICER_QUEUE_FULL
 *                              if the queue is full.
 */
control_ret_t queue_get_free_pkt(control_pkt_t **pkt, control_pkt_queue_t *control_pkt_queue);

/**
 * @brief Add packet to the control packet queue
 *
 * This command adds the packet at queue_wr_index to the queue. This means that the packet has now been written
 * with a command from the servicer and is waiting for being serviced by the resource.
 *
 * @param control_pkt_queue     Pointer to the control packet queue
 * @param free_when_done        When set to 1, queue_get_free_pkt() frees the pkt when the resource marks it as Done.
 */
void queue_add_pkt(control_pkt_queue_t *control_pkt_queue, int32_t free_when_done);
