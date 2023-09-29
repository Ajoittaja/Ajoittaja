/**
 * @file bluetooth.c
 * @brief Bluetooth handler functions.
 * @author Kasperi Kiviluoma
 * @license MIT
 *
 * Handles communication between Ajoittaja and Ajoittaja Android
 * application with bluetooth.
 */

#include "bluetooth.h"
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(Ajoittaja, LOG_LEVEL_DBG);

int bluetooth_init(void) {
    int err;

    // Enable
    err = bt_enable(NULL);
    if (err) {
        LOG_ERR("BLE init failed with error code %d\n", err);
        return err;
    }

    // Start advertising
    err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err) {
        LOG_ERR("Advertising failed to start (err %d)\n", err);
        return err;
    }

    return err;
}