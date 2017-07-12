//GPS output function 
//update with long type lat/lon output
//added speed, course & optional function check
//added calculation of bearing and distance
//under development


#include "./TinyGPS.h"
#include "./Difinitions.h"
#include "HardwareSerial.h"
HardwareSerial Serial2(2);


#define DEGTORAD 0.0174532925199432957f
#define RADTODEG 57.295779513082320876f

float flat,flon;
long lat,lon; // create variable for latitude and longitude object
unsigned long fix_age, time, date, speed, course;
unsigned long fed_chars; // the number of characters fed to the object
unsigned short sentences, failed_checksum; 
// the number of valid $GPGGA and $GPRMC sentences processed
// and the number of sentences that failed the checksum test

TinyGPS gps; // create gps object
 
void setup(){
  Serial.begin(115200); // connect serial
  Serial2.begin(115200);

}
 
void loop(){

    if(Serial2.available()){
      int c = Serial2.read();
      if (gps.encode(c)){
          gps.get_position(&lat,&lon); // get latitude and longitude

          // retrieves +/- lat/long in 100000ths of a degree
          gps.get_position(&lat, &lon, &fix_age);
          gps.f_get_position(&flat,&flon);

          // time in hhmmsscc, date in ddmmyy
          gps.get_datetime(&date, &time, &fix_age);

          // returns speed in 100ths of a knot
          speed = gps.speed();
 
          // course in 100ths of a degree
          course = gps.course();
          
          // display position
          Serial.print("Position: ");
          Serial.print("lat: ");Serial.print(lat);Serial.print(" ");// print latitude
          Serial.print("lon: ");Serial.println(lon); // print longitude
          //delay(1000);

          GeoLoc ArdLoc;  //GeoLoc defined in definition header
          ArdLoc.lat = flat;
          ArdLoc.lon = flon;
          return Ardloc;
          
          //stats for function check (opt)
          gps.stats(&chars, &sentences, &failed_checksum);
          
          if (fix_age == TinyGPS::GPS_INVALID_AGE)
          Serial.println("No fix detected");
          else if (fix_age > 5000)
          Serial.println("Warning: possible stale data!");
          else
          Serial.println("Data is current.");

          // Calculate Bearing
          float Bearing(struct GeoLoc &a, struct GeoLoc &b) {
          float y = sin(b.lon-a.lon) * cos(b.lat);
          float x = cos(a.lat)*sin(b.lat) - sin(a.lat)*cos(b.lat)*cos(b.lon-a.lon);
          return atan2(y, x) * RADTODEG;
          }

          // Calculate Distance
          float Distance(struct GeoLoc &a, struct GeoLoc &b) {
          const float R = 6371000; // km
          float p1 = a.lat * DEGTORAD;
          float p2 = b.lat * DEGTORAD;
          float dp = (b.lat-a.lat) * DEGTORAD;
          float dl = (b.lon-a.lon) * DEGTORAD;

          float x = sin(dp/2) * sin(dp/2) + cos(p1) * cos(p2) * sin(dl/2) * sin(dl/2);
          float y = 2 * atan2(sqrt(x), sqrt(1-x));

          return R * y;
}


      }

   }
  
}
