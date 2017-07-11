// 2017.07.08.BTC Culled from https://github.com/espressif/esp-idf/blob/master/examples/bluetooth/gatt_client/main/gattc_demo.c
//   squeezed on top of https://github.com/espressif/arduino-esp32/blob/master/libraries/SimpleBLE/examples/SimpleBleDevice/SimpleBleDevice.ino

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
#include "esp_gap_ble_api.h"
#include "esp_bt_main.h"

#include <gatt_api.h> // bluedroid include

static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);

static bool connect = false;
static const char device_name[] = "Alert Notification";

SimpleBLE ble;

void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    pinMode(0, INPUT_PULLUP);
    Serial.print("ESP32 SDK: ");
    Serial.println(ESP.getSdkVersion());
    ble.begin("Ben's SimpleBLE");
    esp_bluedroid_init();
    esp_bluedroid_enable();
    ble_client_appRegister();
    Serial.println("Press the button to change the device name");
}

void loop() {
    delay(500); digitalWrite(LED_BUILTIN, HIGH);
    delay(500); digitalWrite(LED_BUILTIN, LOW);
    //while(Serial.available()) Serial.write(Serial.read());
}

static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    uint8_t *adv_name = NULL;
    uint8_t adv_name_len = 0;
    Serial.print("esp_gap_cb!");
    switch (event) {
    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT: {
    Serial.print("ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT ");
        //the unit of the duration is second
        uint32_t duration = 10;
        esp_ble_gap_start_scanning(duration);
        break;
    }
    case ESP_GAP_BLE_SCAN_RESULT_EVT: {
   Serial.print("ESP_GAP_BLE_SCAN_RESULT_EVT ");
        esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;
        switch (scan_result->scan_rst.search_evt) {
        case ESP_GAP_SEARCH_INQ_RES_EVT:
     Serial.print("ESP_GAP_SEARCH_INQ_RES_EVT ");
           for (int i = 0; i < 6; i++) {
                Serial.printf("%x:", scan_result->scan_rst.bda[i]);
            }
            Serial.printf("\n");
            adv_name = esp_ble_resolve_adv_data(scan_result->scan_rst.ble_adv,
                                                ESP_BLE_AD_TYPE_NAME_CMPL, &adv_name_len);
            Serial.printf("Searched Device Name Len %d\n", adv_name_len);
            for (int j = 0; j < adv_name_len; j++) {
                Serial.printf("%c", adv_name[j]);
            }

            if (adv_name != NULL) {
                if (strcmp((char *)adv_name, device_name) == 0) {
                    Serial.printf("Searched device %s\n", device_name);
                    if (connect == false) {
                        connect = true;
                        Serial.printf("Connect to the remote device.\n");
                        esp_ble_gap_stop_scanning();
                   }
                }
            }
            break;
        case ESP_GAP_SEARCH_INQ_CMPL_EVT:
    Serial.print("ESP_GAP_SEARCH_INQ_CMPL_EVT ");
           break;
        default:
    Serial.print("DEFAULT1 ");
            break;
        }
        break;
    }
    default:
        break;
    }
    Serial.println();
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
}
