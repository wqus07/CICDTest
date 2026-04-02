/**
 * @file    uds_tp.h
 * @brief   ISO 15765-2 (ISO-TP) Transport Layer over CAN
 *
 * Handles segmentation and reassembly of UDS messages that exceed a single
 * CAN frame.  Supports Single Frame, First Frame, Consecutive Frame, and
 * Flow Control.
 */
#ifndef UDS_TP_H
#define UDS_TP_H

#include <stdint.h>
#include <stdbool.h>
#include "uds_config.h"

/* ── ISO-TP PCI types ──────────────────────────────────────────────────── */
#define UDS_TP_PCI_SF       0x00U   /**< Single Frame       */
#define UDS_TP_PCI_FF       0x10U   /**< First Frame        */
#define UDS_TP_PCI_CF       0x20U   /**< Consecutive Frame  */
#define UDS_TP_PCI_FC       0x30U   /**< Flow Control       */

/* ── Flow Control Status ───────────────────────────────────────────────── */
#define UDS_TP_FC_CTS       0x00U   /**< Continue To Send   */
#define UDS_TP_FC_WAIT      0x01U   /**< Wait               */
#define UDS_TP_FC_OVFLW     0x02U   /**< Overflow / Abort   */

/* ── Transport layer state ─────────────────────────────────────────────── */
typedef enum {
    UDS_TP_IDLE = 0,
    UDS_TP_RX_IN_PROGRESS,      /**< Receiving multi-frame message       */
    UDS_TP_TX_IN_PROGRESS,      /**< Transmitting multi-frame response   */
    UDS_TP_TX_WAIT_FC,          /**< Waiting for Flow Control from tester*/
} uds_tp_state_t;

/* ── Transport layer context ───────────────────────────────────────────── */
typedef struct {
    /* RX side */
    uint8_t  rx_buf[UDS_TP_RX_BUF_SIZE];
    uint16_t rx_len;            /**< Expected total length (from FF)     */
    uint16_t rx_offset;         /**< Bytes received so far               */
    uint8_t  rx_sn;             /**< Expected sequence number            */
    uint32_t rx_timer;          /**< Timeout watchdog (ms)               */

    /* TX side */
    uint8_t  tx_buf[UDS_TP_TX_BUF_SIZE];
    uint16_t tx_len;            /**< Total response length               */
    uint16_t tx_offset;         /**< Bytes sent so far                   */
    uint8_t  tx_sn;             /**< Next sequence number                */
    uint8_t  tx_bs_count;       /**< Remaining frames in current block   */
    uint8_t  tx_bs;             /**< Block Size from FC                  */
    uint8_t  tx_stmin;          /**< STmin from FC (ms)                  */
    uint32_t tx_timer;          /**< Interval / timeout timer            */

    /* Common */
    uds_tp_state_t state;
    bool     msg_ready;         /**< Complete message available           */
} uds_tp_t;

/* ── Public API ────────────────────────────────────────────────────────── */

/**
 * @brief  Initialise the ISO-TP transport layer.
 * @param  tp  Pointer to transport context
 */
void uds_tp_init(uds_tp_t *tp);

/**
 * @brief  Feed a raw CAN frame into the transport layer.
 * @param  tp      Transport context
 * @param  data    CAN frame payload (up to UDS_CAN_DL bytes)
 * @param  dlc     Number of valid bytes
 * @note   Called from CAN RX ISR / callback.
 */
void uds_tp_rx_indication(uds_tp_t *tp, const uint8_t *data, uint8_t dlc);

/**
 * @brief  Start transmitting a UDS response via ISO-TP.
 * @param  tp      Transport context
 * @param  data    Pointer to response payload
 * @param  len     Response length in bytes
 */
void uds_tp_transmit(uds_tp_t *tp, const uint8_t *data, uint16_t len);

/**
 * @brief  Periodic tick – call every 1 ms from main loop or timer ISR.
 *         Handles CF transmission pacing and timeout monitoring.
 * @param  tp  Transport context
 */
void uds_tp_tick(uds_tp_t *tp);

/**
 * @brief  Check if a complete message has been received.
 * @param  tp  Transport context
 * @return true  if tp->rx_buf contains a complete UDS request
 */
static inline bool uds_tp_msg_ready(const uds_tp_t *tp) {
    return tp->msg_ready;
}

/**
 * @brief  Mark the current message as consumed.
 * @param  tp  Transport context
 */
static inline void uds_tp_msg_consumed(uds_tp_t *tp) {
    tp->msg_ready = false;
    tp->rx_len = 0;
    tp->rx_offset = 0;
}

/* ── Platform callback (implement per project) ─────────────────────────── */

/**
 * @brief  Send a single CAN frame.  Must be implemented by the integrator.
 * @param  can_id  CAN identifier
 * @param  data    Payload (up to UDS_CAN_DL bytes)
 * @param  dlc     Payload length
 * @return 0 on success
 */
extern uint8_t uds_tp_can_send(uint32_t can_id, const uint8_t *data, uint8_t dlc);

#endif /* UDS_TP_H */
