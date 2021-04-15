/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 * In order to evaluate the performance of BLEX, it is a throughput measurement code that was modified and brought from Nordic SDK. 
 * In addition, it was modified so that several slaves can connect to one master.
 * In particular, this code accepts up to two slaves using the same connection interval from the master's point of view, 
 *      and changes the traffic sent to each slave according to time.
 */

#include <kernel.h>
#include <console/console.h>
#include <sys/printk.h>
#include <string.h>
#include <stdlib.h>
#include <zephyr/types.h>

#include <stddef.h>
#include <errno.h>
#include <zephyr.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/crypto.h>
#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>
#include <bluetooth/hci.h>
#include <bluetooth/uuid.h>
#include "gatt_dm.h"
#include "throughput.h"

#define DEVICE_NAME	CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

static volatile bool test_ready = true;
static volatile bool nodes_ready = false;
static struct bt_conn *default_conn[CONFIG_BT_MAX_CONN];
static bool scan_start = false;

static struct bt_gatt_throughput gatt_throughput[CONFIG_BT_MAX_CONN];
static struct bt_gatt_exchange_params exchange_params;
int cnt = 0;
int M = 10;
int old_cnt = 0;
int iter_cnt = 2;
int iter = 0;	
bool start_flag = false;
#define sys_le16_to_cpu(val)(val)
static const char img[][81] = {
#include "img.file"
};

static void exchange_func(struct bt_conn *conn, uint8_t att_err,
		struct bt_gatt_exchange_params *params)
{
	struct bt_conn_info info = {0};
	int err;


	err = bt_conn_get_info(conn, &info);
	if (err){
		
		return;
	}

	if (info.role == BT_CONN_ROLE_MASTER){ 
		k_sleep(K_SECONDS(15));
		if(iter == iter_cnt)
			nodes_ready = true;;
		iter++;
		scan_start= false;
		int i;
	}
}

static void discovery_complete(struct bt_gatt_dm *dm,
		void *context)
{
	int err;
	struct bt_gatt_throughput *throughput = context;

	bt_gatt_dm_data_print(dm);
	bt_gatt_throughput_handles_assign(dm, throughput);
	bt_gatt_dm_data_release(dm);


	exchange_params.func = exchange_func;
	err = bt_gatt_exchange_mtu(bt_gatt_dm_conn_get(dm), &exchange_params);
	if (err) {
	} else {
	}
}

static void discovery_service_not_found(struct bt_conn *conn,
		void *context)
{
}

static void discovery_error(struct bt_conn *conn,
		int err,
		void *context)
{
}

struct bt_gatt_dm_cb discovery_cb = {
	.completed         = discovery_complete,
	.service_not_found = discovery_service_not_found,
	.error_found       = discovery_error,
};

static uint8_t throughput_read(struct bt_conn *conn, const struct bt_gatt_throughput_metrics *met)
{
	int i;
	for(i = 0; i <CONFIG_BT_MAX_CONN; i ++)
		if(default_conn[i] == conn){
			printk("\n[peer]round %u: received %u bytes (%u KB)"
				" in %u GATT writes at %u bps from slave %u\n",cnt,
				met->write_len, met->write_len / 1024, met->write_count,
				met->write_rate, i);
			i = CONFIG_BT_MAX_CONN;
		}
		test_ready = true;

	return BT_GATT_ITER_STOP;
}
static void throughput_received(const struct bt_gatt_throughput_metrics *met)
{
	static uint32_t kb;

	if (met->write_len == 0) {
		kb = 0;

		return;
	}

	if ((met->write_len / 1024) != kb) {
		kb = (met->write_len / 1024);
	}
}
static void throughput_send(const struct bt_gatt_throughput_metrics *met)
{
}
static const struct bt_gatt_throughput_cb throughput_cb = {
	.data_read = throughput_read,
	.data_received = throughput_received,
	.data_send = throughput_send
};
static void connected(struct bt_conn *conn, uint8_t hci_err)
{
	struct bt_conn_info info = {0};
	int err;
	err = bt_conn_get_info(conn, &info);
	if (err) {
		return;
	}


	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (hci_err) {
		for(int i = 0; i < CONFIG_BT_MAX_CONN; i ++)
			if(default_conn[i] == conn){
				bt_conn_unref(default_conn[i]);
				default_conn[i] = NULL;
				start_scan();
			}

		return;
	}
	printk("[peer] Conn. interval is %u units\n", info.le.interval);
	int i;
	for( i = 0; i <CONFIG_BT_MAX_CONN; i ++)
		if(default_conn[i] == conn) {
			err = bt_gatt_dm_start(default_conn[i],
					BT_UUID_THROUGHPUT,
					&discovery_cb,
					&gatt_throughput[i]);
			if (err) {
			}
			i = CONFIG_BT_MAX_CONN;
		}
}
static bool eir_found(struct bt_data *data, void *user_data)
{
	bt_addr_le_t *addr = user_data;
	int i;


	switch (data->type) {
		case BT_DATA_UUID16_SOME:
		case BT_DATA_UUID16_ALL:
			if (data->data_len % sizeof(uint16_t) != 0U) {
				return true;
			}

			for (i = 0; i < data->data_len; i += sizeof(uint16_t)) {
				struct bt_le_conn_param *param, *param2;
				struct bt_uuid *uuid;
				uint16_t u16;
				int err;

				memcpy(&u16, &data->data[i], sizeof(u16));
				uuid = BT_UUID_DECLARE_16(sys_le16_to_cpu(u16));
				if (bt_uuid_cmp(uuid, BT_UUID_HRS)) {
					continue;
				}

				param = BT_LE_CONN_PARAM_DEFAULT;
				param2 = BT_LE_CONN_PARAM_DEFAULT2;
				int number= 0;
				for(int i = 0; i <CONFIG_BT_MAX_CONN && number<iter+1 && iter<iter_cnt; i ++){
					if(default_conn[i] == NULL){
						err = bt_le_scan_stop();
						if (err) {
							continue;
						}
						scan_start = false;
						err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN,
								param, &default_conn[i]);
						if (err) {
						}
						i = CONFIG_BT_MAX_CONN;
					}else{
						 number ++;
					}
				}

				return false;
			}
	}

	return true;
}

