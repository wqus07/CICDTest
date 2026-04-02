/**
 * @file    uds_service.c
 * @brief   UDS Diagnostic Service handler (ISO 14229-1)
 *
 * Implements the flash reprogramming sequence:
 *   DiagSessionControl(Programming) → SecurityAccess → RoutineControl(Erase)
 *   → RequestDownload → TransferData x N → RequestTransferExit
 *   → RoutineControl(CheckIntegrity) → ECUReset
 */
#include "uds_service.h"
#include <string.h>

/* ── Helpers ───────────────────────────────────────────────────────────── */

static void send_negative(uds_ctx_t *ctx, uint8_t sid, uint8_t nrc)
{
    ctx->resp_buf[0] = 0x7FU;
    ctx->resp_buf[1] = sid;
    ctx->resp_buf[2] = nrc;
    ctx->resp_len = 3U;
}

static void send_positive(uds_ctx_t *ctx, uint8_t sid, uint8_t sub)
{
    ctx->resp_buf[0] = UDS_POSITIVE_RSP(sid);
    ctx->resp_buf[1] = sub;
    ctx->resp_len = 2U;
}

static void refresh_s3(uds_ctx_t *ctx)
{
    ctx->s3_timer = UDS_GET_TICK_MS();
}

static bool check_security(uds_ctx_t *ctx, uint8_t sid)
{
    if (!ctx->security.unlocked) {
        send_negative(ctx, sid, UDS_NRC_SECURITY_ACCESS_DENIED);
        return false;
    }
    return true;
}

static bool check_session(uds_ctx_t *ctx, uint8_t sid, uds_session_t required)
{
    if (ctx->session != required) {
        send_negative(ctx, sid, UDS_NRC_SERVICE_NOT_IN_SESSION);
        return false;
    }
    return true;
}

/* ── Security seed/key (simple XOR – replace with production algorithm) ── */

static uint32_t generate_seed(void)
{
    /* Use SysTick + DWT cycle counter for entropy */
    uint32_t seed = UDS_GET_TICK_MS();
    seed ^= SysTick->VAL;
    seed ^= (seed << 13);
    seed ^= (seed >> 7);
    seed ^= (seed << 17);
    if (seed == 0U) seed = 0xDEADBEEFU;
    return seed;
}

static bool verify_key(uds_ctx_t *ctx, const uint8_t *key, uint8_t len)
{
    if (len != UDS_SA_KEY_LEN) return false;

    /* Simple algorithm: key = seed XOR 0x12345678 (replace for production) */
    uint32_t expected = ctx->security.seed[0] ^ 0x12345678U;
    uint32_t received;
    memcpy(&received, key, 4U);

    return (received == expected);
}

/* ── Service handlers ──────────────────────────────────────────────────── */

static void handle_session_control(uds_ctx_t *ctx,
                                   const uint8_t *data, uint16_t len)
{
    if (len < 2U) {
        send_negative(ctx, data[0], UDS_NRC_INCORRECT_MSG_LEN);
        return;
    }
    uint8_t sub = data[1] & 0x7FU;  /* Mask suppressPosRsp bit */

    switch (sub) {
    case UDS_SESSION_DEFAULT:
        ctx->session = UDS_SESSION_DEFAULT;
        ctx->security.unlocked = false;
        break;
    case UDS_SESSION_PROGRAMMING:
        ctx->session = UDS_SESSION_PROGRAMMING;
        ctx->security.unlocked = false;
        break;
    case UDS_SESSION_EXTENDED:
        ctx->session = UDS_SESSION_EXTENDED;
        break;
    default:
        send_negative(ctx, data[0], UDS_NRC_SUBFUNCTION_NOT_SUPP);
        return;
    }

    refresh_s3(ctx);

    /* Build positive response with P2/P2* timing */
    ctx->resp_buf[0] = UDS_POSITIVE_RSP(UDS_SID_DIAG_SESSION_CTRL);
    ctx->resp_buf[1] = sub;
    ctx->resp_buf[2] = (uint8_t)(UDS_P2_SERVER_MS >> 8);
    ctx->resp_buf[3] = (uint8_t)(UDS_P2_SERVER_MS & 0xFF);
    ctx->resp_buf[4] = (uint8_t)((UDS_P2_STAR_SERVER_MS / 10U) >> 8);
    ctx->resp_buf[5] = (uint8_t)((UDS_P2_STAR_SERVER_MS / 10U) & 0xFF);
    ctx->resp_len = 6U;
}

