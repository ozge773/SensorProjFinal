#include <Wire.h>
//This codde is obtained from Pulsesensor.com
int PulseSensorPurplePin = A0;        // Pulse Sensor PURPLE WIRE connected to ANALOG PIN 0
int LED13 = 13;   //  The on-board Arduion LED


int Signal;                // holds the incoming raw data. Signal value can range from 0-1024
int Threshold = 200;            // Determine which Signal to "count as a beat", and which to ingore.


char string_temp[32];
char write_buffer[256];

unsigned long prev_micros = 0;
void setup() {
    Serial.begin(9600);
    // Begin I2c
    //pINMODE WAS OUTPUT
    pinMode(LED13,INPUT);
    Wire.begin();
    

}
 

void loop() {
    int Signal = analogRead(PulseSensorPurplePin);

    // Build Accel String
    
    Serial.println(Signal);   
    dtostrf(Signal,6,4,string_temp);
    sprintf(write_buffer, "%c %s",'A',string_temp);

    if(Signal > Threshold){                          // If the signal is above "550", then "turn-on" Arduino's on-Board LED.
     digitalWrite(LED13,HIGH);
    } else {
     digitalWrite(LED13,LOW);                //  Else, the sigal must be below "550", so "turn-off" this LED.
    }
    dtostrf(0,6,4,string_temp);
    sprintf(write_buffer, "%s %s",write_buffer,string_temp);
    dtostrf(0,6,4,string_temp);
    sprintf(write_buffer, "%s %s",write_buffer,string_temp);

    Wire.beginTransmission(0x1A);
    Wire.write(write_buffer);
    Wire.endTransmission(true);


    delay(10);
}





