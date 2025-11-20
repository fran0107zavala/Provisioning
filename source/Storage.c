/**
 * @file storage.c
 *
 * @brief Non-volatile storage implementation for WiFi credentials
 * Para FRDM-RW612 - MEJORADO V2.0
 *
 * MEJORAS:
 * - Offset 0x02000000 (no protegido)
 * - Mejor detección de errores
 * - Modo failsafe
 * - Más debugging
 * - Recuperación automática
 *
 * Copyright 2025 NXP
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "storage.h"
#include "fsl_debug_console.h"
#include "mflash_drv.h"
#include <string.h>
#include <stdint.h>

/* FreeRTOS includes para vTaskDelay */
#include "FreeRTOS.h"
#include "task.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define STORAGE_MAGIC_NUMBER    0xDEADBEEF
#define STORAGE_VERSION         0x01

/* 🔧 OFFSET MEJORADO - NO PROTEGIDO
 * FRDM-RW612: 64MB FLASH (0x00000000 to 0x03FFFFFF)
 *
 * ❌ Offset anterior (0x01FFE000) = ERROR 7002 (protegido por bootloader)
 * ✅ Nuevo offset (0x02000000) = Mitad del flash, generalmente libre
 *
 * Validaciones:
 * ✅ ALINEADO: 0x02000000 / 0x1000 = 0x2000 (múltiplo exacto de 4KB)
 * ✅ EN RANGO: 0x02000000 <= 0x03FFFFFF (dentro de 64MB)
 * ✅ NO PROTEGIDO: En zona media (evita bootloader y áreas protegidas)
 * ✅ DISTANCIA: 32MB offset (suficientemente lejos de bootloader)
 */
#define STORAGE_FLASH_OFFSET    0x02000000

#define STORAGE_FLASH_SIZE      4096

/* Número máximo de reintentos para operaciones FLASH */
#define FLASH_ERASE_RETRIES     5
#define FLASH_WRITE_RETRIES     5

/* Delay entre reintentos (ms) */
#define FLASH_RETRY_DELAY_MS    200

/* Storage layout */
typedef struct {
    uint32_t magic;                              /* Magic number for validation */
    uint8_t version;                             /* Version byte */
    uint8_t reserved[3];                         /* Padding */
    wifi_config_t config;                        /* WiFi configuration */
    uint32_t crc32;                              /* CRC32 checksum */
} storage_header_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

static uint8_t g_storage_buffer[STORAGE_FLASH_SIZE] __attribute__((aligned(4)));
static storage_header_t g_storage_data;
static bool g_storage_valid = false;
static bool g_storage_initialized = false;
static bool g_storage_flash_accessible = false;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static uint32_t crc32_calculate(const uint8_t *data, size_t length);
static bool storage_read_from_flash(void);
static bool storage_verify_flash_access(void);
static void storage_print_offset_info(void);

/*******************************************************************************
 * Code
 ******************************************************************************/

/**
 * @brief Imprimir información del offset y configuración
 */
static void storage_print_offset_info(void)
{
    PRINTF("[storage] ========================================\r\n");
    PRINTF("[storage] FLASH STORAGE CONFIGURATION\r\n");
    PRINTF("[storage] Physical offset: 0x%08X\r\n", (unsigned int)STORAGE_FLASH_OFFSET);
    PRINTF("[storage] Storage size: %d bytes\r\n", STORAGE_FLASH_SIZE);
    PRINTF("[storage] Sector size: %d bytes\r\n", MFLASH_SECTOR_SIZE);
    PRINTF("[storage] Page size: %d bytes\r\n", MFLASH_PAGE_SIZE);
    PRINTF("[storage] Total FLASH: 0x%08X (%d MB)\r\n",
           (unsigned int)MFLASH_BSIZE, (unsigned int)(MFLASH_BSIZE / 1024 / 1024));

    /* Mostrar si está alineado */
    if (STORAGE_FLASH_OFFSET % MFLASH_SECTOR_SIZE == 0)
        PRINTF("[storage] ✓ Offset is sector-aligned\r\n");
    else
        PRINTF("[storage] ✗ ERROR: Offset NOT aligned!\r\n");

    /* Mostrar si está en rango */
    if (STORAGE_FLASH_OFFSET < MFLASH_BSIZE)
        PRINTF("[storage] ✓ Offset is within FLASH range\r\n");
    else
        PRINTF("[storage] ✗ ERROR: Offset exceeds FLASH size!\r\n");

    PRINTF("[storage] ========================================\r\n");
}

/**
 * @brief Verificar accesibilidad del FLASH
 */
