/**
 * @file    uds_service.h
 * @brief   UDS Diagnostic Services for firmware update (ISO 14229-1)
 *
 * Implements the minimum service set required for CANoe / ODX-based
 * flash reprogramming:
 *
 *   0x10  DiagnosticSessionControl  (Default / Programming)
 *   0x11  ECUReset
 *   0x27  SecurityAccess
 *   0x22  ReadDataByIdentifier
 *   0x31  RoutineControl            (EraseMemory / CheckIntegrity)
 *   0x34  RequestDownload
 *   0x36  TransferData
 *   0x37  RequestTransferExit
 *   0x3E  TesterPresent
 */
#ifndef UDS_SERVICE_H
#define UDS_SERVICE_H

#include <stdint.h>
#include <stdbool.h>
#include "uds_config.h"
#include "uds_tp.h"
#include "uds_flash.h"

/* ── UDS Service IDs ───────────────────────────────────────────────────── */
#define UDS_SID_DIAG_SESSION_CTRL       0x10U
#define UDS_SID_ECU_RESET               0x11U
#define UDS_SID_READ_DATA_BY_ID         0x22U
#define UDS_SID_SECURITY_ACCESS         0x27U
#define UDS_SID_ROUTINE_CONTROL         0x31U
#define UDS_SID_REQUEST_DOWNLOAD        0x34U
#define UDS_SID_TRANSFER_DATA           0x36U
#define UDS_SID_REQUEST_TRANSFER_EXIT   0x37U
#define UDS_SID_TESTER_PRESENT          0x3EU

/** Positive response = SID + 0x40 */
#define UDS_POSITIVE_RSP(sid)           ((sid) + 0x40U)

/* ── UDS NRC (Negative Response Codes) ─────────────────────────────────── */
#define UDS_NRC_GENERAL_REJECT          0x10U
#define UDS_NRC_SERVICE_NOT_SUPPORTED   0x11U
#define UDS_NRC_SUBFUNCTION_NOT_SUPP    0x12U
#define UDS_NRC_INCORRECT_MSG_LEN       0x13U
#define UDS_NRC_CONDITIONS_NOT_CORRECT  0x22U
#define UDS_NRC_REQUEST_SEQUENCE_ERROR  0x24U
#define UDS_NRC_REQUEST_OUT_OF_RANGE    0x31U
#define UDS_NRC_SECURITY_ACCESS_DENIED  0x33U
#define UDS_NRC_INVALID_KEY             0x35U
#define UDS_NRC_EXCEEDED_ATTEMPTS       0x36U
#define UDS_NRC_UPLOAD_DOWNLOAD_NOT_ACC 0x70U
#define UDS_NRC_TRANSFER_SUSPENDED      0x71U
#define UDS_NRC_GENERAL_PROGRAMMING_ERR 0x72U
#define UDS_NRC_RESPONSE_PENDING        0x78U
#define UDS_NRC_SERVICE_NOT_IN_SESSION  0x7FU

/* ── Session types ─────────────────────────────────────────────────────── */
typedef enum {
    UDS_SESSION_DEFAULT     = 0x01U,
    UDS_SESSION_PROGRAMMING = 0x02U,
    UDS_SESSION_EXTENDED    = 0x03U,
} uds_session_t;

/* ── DID identifiers ──────────────────────────────────────────────────── */
#define UDS_DID_SW_VERSION              0xF195U
#define UDS_DID_BOOT_SW_ID             0xF180U
#define UDS_DID_ACTIVE_SESSION         0xF186U

/* ── Security state ────────────────────────────────────────────────────── */
typedef struct {
    bool     unlocked;
    uint8_t  attempt_count;
    uint32_t lock_timer;        /**< Timestamp when locked              */
    uint32_t seed[1];           /**< Last generated seed                */
    bool     seed_pending;      /**< Seed was sent, waiting for key     */
} uds_security_t;

/* ── Service handler context ───────────────────────────────────────────── */
typedef struct {
    uds_session_t   session;
    uds_security_t  security;
    uds_flash_ctx_t flash;
    uds_tp_t        tp;

    uint32_t        s3_timer;       /**< Session timeout watchdog       */
    uint8_t         block_counter;  /**< TransferData block sequence    */

    uint8_t         resp_buf[UDS_TP_TX_BUF_SIZE];
    uint16_t        resp_len;
} uds_ctx_t;

/* ── Public API ────────────────────────────────────────────────────────── */

/**
 * @brief  Initialise UDS service layer.
 * @param  ctx  Service context
 */
void uds_service_init(uds_ctx_t *ctx);

/**
 * @brief  Process a received UDS request message.
 * @param  ctx   Service context
 * @param  data  Raw UDS request bytes
 * @param  len   Request length
 */
void uds_service_dispatch(uds_ctx_t *ctx, const uint8_t *data, uint16_t len);

/**
 * @brief  1 ms periodic tick – session timeout monitoring.
 * @param  ctx  Service context
 */
void uds_service_tick(uds_ctx_t *ctx);

#endif /* UDS_SERVICE_H */
