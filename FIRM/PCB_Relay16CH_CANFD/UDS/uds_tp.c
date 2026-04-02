/**
 * @file    uds_tp.c
 * @brief   ISO 15765-2 (ISO-TP) Transport Layer implementation
 */
#include "uds_tp.h"
#include <string.h>

/* ── Helpers ───────────────────────────────────────────────────────────── */

static void tp_send_fc(uds_tp_t *tp, uint8_t fs)
{
    uint8_t frame[UDS_CAN_DL];
    memset(frame, 0xCC, sizeof(frame));  /* padding */
    frame[0] = UDS_TP_PCI_FC | (fs & 0x0FU);
    frame[1] = UDS_TP_BLOCK_SIZE;
    frame[2] = UDS_TP_STMIN_MS;
    uds_tp_can_send(UDS_CAN_TX_ID, frame, UDS_CAN_DL);
}

static void tp_send_next_cf(uds_tp_t *tp)
{
    uint8_t frame[UDS_CAN_DL];
    memset(frame, 0xCC, sizeof(frame));

    frame[0] = UDS_TP_PCI_CF | (tp->tx_sn & 0x0FU);
    tp->tx_sn = (tp->tx_sn + 1U) & 0x0FU;

    uint16_t remaining = tp->tx_len - tp->tx_offset;
    uint8_t  copy_len  = (remaining > (UDS_CAN_DL - 1U))
                         ? (UDS_CAN_DL - 1U)
                         : (uint8_t)remaining;
    memcpy(&frame[1], &tp->tx_buf[tp->tx_offset], copy_len);
    tp->tx_offset += copy_len;

    uds_tp_can_send(UDS_CAN_TX_ID, frame, UDS_CAN_DL);
}

/* ── Public functions ──────────────────────────────────────────────────── */

void uds_tp_init(uds_tp_t *tp)
{
    memset(tp, 0, sizeof(*tp));
    tp->state = UDS_TP_IDLE;
}

void uds_tp_rx_indication(uds_tp_t *tp, const uint8_t *data, uint8_t dlc)
{
    if (dlc == 0U) return;

    uint8_t pci_type = data[0] & 0xF0U;

    switch (pci_type) {

    /* ── Single Frame ──────────────────────────────────────────────── */
    case UDS_TP_PCI_SF: {
        uint8_t sf_dl = data[0] & 0x0FU;
        if (sf_dl == 0U || sf_dl > (dlc - 1U)) return;
        if (sf_dl > UDS_TP_RX_BUF_SIZE) return;

        memcpy(tp->rx_buf, &data[1], sf_dl);
        tp->rx_len    = sf_dl;
        tp->rx_offset = sf_dl;
        tp->msg_ready = true;
        tp->state     = UDS_TP_IDLE;
        break;
    }

    /* ── First Frame ───────────────────────────────────────────────── */
    case UDS_TP_PCI_FF: {
        uint16_t ff_dl = (uint16_t)((data[0] & 0x0FU) << 8) | data[1];
        if (ff_dl > UDS_TP_RX_BUF_SIZE) {
            tp_send_fc(tp, UDS_TP_FC_OVFLW);
            return;
        }

        uint8_t copy_len = dlc - 2U;
        if (copy_len > ff_dl) copy_len = (uint8_t)ff_dl;
        memcpy(tp->rx_buf, &data[2], copy_len);

        tp->rx_len    = ff_dl;
        tp->rx_offset = copy_len;
        tp->rx_sn     = 1U;
        tp->msg_ready = false;
        tp->state     = UDS_TP_RX_IN_PROGRESS;
        tp->rx_timer  = UDS_GET_TICK_MS();

        tp_send_fc(tp, UDS_TP_FC_CTS);
        break;
    }

    /* ── Consecutive Frame ─────────────────────────────────────────── */
    case UDS_TP_PCI_CF: {
        if (tp->state != UDS_TP_RX_IN_PROGRESS) return;

        uint8_t sn = data[0] & 0x0FU;
        if (sn != (tp->rx_sn & 0x0FU)) {
            /* Sequence error – abort */
            tp->state = UDS_TP_IDLE;
            return;
        }
        tp->rx_sn = (tp->rx_sn + 1U) & 0x0FU;

        uint16_t remaining = tp->rx_len - tp->rx_offset;
        uint8_t  copy_len  = (remaining > (dlc - 1U))
                             ? (dlc - 1U)
                             : (uint8_t)remaining;
        memcpy(&tp->rx_buf[tp->rx_offset], &data[1], copy_len);
        tp->rx_offset += copy_len;
        tp->rx_timer   = UDS_GET_TICK_MS();

        if (tp->rx_offset >= tp->rx_len) {
            tp->msg_ready = true;
            tp->state     = UDS_TP_IDLE;
        }
        break;
    }

    /* ── Flow Control (for our TX side) ────────────────────────────── */
    case UDS_TP_PCI_FC: {
        if (tp->state != UDS_TP_TX_WAIT_FC) return;

        uint8_t fs = data[0] & 0x0FU;
        if (fs == UDS_TP_FC_OVFLW) {
            tp->state = UDS_TP_IDLE;
            return;
        }
        if (fs == UDS_TP_FC_WAIT) {
            tp->tx_timer = UDS_GET_TICK_MS();
            return;
        }
        /* CTS */
        tp->tx_bs    = (dlc > 1U) ? data[1] : 0U;
        tp->tx_stmin = (dlc > 2U) ? data[2] : 0U;
        tp->tx_bs_count = tp->tx_bs;
        tp->state    = UDS_TP_TX_IN_PROGRESS;
        tp->tx_timer = UDS_GET_TICK_MS();
        break;
    }

    default:
        break;
    }
}

