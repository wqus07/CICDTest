/**
 * @file    uds.h
 * @brief   UDS module integration API
 *
 * Single-header entry point for integrating UDS into an application.
 * Usage:
 *   1. Call uds_init() once at startup, after FDCAN is initialised.
 *   2. Call uds_tick() every 1 ms (from SysTick or timer callback).
 *   3. Route relevant CAN frames to uds_can_rx() from the FDCAN callback.
 *   4. Call uds_process() in the main loop to handle complete messages.
 */
#ifndef UDS_H
#define UDS_H

#include "uds_config.h"
#include "uds_service.h"
#include "uds_tp.h"
#include "uds_flash.h"

/* ── Public API ────────────────────────────────────────────────────────── */

/**
 * @brief  Initialise the UDS stack (transport + services + flash).
 */
void uds_init(void);

/**
 * @brief  Feed a received CAN frame into the UDS stack.
 * @param  data  CAN frame payload
 * @param  dlc   Payload length
 * @note   Call from FDCAN RX callback for UDS_CAN_RX_ID / UDS_CAN_FUNC_ID.
 */
void uds_can_rx(const uint8_t *data, uint8_t dlc);

/**
 * @brief  Process pending UDS messages. Call from main loop.
 */
void uds_process(void);

/**
 * @brief  1 ms periodic tick. Call from timer ISR or SysTick.
 */
void uds_tick(void);

/**
 * @brief  Trigger flash programming of the downloaded image.
 *         Call after the complete UDS download sequence.
 *         Does NOT return on success (resets the MCU).
 * @return Only returns on error.
 */
uds_flash_status_t uds_program(void);

/**
 * @brief  Get pointer to the UDS context (for advanced use / testing).
 * @return Pointer to the global UDS context
 */
uds_ctx_t *uds_get_ctx(void);

#endif /* UDS_H */
