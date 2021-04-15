/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
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
/* size of stack area used by each thread */
#define STACKSIZE 1024

/* scheduling priority used by each thread */
#define PRIORITY 7


static bool test_ready[CONFIG_BT_MAX_CONN];//true;
static volatile bool nodes_ready = false;
static struct bt_conn *default_conn[CONFIG_BT_MAX_CONN];
static bool scan_start = false;
static bool flag = false;

static struct bt_gatt_throughput gatt_throughput[CONFIG_BT_MAX_CONN];
static struct bt_gatt_exchange_params exchange_params;
int cnt = 0;
int M = 10;
int old_cnt = 0;
int iter_cnt = 2;
int iter = 0;	
bool start_flag = false;
//bool test_ready[CONFIG_BT_MAX_CONN];
#define sys_le16_to_cpu(val)(val)
static const char img[][81] = {
#include "img.file"
};

static void exchange_func(struct bt_conn *conn, uint8_t att_err,
		struct bt_gatt_exchange_params *params)
{
	struct bt_conn_info info = {0};
	int err;

	//printk("1: MTU exchange %s\n", att_err == 0 ? "successful" : "failed");

	err = bt_conn_get_info(conn, &info);
	if (err){
		//printk("[ejpark2]Failed to get connection info %d\n", err);
		
		return;
	}

	if (info.role == BT_CONN_ROLE_MASTER){// && default_conn != NULL){// && default_conn2 != NULL) {
		printk("[ejpark2]exchange_fund\n");
		//if(!start_flag) 
		for(int i = 0; i < CONFIG_BT_MAX_CONN; i ++){
			if( default_conn[i] ==conn){
				printk("i %u\n", i);
				test_ready[i] = true;
			}
		}
	}
}

static void discovery_complete(struct bt_gatt_dm *dm,
		void *context)
{
	int err;
	struct bt_gatt_throughput *throughput = context;
	//printk("[ejpark2]Service discovery completed\n");

	bt_gatt_dm_data_print(dm);
	bt_gatt_throughput_handles_assign(dm, throughput);
	bt_gatt_dm_data_release(dm);


	exchange_params.func = exchange_func;
	err = bt_gatt_exchange_mtu(bt_gatt_dm_conn_get(dm), &exchange_params);//ejpark0407
	if (err) {
		//printk("[ejpark2]MTU exchange failed (err %d)\n", err);
	} else {
		//printk("[ejpark2]MTU exchange pending\n");
	}
}

static void discovery_service_not_found(struct bt_conn *conn,
		void *context)
{
	//printk("[ejpark2]Service not found\n");
}

