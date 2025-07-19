
#include <Wire.h>
#include <vl53l4cx_class.h>

VL53L4CX tof_sensor_0(&Wire, A0);
VL53L4CX tof_sensor_1(&Wire1, A1);

int dist_0, pdist_0;
int dist_1, pdist_1;

const int trigger_distance = 200;

bool play_status = false;


// -------------------------------------------------------------------------- //

void setup() {

   Serial.begin(9600);

   // ToF Sensors
   Wire.begin();
   Wire1.begin();
   tof_init(tof_sensor_0);
   tof_init(tof_sensor_1);

   // Voice Module
   Serial1.begin(9600);
   volume(30);

}


void loop() {

   pdist_0 = dist_0;
   pdist_1 = dist_1;
   dist_0 = tof_read(tof_sensor_0);
   dist_1 = tof_read(tof_sensor_1);

   // sometimes the sensor seems to glitch and give a negative reading
   // so let's just ignore them
   dist_0 = dist_0 < 0 ? pdist_0 : dist_0;
   dist_1 = dist_1 < 0 ? pdist_1 : dist_1;

   play_status = check_playback(); 

   Serial.print(play_status);
   Serial.print(',');
   Serial.print(dist_0);
   Serial.print(',');
   Serial.print(dist_1);
   Serial.println();

   if(!play_status) {
      if(dist_0 < trigger_distance) {
         if(random(3)>1) {
            play(1);
         } else {
            play(2);
         }
      } else if(dist_1 < trigger_distance) {
         if(random(3)>1) {
            play(3);
         } else {
            play(4);
         }
      }
   }

   delay(10);

}


// Voice Sensor ------------------------------------------------------------- //

// Play an audio file by index
void play(uint8_t Track) {
   unsigned char play_cmd[6] = {0xAA,0x07,0x02,0x00,Track,Track+0xB3};
   Serial1.write(play_cmd,6);
}

// Set volume between 0 and 30
void volume(uint8_t vol) {
   unsigned char volume_cmd[5] = {0xAA,0x13,0x01,vol,vol+0xBE};
   Serial1.write(volume_cmd,5);
}

// Get playback status
bool check_playback() {
   unsigned char check_cmd[4] = {0xAA,0x01,0x00,0xAB};
   Serial1.write(check_cmd,4);
   uint8_t in_bytes[4];
   uint8_t i = 0;
   while(Serial1.available() > 0) {
      in_bytes[i] = Serial1.read();
      i++;
      if(i>3) { break; } // hack in case we get more bytes back than expected
   }
   return in_bytes[2];
}


// ToF Distance Sensor ------------------------------------------------------ //

// Initialise the sensor
void tof_init(VL53L4CX tof_sensor) {
   tof_sensor.begin();
   tof_sensor.VL53L4CX_Off();
   tof_sensor.InitSensor(0x12);
   tof_sensor.VL53L4CX_StartMeasurement();
}

// Get a distance mesaurement
int tof_read(VL53L4CX tof_sensor) {

   int distance;

   VL53L4CX_MultiRangingData_t MultiRangingData;
   VL53L4CX_MultiRangingData_t *pMultiRangingData = &MultiRangingData;
   uint8_t NewDataReady = 0;
   int no_of_object_found = 0, j;
   char report[64];
   int status;

   while (!NewDataReady) {
      status = tof_sensor.VL53L4CX_GetMeasurementDataReady(&NewDataReady);
   }

   if ((!status) && (NewDataReady != 0)) {
      status = tof_sensor.VL53L4CX_GetMultiRangingData(pMultiRangingData);
      distance = pMultiRangingData->RangeData[0].RangeMilliMeter;
      if (status == 0) {
         status = tof_sensor.VL53L4CX_ClearInterruptAndStartMeasurement();
      }
   }

   return(distance);
}