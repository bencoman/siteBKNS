// 2017.07.12.BenComan Sourced from... me-no-dev/BLE_GATT_Client.ino (https://gist.github.com/me-no-dev/5f8851e1e1112b7c3d9eefe6e1e1cf9d)
//     Successfully observes the advertisments from Evolving4SimpleBleDevice.ino and beacon-advertiser.ino

// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.



/****************************************************************************
*
* This file is for gatt client. It can scan ble device, connect one device,
*
****************************************************************************/

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "controller.h"

#include "bt.h"
#include "bt_trace.h"
#include "bt_types.h"
#include "btm_api.h"
#include "bta_api.h"
#include "bta_gatt_api.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"

#define GATTC_TAG "GATTC_DEMO"

///Declare static functions
static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
static void esp_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);
static void gattc_profile_a_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);
static void gattc_profile_b_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);


static esp_gatt_srvc_id_t alert_service_id = {
    .id = {
        .uuid = {
            .len = ESP_UUID_LEN_16,
            .uuid = {.uuid16 = 0x1811,},
        },
        .inst_id = 0,
    },
    .is_primary = true,
};

static esp_gatt_id_t notify_descr_id = {
    .uuid = {
        .len = ESP_UUID_LEN_16,
        .uuid = {.uuid16 = GATT_UUID_CHAR_CLIENT_CONFIG,},
    },
    .inst_id = 0,
};
#define BT_BD_ADDR_STR         "%02x:%02x:%02x:%02x:%02x:%02x"
#define BT_BD_ADDR_HEX(addr)   addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]

static bool connect = false;
static const char device_name[] = "Alert Notification";

static esp_ble_scan_params_t ble_scan_params = {
    .scan_type              = BLE_SCAN_TYPE_ACTIVE,
    .own_addr_type          = BLE_ADDR_TYPE_PUBLIC,
    .scan_filter_policy     = BLE_SCAN_FILTER_ALLOW_ALL,
    .scan_interval          = 0x50,
    .scan_window            = 0x30,
};

#define PROFILE_NUM 2
#define PROFILE_A_APP_ID 0
#define PROFILE_B_APP_ID 1

struct gattc_profile_inst {
    esp_gattc_cb_t gattc_cb;
    uint16_t gattc_if;
    uint16_t app_id;
    uint16_t conn_id;
    esp_bd_addr_t remote_bda;
};

/* One gatt-based profile one app_id and one gattc_if, this array will store the gattc_if returned by ESP_GATTS_REG_EVT */
static struct gattc_profile_inst gl_profile_tab[PROFILE_NUM] = {
    [PROFILE_A_APP_ID] = {
        .gattc_cb = gattc_profile_a_event_handler,
        .gattc_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
    },
    [PROFILE_B_APP_ID] = {
        .gattc_cb = gattc_profile_b_event_handler,
        .gattc_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
    },
};