static void discovery_error(struct bt_conn *conn,
		int err,
		void *context)
{
	//printk("[ejpark2]Error while discovering GATT database: (%d)\n", err);
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
			test_ready[i] = true;
			i = CONFIG_BT_MAX_CONN;
		}
	//if(iter == CONFIG_BT_MAX_CONN || start_flag)

	return BT_GATT_ITER_STOP;
}
static void throughput_received(const struct bt_gatt_throughput_metrics *met)
{
	static uint32_t kb;

	if (met->write_len == 0) {
		kb = 0;
		//printk("\n");

		return;
	}

	if ((met->write_len / 1024) != kb) {
		kb = (met->write_len / 1024);
		//printk("=");
	}
}
static void throughput_send(const struct bt_gatt_throughput_metrics *met)
{
//printk("\n[local] received %u bytes (%u KB)"
//			" in %u GATT writes at %u bps\n",
//			met->write_len, met->write_len / 1024,
//			met->write_count, met->write_rate);
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
	err = bt_conn_get_info(conn, &info);//ejpark0407
	if (err) {
		//printk("Failed to get connection info %d\n", err);
		return;
	}


	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (hci_err) {
		//printk("[ejpark2]Failed to connect to %s (%u)\n", addr, hci_err);

		//ejpark0407 start
		for(int i = 0; i < CONFIG_BT_MAX_CONN; i ++)
			if(default_conn[i] == conn){
				bt_conn_unref(default_conn[i]);
				default_conn[i] = NULL;
				//k_sleep(K_MSEC(20));start_scan();
				start_scan();
			}
		
return;
	}
//	printk("[ejpark2] Connected as %s\n",
//			info.role == BT_CONN_ROLE_MASTER ? "master" : "slave");
	printk("[peer] Conn. interval is %u units\n", info.le.interval);
	//test_ready = false;
	int i;
	for( i = 0; i <CONFIG_BT_MAX_CONN; i ++)
		if(default_conn[i] == conn) {
			err = bt_gatt_dm_start(default_conn[i],
					BT_UUID_THROUGHPUT,
					&discovery_cb,
					&gatt_throughput[i]);
			if (err) {
				//printk("[ejpark2]Discover failed (err %d)\n", err);
			}
			i = CONFIG_BT_MAX_CONN;
		}


	//printk("[ejpark2] test_ready %u\n", test_ready);
}
static bool eir_found(struct bt_data *data, void *user_data)
{
	bt_addr_le_t *addr = user_data;
	int i;

	//printk("[AD]: %u data_len %u\n", data->type, data->data_len);

	switch (data->type) {
		case BT_DATA_UUID16_SOME:
		case BT_DATA_UUID16_ALL:
			if (data->data_len % sizeof(uint16_t) != 0U) {
				//printk("AD malformed\n");
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
					//printk("WRONG UUID\n");
					continue;
				}

				////printk("[ejpark2] me...?\n");

				param = BT_LE_CONN_PARAM_DEFAULT;
				param2 = BT_LE_CONN_PARAM_DEFAULT2;
				int number= 0;
				for(int i = 0; i <CONFIG_BT_MAX_CONN; i ++){
					//printk("[ejpark2] enter! %u %u %u\n", i, number, iter);
					if(default_conn[i] == NULL){
						printk("[ejpark2] enter!\n");
						err = bt_le_scan_stop();
						if (err) {
							//printk("[ejpark2]Stop LE scan failed (err %d)\n", err);
							continue;
						}
						scan_start = false;
						err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN,
								param, &default_conn[i]);
						if (err) {
							//printk("[ejpark2]Create conn failed (err %d)\n", err);
						}
						i = CONFIG_BT_MAX_CONN;
					}else{
						//printk("[ejpark2] enter!2\n");
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
	//	printk("[DEVICE]: %s, AD evt type %u, AD data len %u, RSSI %i\n",
				//dev, type, ad->len, rssi);

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
		//printk("Scanning failed to start (err %d)\n", err);
		return;
	}

	printk("[ejpark6]Scanning successfully started\n");
}
static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	struct bt_conn_info info = {0};
	int err;

	//printk("[ejpark2] Disconnected (reason 0x%02x)\n", reason);
	for(int i = 0; i<CONFIG_BT_MAX_CONN; i ++)
		if(conn == default_conn[i]){
			printk("[peer]disconnected\n");
			//test_ready = false;
			bt_conn_unref(conn);
			default_conn[i] = NULL;
			cnt = 0;
			iter -= 1;
			i = CONFIG_BT_MAX_CONN;
		}
	err = bt_conn_get_info(conn, &info);
	if (err) {
		//printk("Failed to get connection info (%d)\n", err);
		return;
	}

	/* Re-connect using same roles */
	if (info.role == BT_CONN_ROLE_MASTER) {
//k_sleep(K_SECONDS(20));
		start_scan();
	}
}