void uds_tp_transmit(uds_tp_t *tp, const uint8_t *data, uint16_t len)
{
    if (len == 0U) return;

    /* Single Frame */
    if (len <= (UDS_CAN_DL - 1U)) {
        uint8_t frame[UDS_CAN_DL];
        memset(frame, 0xCC, sizeof(frame));
        frame[0] = UDS_TP_PCI_SF | (len & 0x0FU);
        memcpy(&frame[1], data, len);
        uds_tp_can_send(UDS_CAN_TX_ID, frame, UDS_CAN_DL);
        return;
    }

    /* Multi-frame: copy into TX buffer */
    if (len > UDS_TP_TX_BUF_SIZE) len = UDS_TP_TX_BUF_SIZE;
    memcpy(tp->tx_buf, data, len);
    tp->tx_len = len;

    /* Send First Frame */
    uint8_t frame[UDS_CAN_DL];
    memset(frame, 0xCC, sizeof(frame));
    frame[0] = UDS_TP_PCI_FF | (uint8_t)((len >> 8) & 0x0FU);
    frame[1] = (uint8_t)(len & 0xFFU);

    uint8_t ff_data_len = UDS_CAN_DL - 2U;
    if (ff_data_len > len) ff_data_len = (uint8_t)len;
    memcpy(&frame[2], data, ff_data_len);
    uds_tp_can_send(UDS_CAN_TX_ID, frame, UDS_CAN_DL);

    tp->tx_offset = ff_data_len;
    tp->tx_sn     = 1U;
    tp->state     = UDS_TP_TX_WAIT_FC;
    tp->tx_timer  = UDS_GET_TICK_MS();
}

void uds_tp_tick(uds_tp_t *tp)
{
    uint32_t now = UDS_GET_TICK_MS();

    /* RX timeout */
    if (tp->state == UDS_TP_RX_IN_PROGRESS) {
        if ((now - tp->rx_timer) >= UDS_TP_N_CR_TIMEOUT) {
            tp->state = UDS_TP_IDLE;
        }
    }

    /* TX: wait for FC timeout */
    if (tp->state == UDS_TP_TX_WAIT_FC) {
        if ((now - tp->tx_timer) >= UDS_TP_N_BS_TIMEOUT) {
            tp->state = UDS_TP_IDLE;
        }
    }

    /* TX: send consecutive frames with STmin pacing */
    if (tp->state == UDS_TP_TX_IN_PROGRESS) {
        if ((now - tp->tx_timer) >= tp->tx_stmin) {
            tp_send_next_cf(tp);
            tp->tx_timer = now;

            if (tp->tx_offset >= tp->tx_len) {
                tp->state = UDS_TP_IDLE;
                return;
            }

            /* Block size handling */
            if (tp->tx_bs != 0U) {
                tp->tx_bs_count--;
                if (tp->tx_bs_count == 0U) {
                    tp->state = UDS_TP_TX_WAIT_FC;
                    tp->tx_timer = now;
                }
            }
        }
    }
}