static void gattc_profile_a_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    uint16_t conn_id = 0;
    esp_ble_gattc_cb_param_t *p_data = (esp_ble_gattc_cb_param_t *)param;

    switch (event) {
    case ESP_GATTC_REG_EVT:
        Serial.printf("REG_EVT\n");
        esp_ble_gap_set_scan_params(&ble_scan_params);
        break;
    case ESP_GATTC_OPEN_EVT:
        conn_id = p_data->open.conn_id;

        memcpy(gl_profile_tab[PROFILE_A_APP_ID].remote_bda, p_data->open.remote_bda, sizeof(esp_bd_addr_t));
        Serial.printf("ESP_GATTC_OPEN_EVT conn_id %d, if %d, status %d, mtu %d\n", conn_id, gattc_if, p_data->open.status, p_data->open.mtu);

        Serial.printf("REMOTE BDA  %02x:%02x:%02x:%02x:%02x:%02x\n",
                            gl_profile_tab[PROFILE_A_APP_ID].remote_bda[0], gl_profile_tab[PROFILE_A_APP_ID].remote_bda[1], 
                            gl_profile_tab[PROFILE_A_APP_ID].remote_bda[2], gl_profile_tab[PROFILE_A_APP_ID].remote_bda[3],
                            gl_profile_tab[PROFILE_A_APP_ID].remote_bda[4], gl_profile_tab[PROFILE_A_APP_ID].remote_bda[5]
                         );

        esp_ble_gattc_search_service(gattc_if, conn_id, NULL);
        break;
    case ESP_GATTC_SEARCH_RES_EVT: {
        esp_gatt_srvc_id_t *srvc_id = &p_data->search_res.srvc_id;
        conn_id = p_data->search_res.conn_id;
        Serial.printf("SEARCH RES: conn_id = %x\n", conn_id);
        if (srvc_id->id.uuid.len == ESP_UUID_LEN_16) {
            Serial.printf("UUID16: %x\n", srvc_id->id.uuid.uuid.uuid16);
        } else if (srvc_id->id.uuid.len == ESP_UUID_LEN_32) {
            Serial.printf("UUID32: %x\n", srvc_id->id.uuid.uuid.uuid32);
        } else if (srvc_id->id.uuid.len == ESP_UUID_LEN_128) {
            Serial.printf("UUID128: %x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n", srvc_id->id.uuid.uuid.uuid128[0],
                     srvc_id->id.uuid.uuid.uuid128[1], srvc_id->id.uuid.uuid.uuid128[2], srvc_id->id.uuid.uuid.uuid128[3],
                     srvc_id->id.uuid.uuid.uuid128[4], srvc_id->id.uuid.uuid.uuid128[5], srvc_id->id.uuid.uuid.uuid128[6],
                     srvc_id->id.uuid.uuid.uuid128[7], srvc_id->id.uuid.uuid.uuid128[8], srvc_id->id.uuid.uuid.uuid128[9],
                     srvc_id->id.uuid.uuid.uuid128[10], srvc_id->id.uuid.uuid.uuid128[11], srvc_id->id.uuid.uuid.uuid128[12],
                     srvc_id->id.uuid.uuid.uuid128[13], srvc_id->id.uuid.uuid.uuid128[14], srvc_id->id.uuid.uuid.uuid128[15]);
        } else {
            Serial.printf("ERROR: UNKNOWN LEN %d\n", srvc_id->id.uuid.len);
        }
        break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT:
        conn_id = p_data->search_cmpl.conn_id;
        Serial.printf("SEARCH_CMPL: conn_id = %x, status %d\n", conn_id, p_data->search_cmpl.status);
        esp_ble_gattc_get_characteristic(gattc_if, conn_id, &alert_service_id, NULL);
        break;
    case ESP_GATTC_GET_CHAR_EVT:
        if (p_data->get_char.status != ESP_GATT_OK) {
            break;
        }
        Serial.printf("GET CHAR: conn_id = %x, status %d\n", p_data->get_char.conn_id, p_data->get_char.status);
        Serial.printf("GET CHAR: srvc_id = %04x, char_id = %04x\n", p_data->get_char.srvc_id.id.uuid.uuid.uuid16, p_data->get_char.char_id.uuid.uuid.uuid16);

        if (p_data->get_char.char_id.uuid.uuid.uuid16 == 0x2a46) {
            Serial.printf("register notify\n");
            esp_ble_gattc_register_for_notify(gattc_if, gl_profile_tab[PROFILE_A_APP_ID].remote_bda, &alert_service_id, &p_data->get_char.char_id);
        }

        esp_ble_gattc_get_characteristic(gattc_if, conn_id, &alert_service_id, &p_data->get_char.char_id);
        break;
    case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
        uint16_t notify_en = 1;
        Serial.printf("REG FOR NOTIFY: status %d\n", p_data->reg_for_notify.status);
        Serial.printf("REG FOR_NOTIFY: srvc_id = %04x, char_id = %04x\n", p_data->reg_for_notify.srvc_id.id.uuid.uuid.uuid16, p_data->reg_for_notify.char_id.uuid.uuid.uuid16);

        esp_ble_gattc_write_char_descr(
                gattc_if,
                conn_id,
                &alert_service_id,
                &p_data->reg_for_notify.char_id,
                &notify_descr_id,
                sizeof(notify_en),
                (uint8_t *)&notify_en,
                ESP_GATT_WRITE_TYPE_RSP,
                ESP_GATT_AUTH_REQ_NONE);
        break;
    }
    case ESP_GATTC_NOTIFY_EVT:
        Serial.printf("NOTIFY: len %d, value %08x\n", p_data->notify.value_len, *(uint32_t *)p_data->notify.value);
        break;
    case ESP_GATTC_WRITE_DESCR_EVT:
        Serial.printf("WRITE: status %d\n", p_data->write.status);
        break;
    default:
        break;
    }
}

