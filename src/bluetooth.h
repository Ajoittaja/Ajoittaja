/**
 * @file bluetooth.h
 * @brief Bluetooth handler declarations
 * @author Kasperi Kiviluoma
 * @license MIT
 *
 * Handles communication between Ajoittaja and Ajoittaja Android
 * application via bluetooth.
 */

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/uuid.h>

#define DEVICE_NAME     CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)
// FIXME: Change to actual company code, needs to be registered
#define COMPANY_ID_CODE 0x0059

/// Manufacturer data structure
typedef struct adv_mfg_data {
    uint16_t company_code;   // Company Identifier Code
} adv_mfg_data_type;

/// Manufacturer data
static adv_mfg_data_type adv_mfg_data = {COMPANY_ID_CODE};

/// Holds advertising data
// TODO: Consider BT_LE_AD_LIMITED
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
    BT_DATA(BT_DATA_MANUFACTURER_DATA, (unsigned char *) &adv_mfg_data,
            sizeof(adv_mfg_data))};

/// Holds scan response data
// TODO: Change UUID to match a correct service
static const struct bt_data sd[] = {BT_DATA_BYTES(
    BT_DATA_UUID128_ALL,
    BT_UUID_128_ENCODE(0x00001523, 0x1212, 0xefde, 0x1523, 0x785feabcd123))};

/// Advertising parameters
/// TODO: Consider disabling BT_LE_ADV_OPT_USE_IDENTITY
/// TODO: Consider changing advertising interval in order to save power
static struct bt_le_adv_param *adv_param =
    BT_LE_ADV_PARAM((BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_USE_IDENTITY),
                    800,     // Min Advertising Interval 500ms (800*0.625ms)
                    801,     // Max Advertising Interval 500.625ms (801*0.625ms)
                    NULL);   // Set to NULL for undirected advertising

/// BLE thread handler
void ble_handler();

/**
 * @brief Start BLE advertising
 *
 * Initialize bluetooth and start advertising.
 *
 * @return Zero on success or (negative) error code otherwise
 */
int bluetooth_init(void);