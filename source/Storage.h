/**
 * @file storage.h
 *
 * @brief Non-volatile storage header for WiFi credentials
 *
 * Copyright 2025 NXP
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __STORAGE_H__
#define __STORAGE_H__

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define WIFI_SSID_MAX_LEN    32
#define WIFI_PASS_MAX_LEN    63

/*******************************************************************************
 * Types
 ******************************************************************************/

/**
 * @brief WiFi configuration structure
 */
typedef struct {
    char ssid[WIFI_SSID_MAX_LEN + 1];       /* WiFi SSID */
    char password[WIFI_PASS_MAX_LEN + 1];   /* WiFi password */
} wifi_config_t;

/*******************************************************************************
 * API Functions
 ******************************************************************************/

/**
 * @brief Check if valid WiFi configuration exists in FLASH
 *
 * Validates:
 * - Magic number
 * - CRC32
 * - SSID not empty
 *
 * @return true if valid config found, false otherwise
 */
bool storage_has_valid_config(void);

/**
 * @brief Read WiFi configuration from FLASH
 *
 * @param config Pointer to wifi_config_t to store data
 * @return true if successful, false otherwise
 *
 * @note Call storage_has_valid_config() first to verify config exists
 */
bool storage_read_wifi_config(wifi_config_t *config);

/**
 * @brief Write WiFi configuration to FLASH
 *
 * Automatically:
 * - Erases sector
 * - Validates input
 * - Calculates CRC32
 * - Writes with magic number
 *
 * @param config Pointer to wifi_config_t to write
 * @return true if successful, false otherwise
 *
 * @note This function modifies FLASH - use carefully
 */
bool storage_write_wifi_config(const wifi_config_t *config);

/**
 * @brief Erase WiFi configuration from FLASH
 *
 * Clears all stored credentials. Device will enter provisioning mode on next boot.
 *
 * @note This is destructive - cannot be undone without new provisioning
 */
void storage_erase_config(void);

/**
 * @brief Get storage base address (debugging)
 *
 * @return FLASH address where config is stored
 */
uint32_t storage_get_address(void);

/**
 * @brief Get storage sector size (debugging)
 *
 * @return Size of FLASH sector
 */
uint32_t storage_get_size(void);

#endif /* __STORAGE_H__ */
