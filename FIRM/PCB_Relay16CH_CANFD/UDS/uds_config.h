/**
 * @file    uds_config.h
 * @brief   UDS firmware update configuration (ISO 14229 / ISO 15765-2)
 *
 * Portable configuration header. Modify this file to adapt the UDS module
 * to different MCU platforms, CAN baud rates, and memory layouts.
 */
#ifndef UDS_CONFIG_H
#define UDS_CONFIG_H

#include <stdint.h>
#include "stm32h7xx_hal.h"

/* ──────────────────────────────────────────────────────────────────────────
 *  CAN Transport Configuration
 * ────────────────────────────────────────────────────────────────────────── */

/** Physical request  CAN-ID  (Tester → ECU) */
#define UDS_CAN_RX_ID               0x7E0U

/** Physical response CAN-ID  (ECU → Tester) */
#define UDS_CAN_TX_ID               0x7E8U

/** Functional request CAN-ID (broadcast) */
#define UDS_CAN_FUNC_ID             0x7DFU

/** 0 = standard 11-bit ID, 1 = extended 29-bit ID */
#define UDS_CAN_ID_TYPE             0

/** Maximum CAN frame payload (8 for classic CAN, 64 for CAN FD) */
#define UDS_CAN_DL                  8U

/* ──────────────────────────────────────────────────────────────────────────
 *  ISO 15765-2 (ISO-TP) Timing  (unit: ms)
 * ────────────────────────────────────────────────────────────────────────── */
#define UDS_TP_N_BS_TIMEOUT         1000U   /**< FC wait timeout            */
#define UDS_TP_N_CR_TIMEOUT         1000U   /**< Consecutive frame timeout  */
#define UDS_TP_STMIN_MS             5U      /**< STmin in Flow Control      */
#define UDS_TP_BLOCK_SIZE           0U      /**< BS in FC (0 = no limit)    */

/** ISO-TP reassembly buffer – must hold the largest UDS message (firmware
 *  blocks).  RequestDownload negotiates max block size within this limit. */
#define UDS_TP_RX_BUF_SIZE          4104U   /**< 4 KB + 8 overhead          */
#define UDS_TP_TX_BUF_SIZE          256U    /**< Response buffer             */

/* ──────────────────────────────────────────────────────────────────────────
 *  UDS Session / Timing
 * ────────────────────────────────────────────────────────────────────────── */
#define UDS_S3_SERVER_TIMEOUT_MS    5000U   /**< Session timeout (no TesterPresent) */
#define UDS_P2_SERVER_MS            50U     /**< Max response time                  */
#define UDS_P2_STAR_SERVER_MS       5000U   /**< Extended response time (0x78 NRC)  */

/* ──────────────────────────────────────────────────────────────────────────
 *  Security Access
 * ────────────────────────────────────────────────────────────────────────── */
#define UDS_SA_SEED_LEN             4U      /**< Seed length in bytes       */
#define UDS_SA_KEY_LEN              4U      /**< Key  length in bytes       */
#define UDS_SA_MAX_ATTEMPTS         3U      /**< Lock after N wrong keys    */
#define UDS_SA_LOCK_TIME_MS         10000U  /**< Lock duration              */

/* ──────────────────────────────────────────────────────────────────────────
 *  Flash / Memory Layout  (STM32H750 specific)
 * ────────────────────────────────────────────────────────────────────────── */

/** Internal flash start */
#define UDS_FLASH_BASE              0x08000000U

/** Internal flash size (128 KB on H750) */
#define UDS_FLASH_SIZE              (128U * 1024U)

/** Minimum flash-writable unit on STM32H7 = 32 bytes (256-bit flash word) */
#define UDS_FLASH_WRITE_GRANULARITY 32U

/** RAM buffer for downloaded image.
 *  Located in AXI-SRAM (0x24000000, 512 KB).  Offset 64 KB to leave
 *  room for normal application data. */
#define UDS_RAM_BUFFER_ADDR         0x24010000U
#define UDS_RAM_BUFFER_SIZE         (128U * 1024U)

/* ──────────────────────────────────────────────────────────────────────────
 *  Software Version (reported via ReadDataByIdentifier 0xF195)
 * ────────────────────────────────────────────────────────────────────────── */
#define UDS_SW_VERSION_YEAR         24
#define UDS_SW_VERSION_MONTH        10
#define UDS_SW_VERSION_DAY          31
#define UDS_SW_VERSION_VARIANT      0

/* ──────────────────────────────────────────────────────────────────────────
 *  Routine IDs  (RoutineControl 0x31)
 * ────────────────────────────────────────────────────────────────────────── */
#define UDS_RID_ERASE_MEMORY        0xFF00U /**< Erase flash                */
#define UDS_RID_CHECK_PROG_DEP      0xFF01U /**< Check programming deps     */
#define UDS_RID_CHECK_INTEGRITY     0x0202U /**< Verify CRC of downloaded   */

/* ──────────────────────────────────────────────────────────────────────────
 *  Platform Abstraction Macros  (override for non-STM32 targets)
 * ────────────────────────────────────────────────────────────────────────── */

/** Place function in RAM (required for flash self-programming on H750) */
#ifndef UDS_RAMFUNC
#define UDS_RAMFUNC  __attribute__((long_call, section(".RamFunc")))
#endif

/** Get millisecond tick (wraps at 2^32) */
#ifndef UDS_GET_TICK_MS
#define UDS_GET_TICK_MS()           HAL_GetTick()
#endif

/** Disable / enable all interrupts */
#ifndef UDS_ENTER_CRITICAL
#define UDS_ENTER_CRITICAL()        __disable_irq()
#endif
#ifndef UDS_EXIT_CRITICAL
#define UDS_EXIT_CRITICAL()         __enable_irq()
#endif

/** System reset */
#ifndef UDS_SYSTEM_RESET
#define UDS_SYSTEM_RESET()          NVIC_SystemReset()
#endif

#endif /* UDS_CONFIG_H */
