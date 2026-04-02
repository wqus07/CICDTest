/**
 * @file    uds_flash.c
 * @brief   Flash driver for UDS firmware update (STM32H750)
 *
 * STM32H750 has a single 128 KB flash sector.  Erasing wipes the entire
 * bank, so the full image must be buffered in RAM before programming.
 *
 * Functions tagged UDS_RAMFUNC execute from SRAM because the flash is
 * inaccessible during erase/program operations.
 */
#include "uds_flash.h"
#include "stm32h7xx_hal.h"
#include <string.h>

extern CRC_HandleTypeDef hcrc;

/* ── Init ──────────────────────────────────────────────────────────────── */

void uds_flash_init(uds_flash_ctx_t *ctx)
{
    memset(ctx, 0, sizeof(*ctx));
    ctx->ram_buf  = (uint8_t *)UDS_RAM_BUFFER_ADDR;
    ctx->buf_size = UDS_RAM_BUFFER_SIZE;
}

/* ── Download API ──────────────────────────────────────────────────────── */

uds_flash_status_t uds_flash_request_download(uds_flash_ctx_t *ctx,
                                               uint32_t target_addr,
                                               uint32_t image_size)
{
    if (target_addr < UDS_FLASH_BASE ||
        (target_addr + image_size) > (UDS_FLASH_BASE + UDS_FLASH_SIZE)) {
        return UDS_FLASH_ERR_ADDR;
    }
    if ((target_addr & (UDS_FLASH_WRITE_GRANULARITY - 1U)) != 0U) {
        return UDS_FLASH_ERR_ALIGN;
    }
    if (image_size > ctx->buf_size) {
        return UDS_FLASH_ERR_SIZE;
    }

    /* Clear buffer and prepare context */
    memset(ctx->ram_buf, 0xFF, ctx->buf_size);
    ctx->target_addr    = target_addr;
    ctx->image_size     = image_size;
    ctx->write_offset   = 0U;
    ctx->download_active = true;

    return UDS_FLASH_OK;
}

uds_flash_status_t uds_flash_transfer_data(uds_flash_ctx_t *ctx,
                                            const uint8_t *data,
                                            uint16_t len)
{
    if (!ctx->download_active) {
        return UDS_FLASH_ERR_ADDR;
    }
    if ((ctx->write_offset + len) > ctx->image_size) {
        return UDS_FLASH_ERR_SIZE;
    }

    memcpy(&ctx->ram_buf[ctx->write_offset], data, len);
    ctx->write_offset += len;

    return UDS_FLASH_OK;
}

uds_flash_status_t uds_flash_request_transfer_exit(uds_flash_ctx_t *ctx)
{
    if (!ctx->download_active) {
        return UDS_FLASH_ERR_ADDR;
    }
    if (ctx->write_offset < ctx->image_size) {
        return UDS_FLASH_ERR_SIZE;
    }

    ctx->download_active = false;
    return UDS_FLASH_OK;
}

/* ── CRC ───────────────────────────────────────────────────────────────── */

uint32_t uds_flash_calc_crc(const uds_flash_ctx_t *ctx)
{
    /* Use STM32 hardware CRC peripheral */
    __HAL_CRC_DR_RESET(&hcrc);
    return HAL_CRC_Calculate(&hcrc, (uint32_t *)ctx->ram_buf,
                             (ctx->write_offset + 3U) / 4U);
}

/* ── Flash operations (run from RAM) ───────────────────────────────────── */

UDS_RAMFUNC uds_flash_status_t uds_flash_erase_bank(void)
{
    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef erase_cfg;
    erase_cfg.TypeErase = FLASH_TYPEERASE_MASSERASE;
    erase_cfg.Banks     = FLASH_BANK_1;
    erase_cfg.VoltageRange = FLASH_VOLTAGE_RANGE_3;  /* 2.7 – 3.6 V */

    uint32_t sector_error = 0;
    HAL_StatusTypeDef hal_rc = HAL_FLASHEx_Erase(&erase_cfg, &sector_error);

    HAL_FLASH_Lock();

    return (hal_rc == HAL_OK) ? UDS_FLASH_OK : UDS_FLASH_ERR_ERASE;
}

UDS_RAMFUNC uds_flash_status_t uds_flash_write(uint32_t addr,
                                                const uint8_t *data,
                                                uint32_t len)
{
    HAL_FLASH_Unlock();

    /* Round up to flash word boundary (32 bytes) */
    uint32_t aligned_len = (len + UDS_FLASH_WRITE_GRANULARITY - 1U)
                           & ~(UDS_FLASH_WRITE_GRANULARITY - 1U);

    for (uint32_t offset = 0; offset < aligned_len;
         offset += UDS_FLASH_WRITE_GRANULARITY)
    {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD,
                              addr + offset,
                              (uint32_t)(uintptr_t)&data[offset]) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return UDS_FLASH_ERR_WRITE;
        }
    }

    HAL_FLASH_Lock();

    /* Verify */
    if (memcmp((const void *)addr, data, len) != 0) {
        return UDS_FLASH_ERR_VERIFY;
    }

    return UDS_FLASH_OK;
}

/* ── High-level program routine ────────────────────────────────────────── */

uds_flash_status_t uds_flash_program(uds_flash_ctx_t *ctx)
{
    if (ctx->download_active || ctx->write_offset == 0U) {
        return UDS_FLASH_ERR_ADDR;
    }

    UDS_ENTER_CRITICAL();

    /* Disable caches before flash operations */
    SCB_DisableICache();
    SCB_DisableDCache();

    /* Step 1: Erase entire flash bank */
    uds_flash_status_t rc = uds_flash_erase_bank();
    if (rc != UDS_FLASH_OK) {
        UDS_EXIT_CRITICAL();
        return rc;
    }

    /* Step 2: Program from RAM buffer to flash */
    rc = uds_flash_write(ctx->target_addr, ctx->ram_buf, ctx->write_offset);
    if (rc != UDS_FLASH_OK) {
        UDS_EXIT_CRITICAL();
        return rc;
    }

    /* Step 3: Reset to boot new firmware */
    UDS_SYSTEM_RESET();

    /* Never reached */
    return UDS_FLASH_OK;
}
