// 2017.07.10.BenComan 
//    Starting withhttps://github.com/espressif/arduino-esp32/blob/master/libraries/SimpleBLE/examples/SimpleBleDevice/SimpleBleDevice.ino
//    evolved by replacing "ble.begin()" with "_init_gap()" from https://github.com/espressif/arduino-esp32/blob/master/libraries/SimpleBLE/src/SimpleBLE.cpp
//    and then chasing down unknown symbols. No other changes. Behaviour is identical to SimpleBleDevice.ino.
// 2017.07.11.BenComan 
//    Modified to broadcast custom data. Ref https://docs.mbed.com/docs/ble-intros/en/latest/Advanced/CustomGAP/
// 2017.07.11.BenComan
//    Removed "Local Name" (ID to be handled in application packet format.)
//    Experimented to determine maximimum size of Advertising Data /     
// 2017.07.11.BenComan
//    Try make advertising data dynamic. Not yet working.

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

// Sketch shows how SimpleBLE to advertise the name of the device and change it on button press
// Usefull if you want to advertise some short message
// Button is attached between GPIO 0 and GND and modes are switched with each press

#include "SimpleBLE.h"
#include "esp32-hal-log.h"

#include "bt.h"
#include "bta_api.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "bta_api.h"
#include "bt_trace.h"
#include "btc_manage.h"
#include "btc_gap_ble.h"

typedef union custom_data_u 
{   uint8_t       raw[10];
    struct 
    {   uint16_t      companyID;    //as reported by "Nordic nRF Connect" android app
        uint8_t       customdata[24];
    } fields;
}custom_data_t;

custom_data_t custom_data = 
{ .fields = 
  {   .companyID = 0xFFFF, //reserved by Bluetooth Specification for internal testing
      .customdata = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24} //maximum visible to nRFConnect android app
  }
};
    
static esp_ble_adv_data_t _adv_config = {
        .set_scan_rsp        = false,
        .include_name        = false,
        .include_txpower     = true,
        .min_interval        = 512,
        .max_interval        = 1024,
        .appearance          = 0,
        .manufacturer_len    = 0,
        .p_manufacturer_data = NULL,
        .service_data_len    = 0,
        .p_service_data      = NULL,
        .service_uuid_len    = 0,
        .p_service_uuid      = NULL,
        .flag                = (ESP_BLE_ADV_FLAG_GEN_DISC|ESP_BLE_ADV_FLAG_BREDR_NOT_SPT)
};

static esp_ble_adv_params_t _adv_params = {
        .adv_int_min         = 512,
        .adv_int_max         = 1024,
        .adv_type            = ADV_TYPE_NONCONN_IND,
        .own_addr_type       = BLE_ADDR_TYPE_PUBLIC,
        .peer_addr           = {0x00, },
        .peer_addr_type      = BLE_ADDR_TYPE_PUBLIC,
        .channel_map         = ADV_CHNL_ALL,
        .adv_filter_policy   = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static void _on_gap(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param){
    if(event == ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT){
        esp_ble_gap_start_advertising(&_adv_params);
    }
}

SimpleBLE ble;

void onButton(){
    String out = "BLE32 at: ";
    out += String(millis() / 1000);
    Serial.println(out);
    ble.begin(out);
}

static bool _init_gap(const char * name){
    if(!btStarted() && !btStart()){
        log_e("btStart failed");
        return false;
    }
    
    esp_bluedroid_status_t bt_state = esp_bluedroid_get_status();
    if(bt_state == ESP_BLUEDROID_STATUS_UNINITIALIZED){
        if (esp_bluedroid_init()) {
            log_e("esp_bluedroid_init failed");
            return false;
        }
    }
    
    if(bt_state != ESP_BLUEDROID_STATUS_ENABLED){
        if (esp_bluedroid_enable()) {
            log_e("esp_bluedroid_enable failed");
            return false;
        }
    }
    
    if(esp_ble_gap_set_device_name(name)){
        log_e("gap_set_device_name failed");
        return false;
    }

    _adv_config.p_manufacturer_data = custom_data.raw;
    _adv_config.manufacturer_len = sizeof(custom_data_t);
    if(esp_ble_gap_config_adv_data(&_adv_config)){
        log_e("gap_config_adv_data failed");
        return false;
    }
    
    if(esp_ble_gap_register_callback(_on_gap)){
        log_e("gap_register_callback failed");
        return false;
    }
    
    return true;
}

void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.print("ESP32 SDK: ");
    Serial.println(ESP.getSdkVersion());
    //ble.begin("WEMOS SimpleBLE");
    //_init_gap("LOCAL NAME");
}

int counter = 0;
void loop() {
    counter = counter + 1;
    custom_data.fields.customdata[0] = counter;
    if(esp_ble_gap_config_adv_data(&_adv_config))
    {   log_e("gap_config_adv_data failed");
        exit(1);
    }
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
}