static void gattc_profile_b_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    uint16_t conn_id = 0;
    esp_ble_gattc_cb_param_t *p_data = (esp_ble_gattc_cb_param_t *)param;

    switch (event) {
    case ESP_GATTC_REG_EVT:
        Serial.printf("REG_EVT\n");
        break;
    case ESP_GATTC_OPEN_EVT:
        conn_id = p_data->open.conn_id;

        memcpy(gl_profile_tab[PROFILE_B_APP_ID].remote_bda, p_data->open.remote_bda, sizeof(esp_bd_addr_t));
        Serial.printf("ESP_GATTC_OPEN_EVT conn_id %d, if %d, status %d, mtu %d\n", conn_id, gattc_if, p_data->open.status, p_data->open.mtu);

        Serial.printf("REMOTE BDA  %02x:%02x:%02x:%02x:%02x:%02x\n",
                            gl_profile_tab[PROFILE_B_APP_ID].remote_bda[0], gl_profile_tab[PROFILE_B_APP_ID].remote_bda[1], 
                            gl_profile_tab[PROFILE_B_APP_ID].remote_bda[2], gl_profile_tab[PROFILE_B_APP_ID].remote_bda[3],
                            gl_profile_tab[PROFILE_B_APP_ID].remote_bda[4], gl_profile_tab[PROFILE_B_APP_ID].remote_bda[5]
                         );

        esp_ble_gattc_search_service(gattc_if, conn_id, NULL);
        break;
    case ESP_GATTC_SEARCH_RES_EVT: {
        esp_gatt_srvc_id_t *srvc_id = &p_data->search_res.srvc_id;
        conn_id = p_data->search_res.conn_id;
        Serial.printf("SEARCH RES: conn_id = %x\n", conn_id);
        if (srvc_id->id.uuid.len == ESP_UUID_LEN_16) {
            Serial.printf("UUID16: %x\n", srvc_id->id.uuid.uuid.uuid16);
        } else if (srvc_id->id.uuid.len == ESP_UUID_LEN_32) {
            Serial.printf("UUID32: %x\n", srvc_id->id.uuid.uuid.uuid32);
        } else if (srvc_id->id.uuid.len == ESP_UUID_LEN_128) {
            Serial.printf("UUID128: %x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n", srvc_id->id.uuid.uuid.uuid128[0],
                     srvc_id->id.uuid.uuid.uuid128[1], srvc_id->id.uuid.uuid.uuid128[2], srvc_id->id.uuid.uuid.uuid128[3],
                     srvc_id->id.uuid.uuid.uuid128[4], srvc_id->id.uuid.uuid.uuid128[5], srvc_id->id.uuid.uuid.uuid128[6],
                     srvc_id->id.uuid.uuid.uuid128[7], srvc_id->id.uuid.uuid.uuid128[8], srvc_id->id.uuid.uuid.uuid128[9],
                     srvc_id->id.uuid.uuid.uuid128[10], srvc_id->id.uuid.uuid.uuid128[11], srvc_id->id.uuid.uuid.uuid128[12],
                     srvc_id->id.uuid.uuid.uuid128[13], srvc_id->id.uuid.uuid.uuid128[14], srvc_id->id.uuid.uuid.uuid128[15]);
        } else {
            Serial.printf("ERROR: UNKNOWN LEN %d\n", srvc_id->id.uuid.len);
        }
        break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT:
        conn_id = p_data->search_cmpl.conn_id;
        Serial.printf("SEARCH_CMPL: conn_id = %x, status %d\n", conn_id, p_data->search_cmpl.status);
        esp_ble_gattc_get_characteristic(gattc_if, conn_id, &alert_service_id, NULL);
        break;
    case ESP_GATTC_GET_CHAR_EVT:
        if (p_data->get_char.status != ESP_GATT_OK) {
            break;
        }
        Serial.printf("GET CHAR: conn_id = %x, status %d\n", p_data->get_char.conn_id, p_data->get_char.status);
        Serial.printf("GET CHAR: srvc_id = %04x, char_id = %04x\n", p_data->get_char.srvc_id.id.uuid.uuid.uuid16, p_data->get_char.char_id.uuid.uuid.uuid16);

        if (p_data->get_char.char_id.uuid.uuid.uuid16 == 0x2a46) {
            Serial.printf("register notify\n");
            esp_ble_gattc_register_for_notify(gattc_if, gl_profile_tab[PROFILE_B_APP_ID].remote_bda, &alert_service_id, &p_data->get_char.char_id);
        }

        esp_ble_gattc_get_characteristic(gattc_if, conn_id, &alert_service_id, &p_data->get_char.char_id);
        break;
    case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
        uint16_t notify_en = 1;
        Serial.printf("REG FOR NOTIFY: status %d\n", p_data->reg_for_notify.status);
        Serial.printf("REG FOR_NOTIFY: srvc_id = %04x, char_id = %04x\n", p_data->reg_for_notify.srvc_id.id.uuid.uuid.uuid16, p_data->reg_for_notify.char_id.uuid.uuid.uuid16);

        esp_ble_gattc_write_char_descr(
                gattc_if,
                conn_id,
                &alert_service_id,
                &p_data->reg_for_notify.char_id,
                &notify_descr_id,
                sizeof(notify_en),
                (uint8_t *)&notify_en,
                ESP_GATT_WRITE_TYPE_RSP,
                ESP_GATT_AUTH_REQ_NONE);
        break;
    }
    case ESP_GATTC_NOTIFY_EVT:
        Serial.printf("NOTIFY: len %d, value %08x\n", p_data->notify.value_len, *(uint32_t *)p_data->notify.value);
        break;
    case ESP_GATTC_WRITE_DESCR_EVT:
        Serial.printf("WRITE: status %d\n", p_data->write.status);
        break;
    default:
        break;
    }
}

