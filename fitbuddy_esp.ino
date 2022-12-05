#include <ThingSpeak.h>

#include <WiFiProv.h>

#include <WiFi.h>
#include <Wire.h>
#include <ThingSpeak.h>



struct hb_sensor{
  float hb;// equal to 0 before
};

hb_sensor heart_beat;
hb_sensor hb;//equal to 0 before

struct light_sensor{
  float intensity;
};

light_sensor light;
light_sensor luce;

struct temp_sensor{
  float temperature;
};


temp_sensor temp_measure;
temp_sensor temp;

struct thermis_raw{
  float raw_temp;
};
thermis_raw measure_tmp;
thermis_raw tmp;

float ard_millis=0; // loop duration from arduino
float prev_millis=0; // For timers



float estimated_altitude = 0;

//****************************************************************************
// Begin Wifi Implementation
// Feel free to see or change how the esp32 connects to PAWS-Secure,
// but this code is provided functional.
//
// To authenticate with the school's network, the user must supply EAP_IDENTITY,
// EAP_USERNAME and EAP_PASSWORD.
// SSO = Single Sign On, the account used for most logins at UGA
// EAP_IDENTITY = SSO username
// EAP_USERNAME = SSO username
// EAP_PASSWORD = SSO password

#include "esp_wpa2.h"
//for the sake of security this part should be filled by user
#define EAP_IDENTITY ""
#define EAP_USERNAME ""
#define EAP_PASSWORD ""
#define MAX_DISCONNECTS 4
const char* ssid = "PAWS-Secure";
unsigned char disconnectNum = 0;
WiFiClient client;

// Use for writing to thingspeak
//unsigned long myChannelNumber = 1;
//const char * myWriteAPIKey = "";
//const long CHANNEL = ;
//const char *WRITE_API = "";
#define API_WRITE "9LATEMHGSQLRWJVV"
#define CHANNEL_ID 1968842

/**
 * @brief Executes when a WiFi connection event occurs.
 * 
 * @param event WiFi connection event.
 */
void WiFiStationConnected(WiFiEvent_t event){
  Serial.println("Connected to AP successfully!");
}

/**
 * @brief Executes when a IP assignment event occurs.
 * 
 * @param event IP assignment event
 */