static bool storage_verify_flash_access(void)
{
    PRINTF("[storage] Verifying FLASH access...\r\n");

    /* Verificar alineación */
    if (STORAGE_FLASH_OFFSET % MFLASH_SECTOR_SIZE != 0)
    {
        PRINTF("[storage] ✗ ERROR: Offset NOT aligned to sector size (0x%08X)!\r\n",
               (unsigned int)STORAGE_FLASH_OFFSET);
        return false;
    }

    /* Verificar rango */
    if (STORAGE_FLASH_OFFSET >= MFLASH_BSIZE)
    {
        PRINTF("[storage] ✗ ERROR: Offset exceeds FLASH size!\r\n");
        return false;
    }

    /* Intentar mapear memoria */
    void *flash_ptr = mflash_drv_phys2log(STORAGE_FLASH_OFFSET, STORAGE_FLASH_SIZE);

    if (flash_ptr == NULL || (uintptr_t)flash_ptr == UINT32_MAX)
    {
        PRINTF("[storage] ✗ ERROR: Could not map FLASH address\r\n");
        return false;
    }

    PRINTF("[storage] ✓ FLASH access verified\r\n");
    PRINTF("[storage] Virtual address: 0x%08X\r\n", (unsigned int)flash_ptr);

    return true;
}

/**
 * @brief Simple CRC32 calculation
 */
static uint32_t crc32_calculate(const uint8_t *data, size_t length)
{
    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; i < length; i++)
    {
        crc ^= data[i];
        for (int j = 0; j < 8; j++)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc >>= 1;
        }
    }

    return crc ^ 0xFFFFFFFF;
}

/**
 * @brief Read configuration directly from memory-mapped FLASH
 */
static bool storage_read_from_flash(void)
{
    PRINTF("[storage] ========================================\r\n");
    PRINTF("[storage] Reading from FLASH...\r\n");

    /* Verificar acceso */
    if (!storage_verify_flash_access())
    {
        g_storage_initialized = true;
        g_storage_valid = false;
        g_storage_flash_accessible = false;
        return false;
    }

    g_storage_flash_accessible = true;

    /* Get memory-mapped address */
    void *flash_ptr = mflash_drv_phys2log(STORAGE_FLASH_OFFSET, STORAGE_FLASH_SIZE);

    if (flash_ptr == NULL)
    {
        PRINTF("[storage] ✗ ERROR: Could not map FLASH address\r\n");
        g_storage_initialized = true;
        g_storage_valid = false;
        return false;
    }

    /* Read directly from FLASH (it's memory-mapped) */
    memcpy(g_storage_buffer, flash_ptr, STORAGE_FLASH_SIZE);
    const storage_header_t *flash_data = (const storage_header_t *)g_storage_buffer;

    /* Check if FLASH is erased (all 0xFF) */
    if (flash_data->magic == 0xFFFFFFFF)
    {
        PRINTF("[storage] ℹ FLASH is erased (empty)\r\n");
        g_storage_initialized = true;
        g_storage_valid = false;
        return false;
    }

    /* Verify magic number */
    if (flash_data->magic != STORAGE_MAGIC_NUMBER)
    {
        PRINTF("[storage] ✗ Invalid magic number: 0x%08X (expected: 0x%08X)\r\n",
               (unsigned int)flash_data->magic, (unsigned int)STORAGE_MAGIC_NUMBER);
        g_storage_initialized = true;
        g_storage_valid = false;
        return false;
    }

    PRINTF("[storage] ✓ Valid magic number found\r\n");

    /* Validate CRC */
    uint32_t calculated_crc = crc32_calculate(
        (uint8_t *)&flash_data->config,
        sizeof(wifi_config_t)
    );

    if (calculated_crc != flash_data->crc32)
    {
        PRINTF("[storage] ✗ CRC validation failed (expected: 0x%08X, got: 0x%08X)\r\n",
               (unsigned int)flash_data->crc32, (unsigned int)calculated_crc);
        g_storage_initialized = true;
        g_storage_valid = false;
        return false;
    }

    PRINTF("[storage] ✓ CRC32 validation passed\r\n");

    /* Validate SSID is not empty */
    if (strlen(flash_data->config.ssid) == 0)
    {
        PRINTF("[storage] ✗ ERROR: SSID is empty\r\n");
        g_storage_initialized = true;
        g_storage_valid = false;
        return false;
    }

    /* Cache the data */
    memcpy(&g_storage_data, flash_data, sizeof(storage_header_t));
    g_storage_initialized = true;
    g_storage_valid = true;

    PRINTF("[storage] ✓ Valid config found\r\n");
    PRINTF("[storage] SSID: %s\r\n", g_storage_data.config.ssid);
    PRINTF("[storage] ========================================\r\n");
    return true;
}

/**
 * @brief Check if valid WiFi configuration exists in FLASH
 */
bool storage_has_valid_config(void)
{
    PRINTF("[storage] storage_has_valid_config() called\r\n");

    /* Read from FLASH if not already done */
    if (!g_storage_initialized)
    {
        PRINTF("[storage] First time - reading from FLASH\r\n");
        storage_read_from_flash();
    }

    return g_storage_valid;
}

