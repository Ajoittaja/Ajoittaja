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
#include "dk_buttons_and_leds.h"
#include "time-now.h"
#include <bluetooth/services/lbs.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <dk_buttons_and_leds.h>

LOG_MODULE_DECLARE(Ajoittaja, LOG_LEVEL_DBG);
#define CONNECTION_STATUS_LED DK_LED2
#define USER_BUTTON           DK_BTN1_MSK

/**
 * @brief Callback for connection
 *
 * Set optimal settings for phy, data length and mtu.
 *
 * @param conn Connection
 * @param err Error code
 */
// TODO: OK
void on_connected(struct bt_conn *conn, uint8_t err);
/**
 * @brief Callback for disconnection
 *
 * @param conn Connection
 * @param reason Reason for disconnection
 */
// TODO: OK
void on_disconnected(struct bt_conn *conn, uint8_t reason);
/**
 * @brief Callback for connection parameter update
 *
 * Requested by the central device
 *
 * @param conn Connection
 * @param interval Connection interval
 * @param latency Connection latency
 */
// TODO: OK
void on_le_param_updated(struct bt_conn *conn, uint16_t interval,
                         uint16_t latency, uint16_t timeout);
/**
 * @brief Callback for PHY update (connection speed)
 *
 * @param conn Connection
 * @param param PHY parameters
 */
// TODO: OK
void on_le_phy_updated(struct bt_conn *conn, struct bt_conn_le_phy_info *param);
/**
 * @brief Callback for data length update
 *
 * @param conn Connection
 * @param info Data length parameters
 */
// TODO: OK
void on_le_data_len_updated(struct bt_conn *conn,
                            struct bt_conn_le_data_len_info *info);

/// Bt connection handle
// TODO: OK
struct bt_conn *bt_handle;

/// Connection callbacks
// TODO: OK
struct bt_conn_cb connection_callbacks = {
    .connected = on_connected,
    .disconnected = on_disconnected,
    .le_param_updated = on_le_param_updated,
    .le_phy_updated = on_le_phy_updated,
    .le_data_len_updated = on_le_data_len_updated};

/// MTU exchange parameters
// TODO: OK
struct bt_gatt_exchange_params exchange_params;

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

// TODO: OK
void on_connected(struct bt_conn *conn, uint8_t err) {
    if (err) {
        LOG_ERR("Connection error %d.", err);
        return;
    }

    LOG_INF("Connected");
    bt_handle = bt_conn_ref(conn);

    // Log connection parameters
    struct bt_conn_info info;
    err = bt_conn_get_info(conn, &info);
    if (err) {
        LOG_ERR("bt_conn_get_info() returned %d.", err);
        return;
    }
    double connection_interval = info.le.interval * 1.25;   // in ms
    uint16_t supervision_timeout = info.le.timeout * 10;    // in ms
    LOG_INF("Initial connection parameters: interval %.2f ms, latency %d "
            "intervals, "
            "timeout %d ms.",
            connection_interval, info.le.latency, supervision_timeout);

    update_phy(bt_handle);
    update_data_length(bt_handle);
    update_mtu(bt_handle);
}

// TODO: OK
void on_disconnected(struct bt_conn *conn, uint8_t reason) {
    LOG_INF("Disconnected. Reason %d.", reason);
    bt_conn_unref(bt_handle);
}

// TODO: OK
void on_le_param_updated(struct bt_conn *conn, uint16_t interval,
                         uint16_t latency, uint16_t timeout) {

    double connection_interval = interval * 1.25;   // in ms
    uint16_t supervision_timeout = timeout * 10;    // in ms
    LOG_INF("Connection parameters updated: interval %.2f ms, latency %d "
            "intervals, timeout %d ms",
            connection_interval, latency, supervision_timeout);
    update_phy(bt_handle);   // Force 2M PHY
}

