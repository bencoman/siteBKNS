#include "./TinyGPS.h"
#include "HardwareSerial.h"
HardwareSerial Serial2(2);

 
long lat,lon; // create variable for latitude and longitude object

TinyGPS gps; // create gps object
 
void setup(){
  Serial.begin(9600); // connect serial
  Serial2.begin(9600);

}
 
void loop(){

    if(Serial2.available()){
      int c = Serial2.read();
      if (gps.encode(c)){
          gps.get_position(&lat,&lon); // get latitude and longitude
          
          // display position
          Serial.print("Position: ");
          Serial.print("lat: ");Serial.print(lat);Serial.print(" ");// print latitude
          Serial.print("lon: ");Serial.println(lon); // print longitude
          //delay(1000);
      }

   }
  
}
