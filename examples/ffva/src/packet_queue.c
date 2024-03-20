// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.
#include <stdio.h>
#include <string.h>
#include "debug_print.h"
#include "device_control_shared.h"
#include "packet_queue.h"

// Return pointer to the first free pkt in the queue
control_ret_t queue_get_free_pkt(control_pkt_t **pkt, control_pkt_queue_t *control_pkt_queue)
{
    // queue_wr_index should always point to the first free packet that can be used. Since commands are always processed in order,
    // we shouldn't look beyond queue_wr_index when searching for a free packet.
    *pkt = &control_pkt_queue->pkts[control_pkt_queue->queue_wr_index];
    if((*pkt)->pkt_status != PKT_FREE)
    {
        if ((*pkt)->pkt_status == PKT_DONE && // Resource is done processing the pkt
            ((*pkt)->free_when_done) // pkt set to be freed when completed
            )
        {
            // If this pkt was used for a special command original payload ptr might have been overwritten.
            if((*pkt)->save_payload_ptr != NULL) // Payload ptr was overwritten earlier for a special command
            {
                // Restore original payload ptr
                (*pkt)->payload = (*pkt)->save_payload_ptr;
                (*pkt)->save_payload_ptr = NULL;
            }
            (*pkt)->pkt_status = PKT_FREE;    
        }
        else {
            debug_printf("ERROR: Control packet queue FULL\n");
            (*pkt) = NULL;
            return SERVICER_QUEUE_FULL;
        }
    }
    return CONTROL_SUCCESS;
}

// Add the packet at queue_wr_index to queue.
void queue_add_pkt(control_pkt_queue_t *control_pkt_queue, int32_t free_when_done)
{
    control_pkt_queue->pkts[control_pkt_queue->queue_wr_index].pkt_status = PKT_WAIT;
    control_pkt_queue->pkts[control_pkt_queue->queue_wr_index].free_when_done = free_when_done;

    // Update queue_wr_index
    control_pkt_queue->queue_wr_index = (control_pkt_queue->queue_wr_index + 1) % control_pkt_queue->depth;
}

control_ret_t queue_write_command_to_free_packet(control_pkt_queue_t *control_pkt_queue, control_resid_t resid, control_cmd_t cmd, const uint8_t *payload, size_t payload_len) {
    // Make sure we're writing to a free packet
    control_pkt_t *pkt;  
    control_ret_t ret = queue_get_free_pkt(&pkt, control_pkt_queue);
    if(ret != CONTROL_SUCCESS)
    {
        return ret;
    }
    pkt->res_id = resid;
    pkt->cmd_id = cmd;
    pkt->payload_len = payload_len;
    if(!IS_CONTROL_CMD_READ(pkt->cmd_id)) { // Copy payload for write commands.
        memcpy(pkt->payload, &payload[0], pkt->payload_len * sizeof(uint8_t));
    }
    // Add packet to the queue
    /** free_when_done set to 1 for both read and write packets. This is to ensure
     * that the packet queue doesn't get full if the host gives up on a read packet
     * and stops retrying for the read response. While this would never happen with our
     * host app design, this would make a more robust design.
     */
    queue_add_pkt(control_pkt_queue, 1);
    
    //debug_printf("Cmd queue write index = %d\n", control_pkt_queue->queue_wr_index);
    return CONTROL_SUCCESS;
}

// Check if command is present in the packet.
// Return pkt pointer if present, NULL otherwise
control_pkt_t* queue_check_packet(control_pkt_queue_t *pkt_queue, control_cmd_t cmd)
{
    for(int i=0; i<pkt_queue->depth; i++)
    {
        if(pkt_queue->pkts[i].cmd_id == cmd)
        {
            if(pkt_queue->pkts[i].pkt_status != PKT_FREE)
            {
                return &pkt_queue->pkts[i];
            }
        }
    }
    return NULL;
}

// Return the packet that queue_rd_index is pointing to
control_pkt_t* queue_read_packet(control_pkt_queue_t *pkt_queue)
{
    // Packets read in order so only check packet at read_pointer location
    if(pkt_queue->pkts[pkt_queue->queue_rd_index].pkt_status == PKT_WAIT)
    {
        return &pkt_queue->pkts[pkt_queue->queue_rd_index];
    }
    return NULL;
}

void queue_set_packet_done(control_pkt_queue_t *pkt_queue)
{
    // Set the packet the current queue_rd_index is pointing to to DONE. Increment queue_rd_index
    pkt_queue->pkts[pkt_queue->queue_rd_index].pkt_status = PKT_DONE;
    pkt_queue->queue_rd_index = (pkt_queue->queue_rd_index + 1) % pkt_queue->depth;
}


