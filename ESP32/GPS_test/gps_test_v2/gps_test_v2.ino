#include "./TinyGPS.h"
#include "HardwareSerial.h"
HardwareSerial Serial2(2);

 
float flat,flon; // create variable for latitude and longitude object

TinyGPS gps; // create gps object
 
void setup(){
  Serial.begin(9600); // connect serial
  Serial2.begin(9600);

}
 
void loop(){

    if(Serial2.available()){
      int c = Serial2.read();
      if (gps.encode(c)){
          gps.f_get_position(&flat,&flon); // get latitude and longitude
          
          // display position
          Serial.print("Position: ");
          Serial.print("lat: ");Serial.print(flat);Serial.print(" ");// print latitude
          Serial.print("lon: ");Serial.println(flon); // print longitude
          //delay(1000);
      }

   }
  
}