static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    uint8_t *adv_name = NULL;
    uint8_t adv_name_len = 0;
    switch (event) {
    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT: {
        //the unit of the duration is second
        uint32_t duration = 10;
        esp_ble_gap_start_scanning(duration);
        break;
    }
    case ESP_GAP_BLE_SCAN_RESULT_EVT: {
        esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;
          // For quick reference, scan_result->scan_rst {
          //     .esp_gap_search_evt_t search_evt;            /*!< Search event type */
          //     .esp_bd_addr_t bda;                          /*!< Bluetooth device address which has been searched */
          //     .esp_bt_dev_type_t dev_type;                 /*!< Device type */
          //     .esp_ble_addr_type_t ble_addr_type;          /*!< Ble device address type */
          //     .esp_ble_evt_type_t ble_evt_type;            /*!< Ble scan result event type */
          //     .int rssi;                                   /*!< Searched device's RSSI */
          //     .uint8_t  ble_adv[ESP_BLE_ADV_DATA_LEN_MAX + ESP_BLE_SCAN_RSP_DATA_LEN_MAX];     /*!< Received EIR */
          //     .int flag;                                   /*!< Advertising data flag bit */
          //     .int num_resps;                              /*!< Scan result number */
          //     .uint8_t adv_data_len;                       /*!< Adv data length */
          //     .uint8_t scan_rsp_len;                       /*!< Scan response length */
          //     }


        
        switch (scan_result->scan_rst.search_evt) {
        case ESP_GAP_SEARCH_INQ_RES_EVT:
Serial.printf("\n....search_evt=ESP_GAP_SEARCH_INQ_RES_EVT");
Serial.printf("\n....bda=");
            for (int i = 0; i < 6; i++) {
                Serial.printf("%x:", scan_result->scan_rst.bda[i]);
            }
Serial.printf("\n");


Serial.printf("\n....(esp_gap_search_evt_t)search_evt=%d",   scan_result->scan_rst.search_evt);            /*!< Search event type */
//Serial.printf("\n....(esp_bd_addr_t)bda=",                   scan_result->scan_rst.bda);                          /*!< Bluetooth device address which has been searched */
Serial.printf("\n....(esp_bt_dev_type_t)dev_type=%x",        scan_result->scan_rst.dev_type);                 /*!< Device type */
Serial.printf("\n....(esp_ble_addr_type_t)ble_addr_type=%x", scan_result->scan_rst.ble_addr_type);           /*!< Ble device address type */
Serial.printf("\n....(esp_ble_evt_type_t)ble_evt_type=%x",   scan_result->scan_rst.ble_evt_type);           /*!< Ble scan result event type */
Serial.printf("\n....(int)rssi=%d",                          scan_result->scan_rst.rssi);         /*!< Searched device's RSSI */
Serial.printf("\n....(uint8_t)ble_adv=");
for (int i = 0; i < scan_result->scan_rst.adv_data_len; i++) {
  Serial.printf("%x:", scan_result->scan_rst.ble_adv[i]); 
}/*!< Received EIR */
Serial.printf("\n....(int)flag=%x",                          scan_result->scan_rst.flag);         /*!< Advertising data flag bit */
Serial.printf("\n....(int)num_resps=%d",                     scan_result->scan_rst.num_resps);         /*!< Scan result number */
Serial.printf("\n....(uint8_t)adv_data_len=%d",               scan_result->scan_rst.adv_data_len);         /*!< Adv data length */
Serial.printf("\n....(uint8_t)scan_rsp_len=%d",              scan_result->scan_rst.scan_rsp_len);         /*!< Scan response length */
Serial.printf("\n");
        

            adv_name = esp_ble_resolve_adv_data(scan_result->scan_rst.ble_adv,
                                                ESP_BLE_AD_TYPE_NAME_CMPL, &adv_name_len);
            Serial.printf("Searched Device Name Len %d\n", adv_name_len);
            for (int j = 0; j < adv_name_len; j++) {
                Serial.printf("%c", adv_name[j]);
            }
Serial.println("|__!!");
 
            if (adv_name != NULL) {
                if (strcmp((char *)adv_name, device_name) == 0) {
                    Serial.printf("Searched device %s\n", device_name);
                    if (connect == false) {
                        connect = true;
                        Serial.printf("Connect to the remote device.\n");
                        esp_ble_gap_stop_scanning();
                        esp_ble_gattc_open(gl_profile_tab[PROFILE_A_APP_ID].gattc_if, scan_result->scan_rst.bda, true);
                        esp_ble_gattc_open(gl_profile_tab[PROFILE_B_APP_ID].gattc_if, scan_result->scan_rst.bda, true);
                    }
                    else {
                        Serial.println("NOCONNECT");
                    }
                }
            }
Serial.println("#####10");
            break;
        case ESP_GAP_SEARCH_INQ_CMPL_EVT:
Serial.println("#####11-ESP_GAP_SEARCH_INQ_CMPL_EVT");
            break;
        default:
Serial.println("#####12");
        
            break;
        }
Serial.println("#####13");
        break;
    }
    default:
Serial.println("#####14");
        break;
    }
}