static void handle_ecu_reset(uds_ctx_t *ctx,
                             const uint8_t *data, uint16_t len)
{
    if (len < 2U) {
        send_negative(ctx, data[0], UDS_NRC_INCORRECT_MSG_LEN);
        return;
    }
    uint8_t sub = data[1] & 0x7FU;
    if (sub != 0x01U /* hardReset */ && sub != 0x03U /* softReset */) {
        send_negative(ctx, data[0], UDS_NRC_SUBFUNCTION_NOT_SUPP);
        return;
    }

    send_positive(ctx, UDS_SID_ECU_RESET, sub);

    /* Transmit response, then reset */
    uds_tp_transmit(&ctx->tp, ctx->resp_buf, ctx->resp_len);
    /* Small delay to allow CAN TX to complete */
    uint32_t t0 = UDS_GET_TICK_MS();
    while ((UDS_GET_TICK_MS() - t0) < 50U) { /* wait */ }
    UDS_SYSTEM_RESET();
}

static void handle_security_access(uds_ctx_t *ctx,
                                   const uint8_t *data, uint16_t len)
{
    if (len < 2U) {
        send_negative(ctx, data[0], UDS_NRC_INCORRECT_MSG_LEN);
        return;
    }
    if (!check_session(ctx, data[0], UDS_SESSION_PROGRAMMING)) return;

    uint8_t sub = data[1];
    uint32_t now = UDS_GET_TICK_MS();

    /* Check lock-out */
    if (ctx->security.attempt_count >= UDS_SA_MAX_ATTEMPTS) {
        if ((now - ctx->security.lock_timer) < UDS_SA_LOCK_TIME_MS) {
            send_negative(ctx, data[0], UDS_NRC_EXCEEDED_ATTEMPTS);
            return;
        }
        ctx->security.attempt_count = 0U;
    }

    if (sub == 0x01U) {
        /* RequestSeed */
        if (ctx->security.unlocked) {
            /* Already unlocked – return zero seed */
            ctx->resp_buf[0] = UDS_POSITIVE_RSP(UDS_SID_SECURITY_ACCESS);
            ctx->resp_buf[1] = 0x01U;
            memset(&ctx->resp_buf[2], 0, UDS_SA_SEED_LEN);
            ctx->resp_len = 2U + UDS_SA_SEED_LEN;
        } else {
            uint32_t seed = generate_seed();
            ctx->security.seed[0] = seed;
            ctx->security.seed_pending = true;

            ctx->resp_buf[0] = UDS_POSITIVE_RSP(UDS_SID_SECURITY_ACCESS);
            ctx->resp_buf[1] = 0x01U;
            memcpy(&ctx->resp_buf[2], &seed, UDS_SA_SEED_LEN);
            ctx->resp_len = 2U + UDS_SA_SEED_LEN;
        }
    }
    else if (sub == 0x02U) {
        /* SendKey */
        if (!ctx->security.seed_pending) {
            send_negative(ctx, data[0], UDS_NRC_REQUEST_SEQUENCE_ERROR);
            return;
        }
        if (len < (2U + UDS_SA_KEY_LEN)) {
            send_negative(ctx, data[0], UDS_NRC_INCORRECT_MSG_LEN);
            return;
        }

        ctx->security.seed_pending = false;

        if (verify_key(ctx, &data[2], UDS_SA_KEY_LEN)) {
            ctx->security.unlocked = true;
            ctx->security.attempt_count = 0U;
            send_positive(ctx, UDS_SID_SECURITY_ACCESS, 0x02U);
        } else {
            ctx->security.attempt_count++;
            if (ctx->security.attempt_count >= UDS_SA_MAX_ATTEMPTS) {
                ctx->security.lock_timer = now;
            }
            send_negative(ctx, data[0], UDS_NRC_INVALID_KEY);
        }
    }
    else {
        send_negative(ctx, data[0], UDS_NRC_SUBFUNCTION_NOT_SUPP);
    }
}

