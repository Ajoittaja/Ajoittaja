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
#include "cts_service.h"
#include "dk_buttons_and_leds.h"
#include "my_lbs.h"
#include "time-now.h"
#include <bluetooth/gatt_dm.h>
#include <bluetooth/services/lbs.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>

#include <dk_buttons_and_leds.h>

LOG_MODULE_DECLARE(Ajoittaja, LOG_LEVEL_DBG);

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

static void notify_current_time_cb(struct bt_cts_client *cts_c,
                                   struct bt_cts_current_time *current_time) {
    current_time_print(current_time);
}

static void enable_notifications(void) {
    int err;

    if (has_cts && (bt_conn_get_security(cts_c.conn) >= BT_SECURITY_L2)) {
        err = bt_cts_subscribe_current_time(&cts_c, notify_current_time_cb);
        if (err) {
            printk("Cannot subscribe to current time value notification (err "
                   "%d)\n",
                   err);
        }
    }
}

static void security_changed(struct bt_conn *conn, bt_security_t level,
                             enum bt_security_err err) {
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    if (!err) {
        printk("Security changed: %s level %u\n", addr, level);

        enable_notifications();
    } else {
        printk("Security failed: %s level %u err %d\n", addr, level, err);
    }
}

/// Connection callbacks
// TODO: OK
struct bt_conn_cb connection_callbacks = {
    .connected = on_connected,
    .disconnected = on_disconnected,
    .le_param_updated = on_le_param_updated,
    .le_phy_updated = on_le_phy_updated,
    .le_data_len_updated = on_le_data_len_updated,
    .security_changed = security_changed,
};

/// MTU exchange parameters
// TODO: OK
struct bt_gatt_exchange_params exchange_params;

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

    has_cts = false;
    err = bt_gatt_dm_start(conn, BT_UUID_CTS, &discover_cb, NULL);
    if (err) {
        printk("Failed to start discovery (err %d)\n", err);
    }
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
        int err;
        err = bt_cts_read_current_time(&cts_c, read_current_time_cb);
        if (err) {
            LOG_WRN("Failed reading current time (err: %d)\n", err);
        }
        k_sleep(K_SECONDS(2));
    }
}

static void app_led_cb(bool led_state) { dk_set_led(USER_LED, led_state); }
static bool app_button_state;
static bool app_button_cb(void) { return app_button_state; }
static struct my_lbs_cb app_callbacks = {
    .led_cb = app_led_cb,
    .button_cb = app_button_cb,
};

static void button_changed(uint32_t button_state, uint32_t has_changed) {
    if (has_changed & USER_BUTTON) {
        uint32_t user_button_state = button_state & USER_BUTTON;
        /* STEP 6 - Send indication on a button press */
        my_lbs_send_button_state_indicate(user_button_state);
        app_button_state = user_button_state ? true : false;
        my_lbs_send_button_state_indicate(user_button_state);
    }
}

static int init_button(void) {
    int err;

    err = dk_buttons_init(button_changed);
    if (err) {
        printk("Cannot init buttons (err: %d)\n", err);
    }

    return err;
}

static void auth_cancel(struct bt_conn *conn) {
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    printk("Pairing cancelled: %s\n", addr);

    bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
}

static struct bt_conn_auth_cb conn_auth_callbacks = {
    .cancel = auth_cancel,
};

static void pairing_complete(struct bt_conn *conn, bool bonded) {
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    printk("Pairing completed: %s, bonded: %d\n", addr, bonded);
}

static void pairing_failed(struct bt_conn *conn, enum bt_security_err reason) {
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    printk("Pairing failed conn: %s, reason %d\n", addr, reason);

    bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
}

static struct bt_conn_auth_info_cb conn_auth_info_callbacks = {
    .pairing_complete = pairing_complete, .pairing_failed = pairing_failed};

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

    // Authorization callbacks
    err = bt_conn_auth_cb_register(&conn_auth_callbacks);
    if (err) {
        printk("Failed to register authorization callbacks\n");
        return 0;
    }
    err = bt_conn_auth_info_cb_register(&conn_auth_info_callbacks);
    if (err) {
        printk("Failed to register authorization info callbacks\n");
        return 0;
    }

    // Current Time Service client
    err = bt_cts_client_init(&cts_c);
    if (err) {
        LOG_ERR("CTS client init failed (err %d)\n", err);
        return 0;
    }

    // Enable
    err = bt_enable(NULL);
    if (err) {
        LOG_ERR("Bluetooth init failed (err %d)\n", err);
        return -1;
    }
    LOG_INF("Bluetooth initialized\n");

    // FIXME: Check
    init_button();
    err = my_lbs_init(&app_callbacks);
    if (err) {
        printk("Failed to init LBS (err:%d)\n", err);
        return -1;
    }

    // Start advertising
    err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if (err) {
        LOG_ERR("Advertising failed to start (err %d)\n", err);
        return -1;
    }

    return err;
}