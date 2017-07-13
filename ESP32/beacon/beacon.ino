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
#include "esp_bt_device.h"
#include "esp_bt_defs.h"
#include "bta_api.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "bta_api.h"
#include "bt_trace.h"
#include "btc_manage.h"
#include "btc_gap_ble.h"

///////////////////////////////////////
///////////////// BEACON PACKET FORMAT
///////////////////////////////////////

enum BeaconTypes
{   UnconfiguredBEACON            = 0,
    ConeBEACON                    = 1,
    PersonnelBEACON               = 2,
    LightVehicleBEACON            = 3,   
    SurfaceMiningEquipmentBEACON  = 4
};

enum BeaconTypes beaconType;

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
        long          gpsLatitute;                  // X, scale 100000/0.00001m
        long          gpsLongitude;                 // Y, scale 100000/0.00001m
        uint8_t       gpsForwardSpeed;              //    scale 1m (not part of exclusion zone calcualtion)
        uint8_t       forwardExclusionZoneDistance; // F, scale 1m
        uint8_t       rearExclusionZoneDistance;    // L, scale 1m 
        uint8_t       sideExclusionZoneDistance;    // W, scale 1m
        uint8_t       packetVersion;                // This is siteBKNS Packet Format Version 2
        uint8_t       watchdogCounter;
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
            .packetVersion                = 2,
            .watchdogCounter              = 0
      }
  };



////////////// GPS
#include "./TinyGPS.h"
#include "HardwareSerial.h"
HardwareSerial Serial2(2);

float flat,flon, bearing; // create variable for latitude and longitude object
unsigned long time, date, speed, course, distance;

TinyGPS gps; // create gps object

long lat, lon;

void 
gps_setup(){
  Serial2.begin(9600);   //Do not change, must match factory setting.
}

void
gps_debug_passthrough()
{
    while (Serial2.available())         // If anything comes in Serial2 (pins 16-Rx & 17-Tx) 
      {Serial.write(Serial2.read());};  // read it and send it out Serial (USB)
    Serial.println();
    Serial.println();
}

void 
gps_read_into( beacon_data_t * beaconData) 
{
    //gps_debug_passthrough(); return;

    while(Serial2.available())
    {
      int c = Serial2.read();      
      if (gps.encode(c)) // do we have a valid gps data that we can encode
      {
          //gps.get_position(&lat,&lon); // get latitude and longitude
          gps.get_position( &beaconData->fields.gpsLatitute, &beaconData->fields.gpsLongitude );
          Serial.printf("Position - lat: %ld lon: %ld \n", beaconData->fields.gpsLatitute, beaconData->fields.gpsLongitude);



          // returns speed in 100ths of a knot
          //speed = gps.speed();

          // course in 100ths of a degree
          //course = gps.course();
          

          //beaconData->fields.gpsForwardBearing            = beaconData->fields.gpsForwardBearing;
          //beaconData->fields.gpsLatitute                  = beaconData->fields.gpsLatitute;  // times by GPS_SCALE;
          //beaconData->fields.gpsLongitude                 = beaconData->fields.gpsLongitude; // times by GPS_SCALE;
          //beaconData->fields.gpsForwardSpeed              = beaconData->fields.gpsForwardSpeed;
          //beaconData->fields.forwardExclusionZoneDistance = beaconData->fields.forwardExclusionZoneDistance;
          //beaconData->fields.rearExclusionZoneDistance    = beaconData->fields.rearExclusionZoneDistance;
          //beaconData->fields.sideExclusionZoneDistance    = beaconData->fields.sideExclusionZoneDistance;
 

         // Calculate Bearing
         // float Bearing(struct Loc &a, struct Loc &b) {
         // float y = sin(b.lon-a.lon) * cos(b.lat);
         // float x = cos(a.lat)*sin(b.lat) - sin(a.lat)*cos(b.lat)*cos(b.lon-a.lon);
         // return atan2(y, x) * RADTODEG;
         // } 
      }
   } 
}


//////////////// END GPS



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

static void
set_gap_device_name(const char * name){
    if(esp_ble_gap_set_device_name(name)){
        log_e("gap_set_device_name failed");
    }
}

static bool 
bluetooth_GAP_setup(const char * name){
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

    set_gap_device_name(name);

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

void
watchdog_into( beacon_data_t * beaconData )
{
    static long counter = 0;  
    counter = counter + 1;    
    beaconData->fields.watchdogCounter = counter;
}



/////////////////////////////////////////////////////
// PROTOYPE DEMOSTRATION SUPPORT
////////////////////////////////////////////////////
int blinkRate = 1000;

enum BeaconDemoRoles
{   
    DemoUnknown = 00,
    DemoCone1   = 10,
    DemoPerson1 = 11,       
    DemoLV1     = 12,
    DemoSME1    = 13,
    DemoCone2   = 20,
    DemoPerson2 = 21,       
    DemoLV2     = 22,
    DemoSME2    = 23,
};

enum BeaconDemoRoles demoRole;

//Preset name and beacon type for prototype demo.  Can post configure to show over-the-air configuration.
void
set_protoype_demo_role()
{   
    char board_address_s[40];  //slightly oversized
    snprintf(board_address_s, sizeof(board_address_s), "BD ADDR: " ESP_BD_ADDR_STR "\n", ESP_BD_ADDR_HEX(esp_bt_dev_get_address()));
    Serial.print(board_address_s);
    if (strcmp("30:ae:a4:04:39:fa",board_address_s)) { //Demo Cone 1
      Serial.println(" SET UP DEMO CONE 1");
      beaconType = ConeBEACON;
      set_gap_device_name("C0001");  
      blinkRate = 250;   
    };
    if (strcmp("24:0a:c4:05:b1:42",board_address_s)) { //Demo Cone 1
      Serial.println(" SET UP DEMO CONE 1");
      beaconType = PersonnelBEACON;
      set_gap_device_name("P0099");   
      blinkRate = 500;  
    };

    
    /*
      switch(demoRole){
      case DemoUnknown: break;
      case DemoCone1: demoCone1_setup(); break;
      case DemoPerson1: break;       
      case DemoLV1: break;
      case DemoSME1: break;
      case DemoCone2: break;
      case DemoPerson2: break;      
      case DemoLV2: break;
      case DemoSME2: break;
     */
}

void
demoCone1_setup()
{
  Serial.println("SETTING UP DEVICE FOR DEMO CONE 1");
}

void
demoCone1_loop()
{
  
}

 
//////////////////////////////
// ARDUINO SUPPORT FUNCTIONS
//////////////////////////////

void 
setup() {
    Serial.begin(9600);
    Serial.setDebugOutput(true);
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.print("ESP32 SDK: ");
    Serial.println(ESP.getSdkVersion());
    //esp_log_level_set("*", ESP_LOG_VERBOSE);
    gps_setup();
    bluetooth_GAP_setup("XXXXX"); //UNconfigure node name - five characters

    //For prototype demonstration
    set_protoype_demo_role();
}

void 
loop() {   
    // Fill the advertisement packet with GPS data
    gps_read_into( &myBeaconData );
    watchdog_into( &myBeaconData );
    
    //The advertisement fires regularly after setup, just update the data it sends 
    //advertisement_config.p_manufacturer_data field points to myBeaconData
    if(esp_ble_gap_config_adv_data(&advertisement_config)) 
    {   log_e("gap_config_adv_data failed");
        exit(1);
    }

    

    
    delay(blinkRate); digitalWrite(LED_BUILTIN, HIGH); 
    delay(blinkRate); digitalWrite(LED_BUILTIN, LOW);
}
