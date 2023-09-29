/**
 * @file bluetooth.h
 * @brief Bluetooth handler declarations
 * @author Kasperi Kiviluoma
 * @license MIT
 *
 * Handles communication between Ajoittaja and Ajoittaja Android
 * application via bluetooth.
 */

#include <bluetooth/gatt_dm.h>

#define DEVICE_NAME     CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

/// Holds advertising data
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

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