static void handle_read_data_by_id(uds_ctx_t *ctx,
                                   const uint8_t *data, uint16_t len)
{
    if (len < 3U) {
        send_negative(ctx, data[0], UDS_NRC_INCORRECT_MSG_LEN);
        return;
    }
    uint16_t did = ((uint16_t)data[1] << 8) | data[2];

    ctx->resp_buf[0] = UDS_POSITIVE_RSP(UDS_SID_READ_DATA_BY_ID);
    ctx->resp_buf[1] = data[1];
    ctx->resp_buf[2] = data[2];

    switch (did) {
    case UDS_DID_SW_VERSION:
        ctx->resp_buf[3] = UDS_SW_VERSION_YEAR;
        ctx->resp_buf[4] = UDS_SW_VERSION_MONTH;
        ctx->resp_buf[5] = UDS_SW_VERSION_DAY;
        ctx->resp_buf[6] = UDS_SW_VERSION_VARIANT;
        ctx->resp_len = 7U;
        break;
    case UDS_DID_ACTIVE_SESSION:
        ctx->resp_buf[3] = (uint8_t)ctx->session;
        ctx->resp_len = 4U;
        break;
    default:
        send_negative(ctx, data[0], UDS_NRC_REQUEST_OUT_OF_RANGE);
        return;
    }
}

static void handle_routine_control(uds_ctx_t *ctx,
                                   const uint8_t *data, uint16_t len)
{
    if (len < 4U) {
        send_negative(ctx, data[0], UDS_NRC_INCORRECT_MSG_LEN);
        return;
    }
    if (!check_session(ctx, data[0], UDS_SESSION_PROGRAMMING)) return;
    if (!check_security(ctx, data[0])) return;

    uint8_t  sub_fn = data[1];  /* 0x01=start, 0x02=stop, 0x03=result */
    uint16_t rid = ((uint16_t)data[2] << 8) | data[3];

    if (sub_fn != 0x01U) {
        send_negative(ctx, data[0], UDS_NRC_SUBFUNCTION_NOT_SUPP);
        return;
    }

    switch (rid) {
    case UDS_RID_ERASE_MEMORY:
        /* Erase is deferred to uds_flash_program(); mark ready */
        ctx->resp_buf[0] = UDS_POSITIVE_RSP(UDS_SID_ROUTINE_CONTROL);
        ctx->resp_buf[1] = sub_fn;
        ctx->resp_buf[2] = data[2];
        ctx->resp_buf[3] = data[3];
        ctx->resp_buf[4] = 0x00U;  /* routineStatusRecord: success */
        ctx->resp_len = 5U;
        break;

    case UDS_RID_CHECK_INTEGRITY: {
        /* Compute CRC of downloaded image */
        if (ctx->flash.download_active || ctx->flash.write_offset == 0U) {
            send_negative(ctx, data[0], UDS_NRC_CONDITIONS_NOT_CORRECT);
            return;
        }
        uint32_t crc = uds_flash_calc_crc(&ctx->flash);
        ctx->resp_buf[0] = UDS_POSITIVE_RSP(UDS_SID_ROUTINE_CONTROL);
        ctx->resp_buf[1] = sub_fn;
        ctx->resp_buf[2] = data[2];
        ctx->resp_buf[3] = data[3];
        ctx->resp_buf[4] = (uint8_t)(crc >> 24);
        ctx->resp_buf[5] = (uint8_t)(crc >> 16);
        ctx->resp_buf[6] = (uint8_t)(crc >> 8);
        ctx->resp_buf[7] = (uint8_t)(crc & 0xFF);
        ctx->resp_len = 8U;
        break;
    }

    case UDS_RID_CHECK_PROG_DEP:
        /* Check programming dependencies – always OK for now */
        ctx->resp_buf[0] = UDS_POSITIVE_RSP(UDS_SID_ROUTINE_CONTROL);
        ctx->resp_buf[1] = sub_fn;
        ctx->resp_buf[2] = data[2];
        ctx->resp_buf[3] = data[3];
        ctx->resp_buf[4] = 0x00U;
        ctx->resp_len = 5U;
        break;

    default:
        send_negative(ctx, data[0], UDS_NRC_REQUEST_OUT_OF_RANGE);
        return;
    }
}

