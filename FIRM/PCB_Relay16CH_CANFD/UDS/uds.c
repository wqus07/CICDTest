/**
 * @file    uds.c
 * @brief   UDS module integration – glue between CAN hardware and UDS stack
 *
 * This file provides:
 *   - The global UDS context
 *   - CAN TX callback (platform-specific, uses project's FDCAN1_Send_Msg)
 *   - Integration API (init / rx / process / tick / program)
 */
#include "uds.h"
#include "fdcan.h"
#include <string.h>

/* ── External: project CAN send function ───────────────────────────────── */
extern uint8_t FDCAN1_Send_Msg(uint32_t ST_ID, uint8_t *msg,
                                uint32_t len, uint8_t Mode);

/* ── Global UDS context ────────────────────────────────────────────────── */
static uds_ctx_t g_uds_ctx;

/* ── Platform callback: ISO-TP → CAN TX ────────────────────────────────── */

uint8_t uds_tp_can_send(uint32_t can_id, const uint8_t *data, uint8_t dlc)
{
    /*
     * Map UDS CAN ID type to FDCAN driver.
     * UDS uses classic CAN (Mode=0) for diagnostics, even if the
     * application uses CAN FD for normal traffic.
     */
    uint32_t fdcan_dlc;
    switch (dlc) {
    case 0:  fdcan_dlc = FDCAN_DLC_BYTES_0;  break;
    case 1:  fdcan_dlc = FDCAN_DLC_BYTES_1;  break;
    case 2:  fdcan_dlc = FDCAN_DLC_BYTES_2;  break;
    case 3:  fdcan_dlc = FDCAN_DLC_BYTES_3;  break;
    case 4:  fdcan_dlc = FDCAN_DLC_BYTES_4;  break;
    case 5:  fdcan_dlc = FDCAN_DLC_BYTES_5;  break;
    case 6:  fdcan_dlc = FDCAN_DLC_BYTES_6;  break;
    case 7:  fdcan_dlc = FDCAN_DLC_BYTES_7;  break;
    default: fdcan_dlc = FDCAN_DLC_BYTES_8;  break;
    }

    /* UDS diagnostic uses standard 11-bit IDs even though the application
     * relay control uses extended 29-bit IDs. Send via a local TxHeader
     * to avoid interfering with the relay CAN traffic. */
    FDCAN_TxHeaderTypeDef tx_header;
    tx_header.Identifier          = can_id;
#if (UDS_CAN_ID_TYPE == 0)
    tx_header.IdType              = FDCAN_STANDARD_ID;
#else
    tx_header.IdType              = FDCAN_EXTENDED_ID;
#endif
    tx_header.TxFrameType         = FDCAN_DATA_FRAME;
    tx_header.DataLength          = fdcan_dlc;
    tx_header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    tx_header.BitRateSwitch       = FDCAN_BRS_OFF;
    tx_header.FDFormat            = FDCAN_CLASSIC_CAN;
    tx_header.TxEventFifoControl  = FDCAN_NO_TX_EVENTS;
    tx_header.MessageMarker       = 0;

    if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &tx_header,
                                       (uint8_t *)data) != HAL_OK) {
        return 1U;
    }
    return 0U;
}

/* ── Integration API ───────────────────────────────────────────────────── */

void uds_init(void)
{
    uds_service_init(&g_uds_ctx);

    /* Add FDCAN filter for UDS physical and functional request IDs */
    FDCAN_FilterTypeDef filter;

#if (UDS_CAN_ID_TYPE == 0)
    /* Standard ID filter for UDS RX */
    filter.IdType       = FDCAN_STANDARD_ID;
    filter.FilterIndex  = 1;  /* Index 0 is used by relay application */
    filter.FilterType   = FDCAN_FILTER_DUAL;
    filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    filter.FilterID1    = UDS_CAN_RX_ID;
    filter.FilterID2    = UDS_CAN_FUNC_ID;
#else
    filter.IdType       = FDCAN_EXTENDED_ID;
    filter.FilterIndex  = 1;
    filter.FilterType   = FDCAN_FILTER_DUAL;
    filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    filter.FilterID1    = UDS_CAN_RX_ID;
    filter.FilterID2    = UDS_CAN_FUNC_ID;
#endif

    HAL_FDCAN_ConfigFilter(&hfdcan1, &filter);
}

void uds_can_rx(const uint8_t *data, uint8_t dlc)
{
    uds_tp_rx_indication(&g_uds_ctx.tp, data, dlc);
}

void uds_process(void)
{
    if (uds_tp_msg_ready(&g_uds_ctx.tp)) {
        uds_service_dispatch(&g_uds_ctx,
                             g_uds_ctx.tp.rx_buf,
                             g_uds_ctx.tp.rx_len);
        uds_tp_msg_consumed(&g_uds_ctx.tp);
    }
}

void uds_tick(void)
{
    uds_service_tick(&g_uds_ctx);
}

uds_flash_status_t uds_program(void)
{
    return uds_flash_program(&g_uds_ctx.flash);
}

uds_ctx_t *uds_get_ctx(void)
{
    return &g_uds_ctx;
}
