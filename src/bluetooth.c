/**
 * @file bluetooth.c
 * @brief Bluetooth handler functions
 * @author Kasperi Kiviluoma
 * @license MIT
 *
 * Handles communication between Ajoittaja and Ajoittaja Android
 * application with bluetooth.
 */

#include "bluetooth.h"
#include "time-now.h"
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(Ajoittaja, LOG_LEVEL_DBG);

static uint8_t battery_level[4] = {
    0};   // Assuming 'battery_level' is a byte array with 4 values

static ssize_t read_blvl(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                         void *buf, uint16_t len, uint16_t offset) {
    Time nnow = now;
    const uint8_t *value = (const uint8_t *) &nnow + offset;

    return bt_gatt_attr_read(conn, attr, buf, len, offset, &now,
                             sizeof(now) - offset);
}

static ssize_t write_test(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                          void *buf, uint16_t len, uint16_t offset) {
    LOG_INF("test");
    memcpy(&battery_level + offset, buf, len);
    LOG_INF("test%d %d %d {}", battery_level[0], battery_level[1],
            battery_level[2]);
    Time t = {battery_level[0], battery_level[1], battery_level[2],
              battery_level[3]};
    now = t;   // FIXME: Potentially dangerous
}

static void blvl_ccc_cfg_changed(const struct bt_gatt_attr *attr,
                                 uint16_t value) {
    ARG_UNUSED(attr);

    bool notif_enabled = (value == BT_GATT_CCC_NOTIFY);

    LOG_INF("BAS Notifications %s", notif_enabled ? "enabled" : "disabled");
}
BT_GATT_SERVICE_DEFINE(basa, BT_GATT_PRIMARY_SERVICE(BT_UUID_GATT),
                       BT_GATT_CHARACTERISTIC(BT_UUID_GATT_VAL,
                                              BT_GATT_CHRC_READ |
                                                  BT_GATT_CHRC_NOTIFY |
                                                  BT_GATT_CHRC_WRITE,
                                              BT_GATT_PERM_READ |
                                                  BT_GATT_PERM_WRITE,
                                              read_blvl, write_test, &now),
                       BT_GATT_CCC(blvl_ccc_cfg_changed,
                                   BT_GATT_PERM_READ | BT_GATT_PERM_WRITE), );

static void batt_notify(void) {
    uint8_t level = 100U;
    bt_gatt_notify(NULL, &basa.attrs[1], &level, sizeof(level));
}

static void mtu_updated(struct bt_conn *conn, uint16_t tx, uint16_t rx) {
    printk("Updated MTU: TX: %d RX: %d bytes\n", tx, rx);
}

static struct bt_gatt_cb gatt_callbacks = {.att_mtu_updated = mtu_updated};

void ble_handler() {
    bool stop = false;

    LOG_INF("Starting GATT Discovery Manager example");
    if (bluetooth_init() != 0) {
        stop = true;
    } else {
        LOG_INF("Advertising successfully started");
    }

    // TODO: Read example
    bt_gatt_cb_register(&gatt_callbacks);

    while (!stop) {
        k_sleep(K_SECONDS(1));

        // TODO: Battery level simulation
        batt_notify();
    }
}

int bluetooth_init() {
    int err;

    // Enable
    err = bt_enable(NULL);
    if (err) {
        LOG_ERR("BLE init failed with error code %d\n", err);
        return err;
    } else {
        printk("Bluetooth initialized\n");
    }

    // Start advertising
    err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err) {
        LOG_ERR("Advertising failed to start (err %d)\n", err);
        return err;
    } else {
        printk("Advertising successfully started\n");
    }

    return err;
}