static void handle_request_download(uds_ctx_t *ctx,
                                    const uint8_t *data, uint16_t len)
{
    if (!check_session(ctx, data[0], UDS_SESSION_PROGRAMMING)) return;
    if (!check_security(ctx, data[0])) return;

    /*
     * ISO 14229 RequestDownload format:
     *   [0] SID=0x34
     *   [1] dataFormatIdentifier (0x00 = uncompressed, unencrypted)
     *   [2] addressAndLengthFormatIdentifier
     *       high nibble = memorySize byte count
     *       low  nibble = memoryAddress byte count
     *   [3..] memoryAddress (big-endian)
     *   [...] memorySize   (big-endian)
     */
    if (len < 3U) {
        send_negative(ctx, data[0], UDS_NRC_INCORRECT_MSG_LEN);
        return;
    }

    uint8_t addr_len = data[2] & 0x0FU;
    uint8_t size_len = (data[2] >> 4) & 0x0FU;

    if (len < (3U + addr_len + size_len)) {
        send_negative(ctx, data[0], UDS_NRC_INCORRECT_MSG_LEN);
        return;
    }

    /* Parse address (big-endian) */
    uint32_t address = 0;
    for (uint8_t i = 0; i < addr_len; i++) {
        address = (address << 8) | data[3 + i];
    }

    /* Parse size (big-endian) */
    uint32_t size = 0;
    for (uint8_t i = 0; i < size_len; i++) {
        size = (size << 8) | data[3 + addr_len + i];
    }

    uds_flash_status_t rc = uds_flash_request_download(&ctx->flash,
                                                        address, size);
    if (rc != UDS_FLASH_OK) {
        send_negative(ctx, data[0], UDS_NRC_UPLOAD_DOWNLOAD_NOT_ACC);
        return;
    }

    ctx->block_counter = 1U;

    /* Positive response: max block length the ECU can accept per
     * TransferData call.  2 bytes length format. */
    uint16_t max_block = UDS_TP_RX_BUF_SIZE - 2U;  /* minus SID + counter */
    ctx->resp_buf[0] = UDS_POSITIVE_RSP(UDS_SID_REQUEST_DOWNLOAD);
    ctx->resp_buf[1] = 0x20U;  /* lengthFormatIdentifier: 2 bytes */
    ctx->resp_buf[2] = (uint8_t)(max_block >> 8);
    ctx->resp_buf[3] = (uint8_t)(max_block & 0xFF);
    ctx->resp_len = 4U;
}

static void handle_transfer_data(uds_ctx_t *ctx,
                                 const uint8_t *data, uint16_t len)
{
    if (!check_session(ctx, data[0], UDS_SESSION_PROGRAMMING)) return;
    if (!check_security(ctx, data[0])) return;

    if (len < 2U) {
        send_negative(ctx, data[0], UDS_NRC_INCORRECT_MSG_LEN);
        return;
    }
    if (!ctx->flash.download_active) {
        send_negative(ctx, data[0], UDS_NRC_REQUEST_SEQUENCE_ERROR);
        return;
    }

    /* Check block sequence counter */
    if (data[1] != ctx->block_counter) {
        send_negative(ctx, data[0], UDS_NRC_REQUEST_SEQUENCE_ERROR);
        return;
    }

    /* Copy payload (skip SID + blockCounter) */
    uds_flash_status_t rc = uds_flash_transfer_data(&ctx->flash,
                                                     &data[2], len - 2U);
    if (rc != UDS_FLASH_OK) {
        send_negative(ctx, data[0], UDS_NRC_TRANSFER_SUSPENDED);
        return;
    }

    ctx->resp_buf[0] = UDS_POSITIVE_RSP(UDS_SID_TRANSFER_DATA);
    ctx->resp_buf[1] = ctx->block_counter;
    ctx->resp_len = 2U;

    ctx->block_counter++;  /* Wraps 0xFF → 0x00 per spec */
}