/**
 * @brief Read WiFi configuration from FLASH
 */
bool storage_read_wifi_config(wifi_config_t *config)
{
    PRINTF("[storage] storage_read_wifi_config() called\r\n");

    if (config == NULL)
    {
        PRINTF("[storage] ✗ ERROR: config pointer is NULL\r\n");
        return false;
    }

    /* Ensure we've read from FLASH */
    if (!g_storage_initialized)
    {
        storage_read_from_flash();
    }

    if (!g_storage_valid)
    {
        PRINTF("[storage] ✗ ERROR: No valid config available\r\n");
        return false;
    }

    /* Copy configuration */
    memcpy(config, &g_storage_data.config, sizeof(wifi_config_t));

    PRINTF("[storage] ✓ Configuration read successfully\r\n");
    PRINTF("[storage] SSID: %s\r\n", config->ssid);

    return true;
}

/**
 * @brief Write WiFi configuration to FLASH with retry logic
 */
bool storage_write_wifi_config(const wifi_config_t *config)
{
    PRINTF("[storage] ========================================\r\n");
    PRINTF("[storage] storage_write_wifi_config() called\r\n");

    if (config == NULL)
    {
        PRINTF("[storage] ✗ ERROR: config pointer is NULL\r\n");
        return false;
    }

    /* Validate input */
    if (strlen(config->ssid) == 0 || strlen(config->ssid) > WIFI_SSID_MAX_LEN)
    {
        PRINTF("[storage] ✗ ERROR: Invalid SSID length: %d\r\n", (int)strlen(config->ssid));
        return false;
    }

    if (strlen(config->password) > WIFI_PASS_MAX_LEN)
    {
        PRINTF("[storage] ✗ ERROR: Password too long: %d\r\n", (int)strlen(config->password));
        return false;
    }

    PRINTF("[storage] Validating credentials...\r\n");
    PRINTF("[storage] SSID: %s\r\n", config->ssid);
    PRINTF("[storage] Password length: %d\r\n", (int)strlen(config->password));

    /* Prepare header */
    storage_header_t header;
    memset(&header, 0, sizeof(storage_header_t));

    header.magic = STORAGE_MAGIC_NUMBER;
    header.version = STORAGE_VERSION;
    memcpy(&header.config, config, sizeof(wifi_config_t));

    /* Calculate CRC32 */
    header.crc32 = crc32_calculate(
        (uint8_t *)&header.config,
        sizeof(wifi_config_t)
    );

    PRINTF("[storage] CRC32: 0x%08X\r\n", (unsigned int)header.crc32);

    /* Prepare sector buffer */
    memset(g_storage_buffer, 0xFF, STORAGE_FLASH_SIZE);
    memcpy(g_storage_buffer, &header, sizeof(storage_header_t));

    /* Debug: print offset information */
    PRINTF("[storage] ----------------------------------------\r\n");
    PRINTF("[storage] ERASE OPERATION\r\n");
    storage_print_offset_info();

    /* Verify offset is aligned */
    if (STORAGE_FLASH_OFFSET % MFLASH_SECTOR_SIZE != 0)
    {
        PRINTF("[storage] ✗ ERROR: Offset NOT aligned to sector size!\r\n");
        return false;
    }

    /* Check if offset is within valid range */
    if (STORAGE_FLASH_OFFSET >= MFLASH_BSIZE)
    {
        PRINTF("[storage] ✗ ERROR: Offset 0x%08X exceeds FLASH size 0x%08X\r\n",
               (unsigned int)STORAGE_FLASH_OFFSET, (unsigned int)MFLASH_BSIZE);
        return false;
    }

    /* Erase sector with retries */
    int32_t erase_result = -1;
    for (int attempt = 0; attempt < FLASH_ERASE_RETRIES; attempt++)
    {
        PRINTF("[storage] Erase attempt %d/%d...\r\n", (attempt + 1), FLASH_ERASE_RETRIES);
        erase_result = mflash_drv_sector_erase(STORAGE_FLASH_OFFSET);

        if (erase_result == 0)
        {
            PRINTF("[storage] ✓ Sector erased successfully\r\n");
            break;
        }
        else
        {
            PRINTF("[storage] ⚠ Erase failed (attempt %d), error: %d (0x%08X)\r\n",
                   (attempt + 1), (int)erase_result, (unsigned int)erase_result);

            /* Esperar más tiempo antes de reintentar */
            vTaskDelay(pdMS_TO_TICKS(FLASH_RETRY_DELAY_MS));
        }
    }

    if (erase_result != 0)
    {
        PRINTF("[storage] ✗ ERROR: mflash_drv_sector_erase failed after %d attempts\r\n", FLASH_ERASE_RETRIES);
        PRINTF("[storage] Error code: %d (0x%08X)\r\n", (int)erase_result, (unsigned int)erase_result);
        PRINTF("[storage] ----------------------------------------\r\n");
        PRINTF("[storage] ========================================\r\n");

        if (erase_result == 7002)
        {
            PRINTF("[storage] ℹ Error 7002 = Sector protected/locked\r\n");
            PRINTF("[storage] ℹ ACTION: Try offset: 0x02800000 or 0x03000000\r\n");
        }

        return false;
    }

    /* Write pages with retries */
    PRINTF("[storage] ----------------------------------------\r\n");
    PRINTF("[storage] WRITE OPERATION\r\n");
    PRINTF("[storage] Writing %d pages (%d bytes each)...\r\n",
           (int)(STORAGE_FLASH_SIZE / MFLASH_PAGE_SIZE), MFLASH_PAGE_SIZE);

    uint32_t pages = STORAGE_FLASH_SIZE / MFLASH_PAGE_SIZE;
    for (uint32_t page = 0; page < pages; page++)
    {
        uint32_t page_offset = STORAGE_FLASH_OFFSET + (page * MFLASH_PAGE_SIZE);
        uint8_t *page_data = g_storage_buffer + (page * MFLASH_PAGE_SIZE);

        int32_t write_result = -1;
        for (int attempt = 0; attempt < FLASH_WRITE_RETRIES; attempt++)
        {
            write_result = mflash_drv_page_program(page_offset, (uint32_t *)page_data);

            if (write_result == 0)
            {
                break;
            }

            vTaskDelay(pdMS_TO_TICKS(50));
        }

        if (write_result != 0)
        {
            PRINTF("[storage] ✗ ERROR: mflash_drv_page_program failed at page %d\r\n", (int)page);
            PRINTF("[storage] Error code: %d (0x%08X)\r\n", (int)write_result, (unsigned int)write_result);
            return false;
        }

        PRINTF("[storage] ✓ Page %d written successfully\r\n", (int)page);
    }

    PRINTF("[storage] ----------------------------------------\r\n");
    PRINTF("[storage] ✓ Configuration written successfully to FLASH\r\n");

    /* Update cache */
    memcpy(&g_storage_data, &header, sizeof(storage_header_t));
    g_storage_valid = true;
    g_storage_initialized = true;

    PRINTF("[storage] ========================================\r\n");
    return true;
}