static void device_found(const bt_addr_le_t *addr, uint8_t rssi, uint8_t type,
		struct net_buf_simple *ad)
{
	char dev[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(addr, dev, sizeof(dev));

	/* We're only interested in connectable events */
	if (type == BT_GAP_ADV_TYPE_ADV_IND ||
			type == BT_GAP_ADV_TYPE_ADV_DIRECT_IND) {
		bt_data_parse(ad, eir_found, (void *)addr);

	}
}


void start_scan(void)
{

	int err;
	nodes_ready = false;
	scan_start= true;
	/* Use active scanning and disable duplicate filtering to handle any
	 * devices that might update their advertising data at runtime. */
	struct bt_le_scan_param scan_param = {
		.type       = BT_LE_SCAN_TYPE_ACTIVE,
		.options    = BT_LE_SCAN_OPT_NONE,
		.interval   = BT_GAP_SCAN_FAST_INTERVAL,
		.window     = BT_GAP_SCAN_FAST_WINDOW,
	};

	err = bt_le_scan_start(&scan_param, device_found);
	if (err) {
		return;
	}

	printk("[peer]Scanning successfully started\n");
}
static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	struct bt_conn_info info = {0};
	int err;

	for(int i = 0; i<CONFIG_BT_MAX_CONN; i ++)
		if(conn == default_conn[i]){
			printk("[peer]disconnected\n");
			bt_conn_unref(conn);
			default_conn[i] = NULL;
			cnt = 0;
			iter -= 1;
			i = CONFIG_BT_MAX_CONN;
		}
	err = bt_conn_get_info(conn, &info);
	if (err) {
		return;
	}

	/* Re-connect using same roles */
	if (info.role == BT_CONN_ROLE_MASTER) {
		start_scan();
	}
}

static void test_run(void)
{
	int err;
	uint64_t stamp;
	uint32_t delta;
	uint32_t data=0;
	uint32_t prog=0;

	/* a dummy data buffer */
	static char dummy[256];//ejpark0401

	/* wait for user input to continue */

	int number;
	number = 0;
	for(int i = 0; i<CONFIG_BT_MAX_CONN; i ++ )
		if( default_conn[i] !=NULL){
			number ++;
		}
	if(iter<iter_cnt|| !test_ready ){
		if(scan_start == false) start_scan();
		return;
	}


	/* reset peer metrics */
	number = 0;
	for(int i =0; i <CONFIG_BT_MAX_CONN && number < iter_cnt;i++ ){
		if(default_conn[i] != NULL){
			struct bt_conn *temp_conn = gatt_throughput[i].conn;
			err = bt_gatt_throughput_write(&gatt_throughput[i], dummy, 1);
			number ++;
		}
	}
	if (err) {
		return;
	}
	int sec = 15000/iter;
	int temp_iter = iter;
	stamp= k_uptime_get_32();
	int break_time = 2;
	uint64_t stamp2;
	while(k_uptime_get_32()-stamp < 20000){
		number = 0;
	stamp2 = k_uptime_get_32();		
		for(int i =0; i <CONFIG_BT_MAX_CONN && number < temp_iter;i++ ){
			if(default_conn[i] != NULL){
				/* BLEX: By changing the data length, we can change the amount of traffic of each slaves*/
				if((i+cnt)%2 == 0)
					err = bt_gatt_throughput_write(&gatt_throughput[i], dummy, 240);
				else 
					err = bt_gatt_throughput_write(&gatt_throughput[i], dummy, 80);
				number ++;
			}
		}
		while(k_uptime_get_32()-stamp2 <break_time);
		prog ++;
		data += 340;
		if (err) {
			break;
		}
	}

	delta = k_uptime_get_32()-stamp;
	printk("\n[ejpark2]Done\n");
		printk("[ejpark2][local] round %d: sent %u bytes (%u KB) in %u ms at %llu kbps\n",cnt,
				data, data / 1024, delta, ((uint64_t)data* 8 / delta));
	/* read back char from peer */
	number = 0;
	for(int i =0; i <CONFIG_BT_MAX_CONN && number < iter;i++ ){
		if(default_conn[i] != NULL){
			err = bt_gatt_throughput_read(&gatt_throughput[i]);
			test_ready = false;
			number ++;
		}
	}
	if (err) {
	}
	start_flag = true;
	cnt ++;
}

static bool le_param_req(struct bt_conn *conn, struct bt_le_conn_param *param)
{
	/* reject peer conn param request */
	return false;
}

void main(void)
{
	int err;

	static struct bt_conn_cb conn_callbacks = {
		.connected = connected,
		.disconnected = disconnected,
		.le_param_req = le_param_req,
	};

	bt_conn_cb_register(&conn_callbacks);

	err = bt_enable(NULL);
	if (err) {
		return;
	}
	for(int i = 0; i<CONFIG_BT_MAX_CONN; i ++){
		default_conn[i] = NULL;
	}

	for(int i = 0; i <CONFIG_BT_MAX_CONN; i ++){
		err = bt_gatt_throughput_init(&gatt_throughput[i], &throughput_cb);
	}
	
	start_scan();
	
	if (err) {
		return;
	}
	test_ready = true;

	for ( ;;) {
		test_run();
	}

}