// TODO: OK
void on_le_phy_updated(struct bt_conn *conn,
                       struct bt_conn_le_phy_info *param) {

    if (param->tx_phy == BT_CONN_LE_TX_POWER_PHY_1M) {
        LOG_INF("PHY updated. New PHY: 1M");
    } else if (param->tx_phy == BT_CONN_LE_TX_POWER_PHY_2M) {
        LOG_INF("PHY updated. New PHY: 2M");
    } else if (param->tx_phy == BT_CONN_LE_TX_POWER_PHY_CODED_S8) {
        LOG_INF("PHY updated. New PHY: Long Range");
    }
}

// TODO: OK
void on_le_data_len_updated(struct bt_conn *conn,
                            struct bt_conn_le_data_len_info *info) {
    uint16_t tx_len = info->tx_max_len;
    uint16_t tx_time = info->tx_max_time;
    uint16_t rx_len = info->rx_max_len;
    uint16_t rx_time = info->rx_max_time;
    LOG_INF("Data length updated. Length %d/%d bytes, time %d/%d us.", tx_len,
            rx_len, tx_time, rx_time);
}

/**
 * @brief Update PHY
 *
 * @param conn Connection
 */
// TODO: OK
void update_phy(struct bt_conn *conn) {
    int err;
    const struct bt_conn_le_phy_param preferred_phy = {
        .options = BT_CONN_LE_PHY_OPT_NONE,
        .pref_rx_phy = BT_GAP_LE_PHY_2M,
        .pref_tx_phy = BT_GAP_LE_PHY_2M,
    };
    err = bt_conn_le_phy_update(conn, &preferred_phy);
    if (err) {
        LOG_ERR("bt_conn_le_phy_update() returned %d.", err);
    }
}

/**
 * @brief Update data length
 *
 * @param conn Connection
 */
// TODO: OK
void update_data_length(struct bt_conn *conn) {
    int err;
    struct bt_conn_le_data_len_param my_data_len = {
        .tx_max_len = BT_GAP_DATA_LEN_MAX,
        .tx_max_time = BT_GAP_DATA_TIME_MAX,
    };
    err = bt_conn_le_data_len_update(bt_handle, &my_data_len);
    if (err) {
        LOG_ERR("data_len_update failed (err %d).", err);
    }
}

/**
 * @brief MTU exchange function
 *
 * @param conn Connection
 * @param att_err ATT error
 * @param params Exchange parameters
 */
// TODO: OK
void exchange_func(struct bt_conn *conn, uint8_t att_err,
                   struct bt_gatt_exchange_params *params) {
    LOG_INF("MTU exchange %s", att_err == 0 ? "successful." : "failed.");
    if (!att_err) {
        uint16_t payload_mtu =
            bt_gatt_get_mtu(conn) - 3;   // 3 bytes used for Attribute headers.
        LOG_INF("New MTU: %d bytes", payload_mtu);
    }
}

/**
 * @brief Update MTU
 *
 * @param conn Connection
 */
// TODO: OK
void update_mtu(struct bt_conn *conn) {
    int err;
    exchange_params.func = exchange_func;

    err = bt_gatt_exchange_mtu(conn, &exchange_params);
    if (err) {
        LOG_ERR("bt_gatt_exchange_mtu failed (err %d).", err);
    }
}

// TODO: OK
void ble_handler() {
    bool stop = false;

    if (bluetooth_init() != 0) {
        stop = true;
    } else {
        LOG_INF("Advertising successfully started");
    }

    while (!stop) {
        // TODO: Change to a more power efficient way to sleep
        k_sleep(K_SECONDS(1));
    }
}

/**
 * @brief Start BLE advertising
 *
 * Initialize bluetooth and start advertising.
 *
 * @return Zero on success or (negative) error code otherwise
 */
// TODO: OK
int bluetooth_init() {
    int err;

    // Connection callbacks
    bt_conn_cb_register(&connection_callbacks);

    // Enable
    err = bt_enable(NULL);
    if (err) {
        LOG_ERR("Bluetooth init failed (err %d)\n", err);
        return -1;
    }
    LOG_INF("Bluetooth initialized\n");

    // Start advertising
    err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if (err) {
        LOG_ERR("Advertising failed to start (err %d)\n", err);
        return -1;
    }

    return err;
}