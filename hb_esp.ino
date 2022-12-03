#include <ThingSpeak.h>

#include <WiFiProv.h>

#include <WiFi.h>
#include <Wire.h>
#include <ThingSpeak.h>



struct hb_sensor{
  float hb=0;
};

hb_sensor heart_beat;
hb_sensor hb;

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
//int status = 0;
//int field[8] = {1,2,3,4,5,6,7,8};
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
    delay(30000);
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



void updateSensors(){

  /*set heart beat*/
  set_hb();
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
  if (message[0]== 'H'){
  hb.hb =strtof(end_ptr, NULL);
}

}



void setup() {
  // Begins Serial
  Serial.begin(115200);
  // Begins WiFi
  wifiSetup();
  // Sets I2C message reception behavior
  Wire.onReceive(onReceive);
  // Begins I2C
  Wire.setPins(5,4);
  Wire.begin(uint8_t(0x1A));
  // Initialize prev_millis
  prev_millis = millis();
  
  Serial.begin(9600);      
}


void thingspeak()
{
ThingSpeak.setField(1,float (hb.hb));

ThingSpeak.writeFields(CHANNEL_ID, API_WRITE); 

delay(15000);
}

void loop() {
  // Sets Accel, Gyro, Magno and BMP280 structs once per loop from most recent data.
  updateSensors();
  

  // Print sensor outputs as a comma seperated list

  thingspeak();

  delay(15000);
}