static void test_run1(struct bt_conn *conn, struct bt_gatt_throughput gatt_throughput)
{
	int err;
	uint64_t stamp;
	uint32_t delta;
	uint32_t data=0;
	uint32_t prog=0;

	/* a dummy data buffer */
	static char dummy[256];//ejpark0401

	/* wait for user input to continue */
	//	printk("Ready, press any key to start\n");
	//	console_getchar();//ejpark


	/* reset peer metrics */
			err = bt_gatt_throughput_write(&gatt_throughput, dummy, 1);
//	printk("[ejpark2] test_run\n");
	if (err) {
		//printk("[ejpark2]Reset peer metrics failed.\n");
		return;
	}
	int sec = 15000/iter;
	int temp_iter = iter;
	stamp= k_uptime_get_32();
	int break_time = 1;
	uint64_t stamp2;
	while(k_uptime_get_32()-stamp < 20000){
	stamp2 = k_uptime_get_32();		
		err = bt_gatt_throughput_write(&gatt_throughput, dummy, 244);//ejpark0401
		//while(k_uptime_get_32()-stamp2 <break_time);
		//k_msleep(break_time-k_uptime_get_32()+stamp2);
		k_sleep(K_MSEC(break_time));
		prog ++;
		data += 240;
		if (err) {
			//printk("[ejpark2]GATT write failed (err %d)\n", err);
			break;
		}
	}
	
delta = k_uptime_get_32()-stamp;
	printk("\n[ejpark2]Done\n");
	//for(int i = 0; i <CONFIG_BT_MAX_CONN; i ++)
		printk("[ejpark2][local] round %d: sent %u bytes (%u KB) in %u ms at %llu kbps\n",cnt,
				data, data / 1024, delta, ((uint64_t)data* 8 / delta));
	/* read back char from peer */
			err = bt_gatt_throughput_read(&gatt_throughput);
	for(int i = 0; i <CONFIG_BT_MAX_CONN; i ++)
		if(conn == default_conn[i]) test_ready[i] = false;
	if (err) {
		//printk("[ejpark2]GATT read failed (err %d)\n", err);
	}
	start_flag = true;
	//if(temp_iter < CONFIG_BT_MAX_CONN && start_flag && cnt%M == M-1)
	//	start_scan();
	cnt ++;
}
static void test_run(struct bt_conn *conn, struct bt_gatt_throughput gatt_throughput)
{
	int err;
	uint64_t stamp;
	uint32_t delta;
	uint32_t data=0;
	uint32_t prog=0;

	/* a dummy data buffer */
	static char dummy[256];//ejpark0401

	/* wait for user input to continue */
	//	printk("Ready, press any key to start\n");
	//	console_getchar();//ejpark


	/* reset peer metrics */
			err = bt_gatt_throughput_write(&gatt_throughput, dummy, 1);
//	printk("[ejpark2] test_run\n");
	if (err) {
		//printk("[ejpark2]Reset peer metrics failed.\n");
		return;
	}
	int sec = 15000/iter;
	int temp_iter = iter;
	stamp= k_uptime_get_32();
	int break_time = 1;
	uint64_t stamp2;
	while(k_uptime_get_32()-stamp < 20000){
	stamp2 = k_uptime_get_32();		
					err = bt_gatt_throughput_write(&gatt_throughput, dummy, 244);//244);//ejpark0401
		//while(k_uptime_get_32()-stamp2 <break_time);
		//k_msleep(break_time-k_uptime_get_32()+stamp2);
		k_sleep(K_MSEC(break_time));
		prog ++;
		data += 240;
		if (err) {
			//printk("[ejpark2]GATT write failed (err %d)\n", err);
			break;
		}
	}
	
delta = k_uptime_get_32()-stamp;
	printk("\n[ejpark2]Done\n");
	//for(int i = 0; i <CONFIG_BT_MAX_CONN; i ++)
		printk("[ejpark2][local] round %d: sent %u bytes (%u KB) in %u ms at %llu kbps\n",cnt,
				data, data / 1024, delta, ((uint64_t)data* 8 / delta));
	/* read back char from peer */
			err = bt_gatt_throughput_read(&gatt_throughput);
	for(int i = 0; i <CONFIG_BT_MAX_CONN; i ++)
		if(conn == default_conn[i]) test_ready[i] = false;
	if (err) {
		//printk("[ejpark2]GATT read failed (err %d)\n", err);
	}
	start_flag = true;
	//if(temp_iter < CONFIG_BT_MAX_CONN && start_flag && cnt%M == M-1)
	//	start_scan();
	cnt ++;
}

static bool le_param_req(struct bt_conn *conn, struct bt_le_conn_param *param)
{
	/* reject peer conn param request */
	return false;
	//return true;
}

void th0(void)
{
	int err;

	static struct bt_conn_cb conn_callbacks = {
		.connected = connected,
		.disconnected = disconnected,
		.le_param_req = le_param_req,
	};

	//printk("Starting Bluetooth Throughput example\n");

	//:console_init();

	bt_conn_cb_register(&conn_callbacks);
	
	err = bt_enable(NULL);
	if (err) {
		//printk("Bluetooth init failed (err %d)\n", err);
		return;
	}
	//printk("Bluetooth initialized\n");
	for(int i = 0; i <CONFIG_BT_MAX_CONN; i ++){
		default_conn[i] = NULL;
		err = bt_gatt_throughput_init(&gatt_throughput[i], &throughput_cb);
		test_ready[i]= false;
	}
	
	start_scan();
	
	if (err) {
		printk("[ejpark2]Throughput service initialization failed.\n");
		return;
	}
	for ( ;;) {
		flag = true;
		for(int i = 0; i <CONFIG_BT_MAX_CONN && flag; i ++)
			if(test_ready[i] == false) flag =false;
		if(flag){
			printk("true!\n");
			test_run(default_conn[0], gatt_throughput[0]);

			/* a dummy data buffer */
		}
		else {
			k_msleep(30);
			start_scan();
		}
	}
}
void th1(void)
{
	k_msleep(15);
	for ( ;;) {
		if(flag){
	k_msleep(15);
			printk("true!\n");
			test_run1(default_conn[1], gatt_throughput[1]);

			/* a dummy data buffer */
		}
		else {
			k_msleep(30);
		}
	}
}
K_THREAD_DEFINE(th0_id, STACKSIZE, th0, NULL, NULL, NULL,
                PRIORITY, 0, 0);
K_THREAD_DEFINE(th1_id, STACKSIZE, th1, NULL, NULL, NULL,
                PRIORITY, 0, 0);
