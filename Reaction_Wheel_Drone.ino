#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include <Adafruit_L3GD20_U.h>
#include <Adafruit_10DOF.h>
#include <NewPing.h>
#include <Servo.h>
#include <LSM303.h>

/* Assign a unique ID to the sensors */
Adafruit_BMP085_Unified       bmp   = Adafruit_BMP085_Unified(18001);
Adafruit_L3GD20_Unified       gyro  = Adafruit_L3GD20_Unified(20);

LSM303 compass;

char report[80];

byte result;

#define TRIGGER_PIN  13
#define ECHO_PIN     12
#define MAX_DISTANCE 200

int hover = 10;

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

int speedF = 0; // Initialize the front motor
int speedB = 0; // Initialize the front motor
int speedS = 0; // Initialize the stabilizing motor
int timer = 0;
int timelimit = 1000*10;

Servo M1, M2, M3; //Create as much as Servoobject you want. You can controll 2 or more Servos at the same time

void setup(void)
{
  Serial.begin(115200);
  //start the Wire library;
  Wire.begin();
  //Assign pins to motors 1, 2, and 3
  M1.attach(8);
  M2.attach(9);
  M3.attach(10);
  compass.init();
  compass.enableDefault();
  delay(100);
}

void loop(void)
{
  /* Get a new sensor event */
  sensors_event_t event;
  compass.read();

  rotorSpeed();
  M1.writeMicroseconds(speedF);
  M2.writeMicroseconds(0);
  M3.writeMicroseconds(0);
  if(timer%100 == 0)printAcc();
  setTimer();
  if(timer > timelimit+5000){
    M1.writeMicroseconds(0);
    M2.writeMicroseconds(0);
    M3.writeMicroseconds(0);
    Serial.println("Finished");
    while(1);
  }
}

void rotorSpeed(){
  if(compass.a.x*0.000061 < 0){       //If the front lifts, reduce the front speed
      speedF--;                         //and increase the back speed
      speedB++;
    } else {                            //Vice versa if the back lifts
      speedF++;
      speedB--;
    }
  if(timer < timelimit){                //Run this for the time limit set
    if(compass.a.x*0.000061 < 0){       //If the front lifts, reduce the front speed
      speedF--;                         //and increase the back speed
      speedB++;
    } else {                            //Vice versa if the back lifts
      speedF++;
      speedB--;
    }
  } else {                              //After time limit is reached, run this
    if(sonar.ping_cm() < -hover/timelimit*(timer-timelimit)+100){
      speedF++;                         //The drone will descend from hovering height
      speedB++;                         //for 5 seconds until it reaches the ground
    } else {
      speedF--;
      speedB--;
    }
  }
  if(speedF > 1800){
    speedF = 1800;
  }
  if(speedB > 1800){
    speedB = 1800;
  }
  if(speedF < 1050){
    speedF = 1050;
  }
  if(speedB < 1050){
    speedB = 1050;
  }
}
void stabilizeSpeed(){
  
}

void setTimer(){
  if(timer < timelimit+5000)timer++;
}

void printGyro(sensors_event_t event){
  /* Display the results (gyrocope values in rad/s) */
  gyro.getEvent(&event);
  Serial.print(F("GYRO  "));
  Serial.print("X: "); Serial.print(event.gyro.x); Serial.print("  ");
  Serial.print("Y: "); Serial.print(event.gyro.y); Serial.print("  ");
  Serial.print("Z: "); Serial.print(event.gyro.z); Serial.print("  ");Serial.println("rad/s ");  

}

void printPress(sensors_event_t event){
  /* Display the pressure sensor results (barometric pressure is measure in hPa) */
  bmp.getEvent(&event);
  if (event.pressure)
  {
    /* Display atmospheric pressure in hPa */
    Serial.print(F("PRESS "));
    Serial.print(event.pressure);
    Serial.print(F(" hPa, "));
    /* Display ambient temperature in C */
    float temperature;
    bmp.getTemperature(&temperature);
    Serial.print(temperature);
    Serial.print(F(" C, "));
    /* Then convert the atmospheric pressure, SLP and temp to altitude    */
    /* Update this next line with the current SLP for better results      */
    float seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA;
    Serial.print(bmp.pressureToAltitude(seaLevelPressure,
                                        event.pressure,
                                        temperature)); 
    Serial.println(F(" m"));
  }
}

void printMag(){
  Serial.print(F("MAG  "));
  Serial.print("X: "); Serial.print(compass.m.x*0.00008, 3); Serial.print("  ");
  Serial.print("Y: "); Serial.print(compass.m.y*0.00008, 3); Serial.print("  ");
  Serial.print("Z: "); Serial.print(compass.m.z*0.00008, 3); Serial.print("  ");Serial.println("GAUSS ");  
}

void printAcc(){
  Serial.print(F("ACC  "));
  Serial.print("X: "); Serial.print(compass.a.x*0.000061, 3); Serial.print("  ");
  Serial.print("Y: "); Serial.print(compass.a.y*0.000061, 3); Serial.print("  ");
  Serial.print("Z: "); Serial.print(compass.a.z*0.000061, 3); Serial.print("  ");Serial.println("G ");  
}

void printPing(){
  Serial.print(F("PING  "));
  Serial.print("D: "); Serial.print(sonar.ping_cm()); Serial.print("  ");Serial.println("cm ");
}

void printFlightData(){
  Serial.print(F("SPEED  "));
  Serial.print("MF: "); Serial.print(speedF); Serial.print("  ");
  Serial.print("MB: "); Serial.print(speedB); Serial.print("  ");
  Serial.print("MS: "); Serial.print(speedS); Serial.print("  ");
  printPing();
  Serial.println("");
}