static void handle_transfer_exit(uds_ctx_t *ctx,
                                 const uint8_t *data, uint16_t len)
{
    if (!check_session(ctx, data[0], UDS_SESSION_PROGRAMMING)) return;

    uds_flash_status_t rc = uds_flash_request_transfer_exit(&ctx->flash);
    if (rc != UDS_FLASH_OK) {
        send_negative(ctx, data[0], UDS_NRC_GENERAL_PROGRAMMING_ERR);
        return;
    }

    ctx->resp_buf[0] = UDS_POSITIVE_RSP(UDS_SID_REQUEST_TRANSFER_EXIT);
    ctx->resp_len = 1U;
}

static void handle_tester_present(uds_ctx_t *ctx,
                                  const uint8_t *data, uint16_t len)
{
    if (len < 2U) {
        send_negative(ctx, data[0], UDS_NRC_INCORRECT_MSG_LEN);
        return;
    }
    uint8_t sub = data[1] & 0x7FU;
    if (sub != 0x00U) {
        send_negative(ctx, data[0], UDS_NRC_SUBFUNCTION_NOT_SUPP);
        return;
    }
    refresh_s3(ctx);
    send_positive(ctx, UDS_SID_TESTER_PRESENT, 0x00U);
}

/* ── Init & Dispatch ───────────────────────────────────────────────────── */

void uds_service_init(uds_ctx_t *ctx)
{
    memset(ctx, 0, sizeof(*ctx));
    ctx->session = UDS_SESSION_DEFAULT;
    uds_tp_init(&ctx->tp);
    uds_flash_init(&ctx->flash);
    refresh_s3(ctx);
}

void uds_service_dispatch(uds_ctx_t *ctx, const uint8_t *data, uint16_t len)
{
    if (len == 0U) return;

    refresh_s3(ctx);
    ctx->resp_len = 0U;

    uint8_t sid = data[0];

    switch (sid) {
    case UDS_SID_DIAG_SESSION_CTRL:
        handle_session_control(ctx, data, len);
        break;
    case UDS_SID_ECU_RESET:
        handle_ecu_reset(ctx, data, len);
        return;  /* ECUReset sends response itself before reset */
    case UDS_SID_SECURITY_ACCESS:
        handle_security_access(ctx, data, len);
        break;
    case UDS_SID_READ_DATA_BY_ID:
        handle_read_data_by_id(ctx, data, len);
        break;
    case UDS_SID_ROUTINE_CONTROL:
        handle_routine_control(ctx, data, len);
        break;
    case UDS_SID_REQUEST_DOWNLOAD:
        handle_request_download(ctx, data, len);
        break;
    case UDS_SID_TRANSFER_DATA:
        handle_transfer_data(ctx, data, len);
        break;
    case UDS_SID_REQUEST_TRANSFER_EXIT:
        handle_transfer_exit(ctx, data, len);
        break;
    case UDS_SID_TESTER_PRESENT:
        handle_tester_present(ctx, data, len);
        break;
    default:
        send_negative(ctx, sid, UDS_NRC_SERVICE_NOT_SUPPORTED);
        break;
    }

    /* Check suppressPositiveResponse bit */
    if (ctx->resp_len > 0U && ctx->resp_buf[0] != 0x7FU) {
        if (len >= 2U && (data[1] & 0x80U)) {
            return;  /* Suppress positive response */
        }
    }

    /* Transmit response */
    if (ctx->resp_len > 0U) {
        uds_tp_transmit(&ctx->tp, ctx->resp_buf, ctx->resp_len);
    }
}

void uds_service_tick(uds_ctx_t *ctx)
{
    /* S3 session timeout */
    if (ctx->session != UDS_SESSION_DEFAULT) {
        if ((UDS_GET_TICK_MS() - ctx->s3_timer) >= UDS_S3_SERVER_TIMEOUT_MS) {
            ctx->session = UDS_SESSION_DEFAULT;
            ctx->security.unlocked = false;
        }
    }

    uds_tp_tick(&ctx->tp);
}
