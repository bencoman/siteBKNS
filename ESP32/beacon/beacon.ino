// 2017.07.11.BenComan - Derived Evolving4SimpleBleDevice
//    Added GPS structure 

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

enum BeaconTypes
{   ConeBEACON = 1,
    PersonnelBEACON = 2,
    LightVehicleBEACON = 3,   
    SurfaceMiningEquipmentBEACON = 4
};

// BEACON WIRE PACKET FORMAT (VERSION 2)
// With five character local name, there remain two spare bytes (according to "nRF Connect" android app)
// Format aligned with... Anurag, Gosh, Bandyopadhyay, "GPS based Vehicular Collision Warning System using IEEE 802.15.4 MAC/PHY Standard"
char * localName = "C9999";
int LOCALNAMESIZE = sizeof(localName);      //use for strncpy()
#define GPS_SCALE 100000

typedef union   
{   uint8_t           raw[27]; 
    struct 
    {   uint16_t      companyID ;                   // Ref https://www.bluetooth.com/specifications/assigned-numbers/company-identifiers
        uint16_t      gpsForwardBearing;            // Azimuth
        uint32_t      gpsLatitute;                  // X, scale 100000/0.00001m
        uint32_t      gpsLongitude;                 // Y, scale 100000/0.00001m
        uint8_t       gpsForwardSpeed;              //    scale 1m (not part of exclusion zone calcualtion)
        uint8_t       forwardExclusionZoneDistance; // F, scale 1m
        uint8_t       rearExclusionZoneDistance;    // L, scale 1m 
        uint8_t       sideExclusionZoneDistance;    // W, scale 1m
        uint8_t       packetVersion;                // This is siteBKNS Packet Format Version 2
    } fields;
}beacon_data_t;

// BEACON DEFAULT DATA CAN BE USED TO OBSERVE ALIGNMENT PACKING OF WIRE PROTOCOL  
beacon_data_t myBeaconData = 
  { .fields = 
      {     .companyID                    = 0xFFFF,    //reserved by Bluetooth Specification for internal testing
            .gpsForwardBearing            = 0x1111,
            .gpsLatitute                  = 0x22222222,
            .gpsLongitude                 = 0x33333333,
            .gpsForwardSpeed              = 0x44,
            .forwardExclusionZoneDistance = 0xAA,
            .rearExclusionZoneDistance    = 0xBB,
            .sideExclusionZoneDistance    = 0xCC,
            .packetVersion                = 2
      }
  };

// BLE GAP ADVERTISING CONFIGURATION
static esp_ble_adv_data_t advertisement_config = {
        .set_scan_rsp        = false,
        .include_name        = true,
        .include_txpower     = true,      // !!!! TX-power can help with backup distance calculation in GPS blackspots
        .min_interval        = 512,
        .max_interval        = 1024,
        .appearance          = 0,
        .manufacturer_len    = sizeof(beacon_data_t),
        .p_manufacturer_data = myBeaconData.raw,    //update myBeaconData then call esp_ble_gap_config_adv_data(&advertisement_config) 
        .service_data_len    = 0,
        .p_service_data      = NULL,
        .service_uuid_len    = 0,
        .p_service_uuid      = NULL,
        .flag                = (ESP_BLE_ADV_FLAG_GEN_DISC|ESP_BLE_ADV_FLAG_BREDR_NOT_SPT)
};

static esp_ble_adv_params_t advertisement_params = {
        .adv_int_min         = 512,
        .adv_int_max         = 1024,
        .adv_type            = ADV_TYPE_NONCONN_IND,
        .own_addr_type       = BLE_ADDR_TYPE_PUBLIC,
        .peer_addr           = {0x00, },
        .peer_addr_type      = BLE_ADDR_TYPE_PUBLIC,
        .channel_map         = ADV_CHNL_ALL,
        .adv_filter_policy   = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

/////////////////////////
// BLE SUPPORT FUNCTIONS

static void 
_on_gap(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param){
    if(event == ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT){
        esp_ble_gap_start_advertising(&advertisement_params);
    }
}

static bool 
initialize_ble_gap(const char * name){
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

    if(esp_ble_gap_config_adv_data(&advertisement_config)){
        log_e("gap_config_adv_data failed");
        return false;
    }
    
    if(esp_ble_gap_register_callback(_on_gap)){
        log_e("gap_register_callback failed");
        return false;
    }
    
    return true;
}

///////////////////////////
// GPS INPUT PARSING

void 
gps_read_into( beacon_data_t * beaconData)
{   
    static long counter = 0;  //example only
    counter = counter + 1;    //example only
    Serial.println(counter);
    //@Nancy, can you fill out the following with real GPS info?
    beaconData->fields.gpsForwardBearing            = counter;
    beaconData->fields.gpsLatitute                  = beaconData->fields.gpsLatitute;  // times by GPS_SCALE;
    beaconData->fields.gpsLongitude                 = beaconData->fields.gpsLongitude; // times by GPS_SCALE;
    beaconData->fields.gpsForwardSpeed              = beaconData->fields.gpsForwardSpeed;
    beaconData->fields.forwardExclusionZoneDistance = beaconData->fields.forwardExclusionZoneDistance;
    beaconData->fields.rearExclusionZoneDistance    = beaconData->fields.rearExclusionZoneDistance;
    beaconData->fields.sideExclusionZoneDistance    = beaconData->fields.sideExclusionZoneDistance;
}

 
//////////////////////////////
// ARDUINO SUPPORT FUNCTIONS

void 
setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.print("ESP32 SDK: ");
    Serial.println(ESP.getSdkVersion());
    //esp_log_level_set("*", ESP_LOG_VERBOSE);

    initialize_ble_gap("C9999");
 }


void 
loop() {
    gps_read_into( &myBeaconData );
    if(esp_ble_gap_config_adv_data(&advertisement_config))
    {   log_e("gap_config_adv_data failed");
        exit(1);
    }
    delay(500); digitalWrite(LED_BUILTIN, HIGH); 
    delay(500); digitalWrite(LED_BUILTIN, LOW);
}