static void esp_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    Serial.printf("EVT %d, gattc if %d\n", event, gattc_if);

    /* If event is register event, store the gattc_if for each profile */
    if (event == ESP_GATTC_REG_EVT) {
        if (param->reg.status == ESP_GATT_OK) {
            gl_profile_tab[param->reg.app_id].gattc_if = gattc_if;
        } else {
            Serial.printf("Reg app failed, app_id %04x, status %d\n",
                    param->reg.app_id, 
                    param->reg.status);
            return;
        }
    }

    /* If the gattc_if equal to profile A, call profile A cb handler,
     * so here call each profile's callback */
    do {
        int idx;
        for (idx = 0; idx < PROFILE_NUM; idx++) {
            if (gattc_if == ESP_GATT_IF_NONE || /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
                    gattc_if == gl_profile_tab[idx].gattc_if) {
                if (gl_profile_tab[idx].gattc_cb) {
                    gl_profile_tab[idx].gattc_cb(event, gattc_if, param);
                }
            }
        }
    } while (0);
}

void ble_client_appRegister(void)
{
    esp_err_t status;

    Serial.printf("register callback\n");

    //register the scan callback function to the gap moudule
    if ((status = esp_ble_gap_register_callback(esp_gap_cb)) != ESP_OK) {
        Serial.printf("ERROR: gap register error, error code = %x\n", status);
        return;
    }

    //register the callback function to the gattc module
    if ((status = esp_ble_gattc_register_callback(esp_gattc_cb)) != ESP_OK) {
        Serial.printf("ERROR: gattc register error, error code = %x\n", status);
        return;
    }
    esp_ble_gattc_app_register(PROFILE_A_APP_ID);
    esp_ble_gattc_app_register(PROFILE_B_APP_ID);
}

void gattc_client_test(void)
{
    esp_bluedroid_init();
    esp_bluedroid_enable();
    ble_client_appRegister();
}

void setup() {
  Serial.begin(115200);
  if(btStart()){
    gattc_client_test();
  }
   pinMode(LED_BUILTIN, OUTPUT);
   uint8_t myBTAddr = esp_bt_dev_get_address();
   Serial.printf("###### %d #####\n", myBTAddr"); 
}

void loop() {
  delay(500); digitalWrite(LED_BUILTIN, HIGH);    
  delay(500); digitalWrite(LED_BUILTIN, LOW);    
}