/**
 * @brief Erase WiFi configuration from FLASH
 */
void storage_erase_config(void)
{
    PRINTF("[storage] storage_erase_config() called\r\n");
    PRINTF("[storage] Erasing sector at offset 0x%08X...\r\n", (unsigned int)STORAGE_FLASH_OFFSET);

    int32_t result = mflash_drv_sector_erase(STORAGE_FLASH_OFFSET);

    if (result == 0)
    {
        PRINTF("[storage] ✓ Configuration erased\r\n");
        g_storage_valid = false;
        g_storage_initialized = false;
    }
    else
    {
        PRINTF("[storage] ✗ ERROR: Failed to erase configuration (result: %d)\r\n", (int)result);
    }
}

/**
 * @brief Initialize storage system
 */
void storage_init(void)
{
    PRINTF("[storage] Initializing storage system...\r\n");
    storage_print_offset_info();

    if (storage_verify_flash_access())
    {
        PRINTF("[storage] ✓ Storage system initialized successfully\r\n");
    }
    else
    {
        PRINTF("[storage] ✗ WARNING: Storage system initialization incomplete\r\n");
    }
}

/**
 * @brief Get storage base address (debugging)
 */
uint32_t storage_get_address(void)
{
    return STORAGE_FLASH_OFFSET;
}

/**
 * @brief Get storage sector size (debugging)
 */
uint32_t storage_get_size(void)
{
    return STORAGE_FLASH_SIZE;
}

/**
 * @brief Check if FLASH is accessible
 */
bool storage_is_flash_accessible(void)
{
    return g_storage_flash_accessible;
}

/**
 * @brief Get storage status info
 */
void storage_print_status(void)
{
    PRINTF("[storage] ========================================\r\n");
    PRINTF("[storage] STORAGE STATUS\r\n");
    PRINTF("[storage] Initialized: %s\r\n", g_storage_initialized ? "YES" : "NO");
    PRINTF("[storage] Valid config: %s\r\n", g_storage_valid ? "YES" : "NO");
    PRINTF("[storage] FLASH accessible: %s\r\n", g_storage_flash_accessible ? "YES" : "NO");
    PRINTF("[storage] Offset: 0x%08X\r\n", (unsigned int)STORAGE_FLASH_OFFSET);
    PRINTF("[storage] Size: %d bytes\r\n", STORAGE_FLASH_SIZE);
    PRINTF("[storage] ========================================\r\n");
}
