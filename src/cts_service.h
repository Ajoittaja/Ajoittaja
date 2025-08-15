/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef CTS_SERVICE_H_
#define CTS_SERVICE_H_

#include <bluetooth/services/cts_client.h>

extern struct bt_cts_client cts_c;

static bool has_cts;

void discover_completed_cb(struct bt_gatt_dm *dm, void *ctx);

void discover_service_not_found_cb(struct bt_conn *conn, void *ctx);

void discover_error_found_cb(struct bt_conn *conn, int err, void *ctx);

void current_time_print(struct bt_cts_current_time *current_time);

void read_current_time_cb(struct bt_cts_client *cts_c,
                          struct bt_cts_current_time *current_time, int err);

static const struct bt_gatt_dm_cb discover_cb = {
    .completed = discover_completed_cb,
    .service_not_found = discover_service_not_found_cb,
    .error_found = discover_error_found_cb,
};

#endif   // CTS_SERVICE_H_