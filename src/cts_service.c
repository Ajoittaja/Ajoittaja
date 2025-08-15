#include "cts_service.h"

struct bt_cts_client cts_c;   // Definition, memory allocated here

static const char *day_of_week[] = {"Unknown",   "Monday",   "Tuesday",
                                    "Wednesday", "Thursday", "Friday",
                                    "Saturday",  "Sunday"};

static const char *month_of_year[] = {
    "Unknown", "January", "February",  "March",   "April",    "May",     "June",
    "July",    "August",  "September", "October", "November", "December"};

void current_time_print(struct bt_cts_current_time *current_time) {
    printk("\nCurrent Time:\n");
    printk("\nDate:\n");

    printk("\tDay of week   %s\n",
           day_of_week[current_time->exact_time_256.day_of_week]);

    if (current_time->exact_time_256.day == 0) {
        printk("\tDay of month  Unknown\n");
    } else {
        printk("\tDay of month  %u\n", current_time->exact_time_256.day);
    }

    printk("\tMonth of year %s\n",
           month_of_year[current_time->exact_time_256.month]);
    if (current_time->exact_time_256.year == 0) {
        printk("\tYear          Unknown\n");
    } else {
        printk("\tYear          %u\n", current_time->exact_time_256.year);
    }
    printk("\nTime:\n");
    printk("\tHours     %u\n", current_time->exact_time_256.hours);
    printk("\tMinutes   %u\n", current_time->exact_time_256.minutes);
    printk("\tSeconds   %u\n", current_time->exact_time_256.seconds);
    printk("\tFractions %u/256 of a second\n",
           current_time->exact_time_256.fractions256);

    printk("\nAdjust reason:\n");
    printk("\tDaylight savings %x\n",
           current_time->adjust_reason.change_of_daylight_savings_time);
    printk("\tTime zone        %x\n",
           current_time->adjust_reason.change_of_time_zone);
    printk("\tExternal update  %x\n",
           current_time->adjust_reason.external_reference_time_update);
    printk("\tManual update    %x\n",
           current_time->adjust_reason.manual_time_update);
}

void read_current_time_cb(struct bt_cts_client *cts_c,
                          struct bt_cts_current_time *current_time, int err) {
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(cts_c->conn), addr, sizeof(addr));

    if (err) {
        printk("Cannot read Current Time: %s, error: %d\n", addr, err);
        return;
    }

    current_time_print(current_time);
}

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

void discover_service_not_found_cb(struct bt_conn *conn, void *ctx) {
    printk("The service could not be found during the discovery\n");
}

void discover_error_found_cb(struct bt_conn *conn, int err, void *ctx) {
    printk("The discovery procedure failed, err %d\n", err);
}

void discover_completed_cb(struct bt_gatt_dm *dm, void *ctx) {
    int err;

    printk("The discovery procedure succeeded\n");

    bt_gatt_dm_data_print(dm);

    err = bt_cts_handles_assign(dm, &cts_c);
    if (err) {
        printk("Could not assign CTS client handles, error: %d\n", err);
    } else {
        has_cts = true;

        if (bt_conn_get_security(cts_c.conn) < BT_SECURITY_L2) {
            err = bt_conn_set_security(cts_c.conn, BT_SECURITY_L2);
            if (err) {
                printk("Failed to set security (err %d)\n", err);
            }
        } else {
            enable_notifications();
        }
    }

    err = bt_gatt_dm_data_release(dm);
    if (err) {
        printk("Could not release the discovery data, error "
               "code: %d\n",
               err);
    }
}