void WiFiGotIP(WiFiEvent_t event){
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
/**
 * @brief Executes when a WiFi disconnection event occurs, automatically reconnects.
 * 
 * @param event WiFi disconnection event
 */
void WiFiStationDisconnected(WiFiEvent_t event){
  Serial.println("Disconnected from WiFi access point");
  Serial.print("WiFi lost connection.");
  Serial.println("Trying to Reconnect");
  if (disconnectNum < MAX_DISCONNECTS) {
    WiFi.begin(ssid, WPA2_AUTH_PEAP, EAP_IDENTITY, EAP_USERNAME, EAP_PASSWORD);
    delay(10);// it was 30000
  }
  disconnectNum++;
}

/**
 * @brief Sets up WiFi connection.
 * 
 */
void wifiSetup() {
  // Removes previous WiFi config if any
  WiFi.disconnect(true);
  // Sets events and functions to be called when events occur
  WiFi.onEvent(WiFiStationConnected, ARDUINO_EVENT_WIFI_STA_CONNECTED);
  WiFi.onEvent(WiFiGotIP, ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(WiFiStationDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  // Sets WiFi mode to STA, a device that can use 802.11 WiFi protocol
  WiFi.mode(WIFI_STA);
  // Begin WiFi
  WiFi.begin(ssid, WPA2_AUTH_PEAP, EAP_IDENTITY, EAP_USERNAME, EAP_PASSWORD);
  // Initializes Thingspeak
  ThingSpeak.begin(client);
}

// End WiFi Implementation
//****************************************************************************

/**
 * @brief Called when I2C message is recieved.
 * 
 * @param len Length of incoming message.
 */
void onReceive(int len){
  // Creates char array to store message in
  char message[len];
  // Iterator
  char num = 0;
  // Writes each received char to array at index num
  while(Wire.available()){
    message[num] = Wire.read();
    num++;
  }
  // Parses message from char array to series of sensor values
  parseMessage(message, len);
}

void set_hb(){
  heart_beat.hb = hb.hb;

}

void set_light(){
  light.intensity = luce.intensity;
}

void set_temp(){
  temp_measure.temperature = temp.temperature;
}


void set_raw_temp(){
  measure_tmp.raw_temp = tmp.raw_temp;
}


void updateSensors(){

  /*set heart beat*/
  set_hb();
  set_light();
  set_temp();
  set_raw_temp();
}

/**
 * @brief Parses Arduino I2C messages into suitable data.
 * 
 * @param message Message from the Arduino.
 * @param len Length of recieved message.
 * @return true If parse is succesful.
 * @return false If parse is unsuccesful.
 */
bool parseMessage(char message[], int len) {
  char *end_ptr;
  char msg[len-1];
  for(int i=1; i < len; i++){
    msg[i-1] = message[i];
  }
  switch (message[0]){
    case ('H'):
      hb.hb =strtof(msg, NULL);//msg null
      return true;
    case ('L'):
      //luce.intensity=strtof(msg, NULL);
      luce.intensity=strtof(msg, &end_ptr);
      return true;

    case('T'):
      temp.temperature = strtof(msg, NULL);
      return true;
    case('R'):
      tmp.raw_temp = strtof(msg, NULL);
    default:
      return false;

  }//switch 

}//bool
int prrev_millis;
int prev_millis1;
int prev_millis2;

  

void setup() {
  // Begins Serial
  Serial.begin(115200);
  // Begins WiFi
  wifiSetup();
  // Sets I2C message reception behavior
  Wire.onReceive(onReceive);
  // Begins I2C
  Wire.setPins(3,2);
  Wire.begin(uint8_t(0x1A));
  // Initialize prev_millis
  prev_millis1 = millis();
  prev_millis2 = millis();
  
       
}

float c_t ;
float raw_temperature;
float lsm35;
void printComplementary(){
  int alpha = 0.4;
  float dt = (millis() - prev_millis)/1000;
  raw_temperature = tmp.raw_temp*100 /1023.0;
  lsm35 = (temp.temperature *(5/10.24))+3.0;
  c_t = (alpha * raw_temperature) + (1-alpha)* lsm35;
  //Serial.println(c_t);

  delay (10);

}

int count_hb=0;
void thingspeak()
{
    //
    //float val;
    //val = map(light, light_ambient, dark_ambient, -50, 100);
    /*if (millis()- prev_millis1 >60000){
        ThingSpeak.setField(1, count_hb); //heart beat
        prev_millis1 = millis();
    } */


    ThingSpeak.setField(1, float(hb.hb)); //heart beat
    ThingSpeak.setField(2,float (luce.intensity)); //heart beat
    ThingSpeak.setField(3,float (c_t)); //heart beat


    ThingSpeak.writeFields(CHANNEL_ID, API_WRITE); 

    //delay(10);//it was 1500
}



void loop() {
  // Sets Accel, Gyro, Magno and BMP280 structs once per loop from most recent data.
  /*
  updateSensors();
  Serial.print("hear beat\t");
  Serial.print(hb.hb);
  Serial.print(" ");
  Serial.print("light\t");
  Serial.print(luce.intensity);
  Serial.print(" ");
  Serial.print("temperature LSM35:\t");
  Serial.print(lsm35);
  Serial.print(" ");
  Serial.print("temperature raw:\t");
  Serial.println(raw_temperature);*/

 

  Serial.print(hb.hb);
  Serial.print(" ");

  Serial.print(luce.intensity);
  Serial.print(" ");

  Serial.print(lsm35);
  Serial.print(" ");

  Serial.println(raw_temperature);

  

  if (hb.hb>500){
    count_hb = count_hb +1;

  }

  if (millis()- prev_millis2 >1000){
       thingspeak();
       prev_millis2 = millis();
  }
  printComplementary();
 

  delay(10);
}
