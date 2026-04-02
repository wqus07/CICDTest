/**
 * @file    uds_flash.h
 * @brief   Flash driver abstraction for UDS firmware update
 *
 * STM32H750 has a single 128 KB flash bank that cannot be partially erased.
 * Strategy: download image into RAM buffer first, then erase + write in one
 * shot with interrupts disabled.
 */
#ifndef UDS_FLASH_H
#define UDS_FLASH_H

#include <stdint.h>
#include <stdbool.h>
#include "uds_config.h"

/* ── Return codes ──────────────────────────────────────────────────────── */
typedef enum {
    UDS_FLASH_OK = 0,
    UDS_FLASH_ERR_ADDR,         /**< Address out of range             */
    UDS_FLASH_ERR_ALIGN,        /**< Address not aligned              */
    UDS_FLASH_ERR_SIZE,         /**< Size exceeds RAM buffer          */
    UDS_FLASH_ERR_ERASE,        /**< Flash erase failed               */
    UDS_FLASH_ERR_WRITE,        /**< Flash program failed             */
    UDS_FLASH_ERR_VERIFY,       /**< Readback verify failed           */
    UDS_FLASH_ERR_CRC,          /**< CRC mismatch                     */
} uds_flash_status_t;

/* ── Download context ──────────────────────────────────────────────────── */
typedef struct {
    uint8_t *ram_buf;           /**< Pointer to RAM buffer            */
    uint32_t buf_size;          /**< Total buffer capacity            */
    uint32_t write_offset;      /**< Current write position in buffer */
    uint32_t target_addr;       /**< Flash start address for image    */
    uint32_t image_size;        /**< Expected total image size        */
    bool     download_active;   /**< Download session in progress     */
} uds_flash_ctx_t;

/* ── Public API ────────────────────────────────────────────────────────── */

/**
 * @brief  Initialise flash context.
 * @param  ctx  Flash context to initialise
 */
void uds_flash_init(uds_flash_ctx_t *ctx);

/**
 * @brief  Prepare for a new download.
 * @param  ctx          Flash context
 * @param  target_addr  Flash address where image will be written
 * @param  image_size   Expected total size in bytes
 * @return UDS_FLASH_OK on success
 */
uds_flash_status_t uds_flash_request_download(uds_flash_ctx_t *ctx,
                                               uint32_t target_addr,
                                               uint32_t image_size);

/**
 * @brief  Append a block of data to the RAM buffer.
 * @param  ctx   Flash context
 * @param  data  Data to copy
 * @param  len   Number of bytes
 * @return UDS_FLASH_OK on success
 */
uds_flash_status_t uds_flash_transfer_data(uds_flash_ctx_t *ctx,
                                            const uint8_t *data,
                                            uint16_t len);

/**
 * @brief  Finalise download – verify received size matches expected.
 * @param  ctx  Flash context
 * @return UDS_FLASH_OK if complete
 */
uds_flash_status_t uds_flash_request_transfer_exit(uds_flash_ctx_t *ctx);

/**
 * @brief  Erase flash bank and program the downloaded image.
 *         Runs from RAM, interrupts disabled.  Does NOT return on success
 *         — triggers a system reset to boot the new firmware.
 * @param  ctx  Flash context (download must be complete)
 * @return Only returns on error
 */
uds_flash_status_t uds_flash_program(uds_flash_ctx_t *ctx);

/**
 * @brief  Compute CRC-32 over the RAM buffer using hardware CRC.
 * @param  ctx  Flash context
 * @return CRC-32 value
 */
uint32_t uds_flash_calc_crc(const uds_flash_ctx_t *ctx);

/**
 * @brief  Erase the entire flash bank (must run from RAM).
 * @return UDS_FLASH_OK on success
 */
UDS_RAMFUNC uds_flash_status_t uds_flash_erase_bank(void);

/**
 * @brief  Write data from RAM buffer to flash (must run from RAM).
 * @param  addr  Flash address (must be 32-byte aligned)
 * @param  data  Source buffer
 * @param  len   Length in bytes (rounded up to 32-byte boundary)
 * @return UDS_FLASH_OK on success
 */
UDS_RAMFUNC uds_flash_status_t uds_flash_write(uint32_t addr,
                                                const uint8_t *data,
                                                uint32_t len);

#endif /* UDS_FLASH_H */
