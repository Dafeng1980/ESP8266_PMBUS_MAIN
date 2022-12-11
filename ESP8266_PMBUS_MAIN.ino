/*   ESP8266 model: ESP-01S Board SDA = 0; SCL = 2; ESP-12F Board SDA = 4; SCl = 5;
 *               HEKR 1.1 Board  SDA = 0; SCL = 13; LED = 4 Button = 14;
 *               Using the ESP8266 HEKR 1.1 Board (Purple);               
 *   https://play.google.com/store/apps/details?id=com.app.vetru.mqttdashboard  Mqtt Dashboard For  Android Iot APP
 *   Author Dafeng 2022
*/


#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ArduinoLog.h>
#include <PubSubClient.h>

#define Base_Topic "npi/"
#define TWI_BUFFER_SIZE 256
#define PEC_ENABLE  1      //Smbus PEC(Packet Error Code) support. 1 = Enabled, 0 = Disabled.
#define PEC_DISABLE  0
#define PS_I2C_ADDRESS 0x58 
#define PS_PARTNER_ADDRESS 0x5B
#define MSG_BUFFER_SIZE  (1024)
#define UI_BUFFER_SIZE (256)
#define I2C_NOSTOP 0
#define LOG_LEVEL LOG_LEVEL_VERBOSE
//#define MQTT_MAX_PACKET_SIZE 1024

static struct PowerPmbus
{
  float inputV;
  float inputA;
  float inputP;
  float inputE;
  float outputV;
  float outputVsb;
  float outputA;
  float outputAsb;
  float outputP;
  float outputE;
  float fanSpeed;
  float temp1;
  float temp2;
  float temp3;
  uint16_t statusWord;
  uint8_t i2cAddr;  
}pd;

uint8_t smbus_data[256];
static uint8_t eepbuffer[256];
static uint8_t key = 0;
static uint8_t ps_i2c_address;
static uint8_t ps_patner_address;
static int eeprom_address;
static uint16_t pmInterval = 1000;  //PMbus refresh rate (miliseconds) 

static bool Protocol = true;   // If true, endTransmission() sends a stop message after transmission, releasing the I2C bus.
static bool wifistatus = true;
static bool mqttflag = false;
static bool dataflag = true;
static bool buttonflag = true;
static bool scani2c = true;    //initialze i2c address, 
static bool pmbusflag = true;
static bool smbuscomun = false;
static bool statusflag = true;
static bool ledstatus = true;
static bool pecflag = true;
static bool eepromsize = true;   // true (eeprom data size > 0x100). 
static bool standbyflag = false;
static bool subsmbusflag = false;
static bool subscpiflag = false;
static bool serialflag = false;
static bool expandengery = false;
static bool expandsensor = false;
static bool setscpicurr = false;
bool currlh = true;

const char* ssid = "FAIOT";           // Enter your WiFi name 
const char* password = "20212021";    // Enter WiFi password
//const char* ssid = "CMCC-kS9s";           //
//const char* password = "DA431431";    // 
const char* mqtt_user = "dfiot";      //Raspberry MQTT Broker
const char* mqtt_password = "123abc";
const char* mqtt_server = "192.168.200.2";
//const char* mqtt_server = "192.168.12.1";
const uint16_t mqtt_port =  1883;
//const char *mqtt_user = "emqx";
//const char *mqtt_password = "public";
//const char *mqtt_server = "broker.emqx.io";  // Free Public MQTT broker 
//const int mqtt_port = 1883;  //There is no privacy protection for public access broker.
//                              //Any device can publish and subscribe to topics on it.
const char* clientID = "zhsnpi1fdevices";

const int SDA_PIN = 14;         //ESP-01S Board SDA = 0; SCL = 2;   ESP8266 HEKR 1.1 Board  SDA = 0; SCL = 13
const int SCL_PIN = 0;
const uint8_t kLedPin = 4;
const uint8_t kButtonPin = 13;       
//const int SDA_PIN = 4;         
//const int SCL_PIN = 5;       // ESP-12F Board SDA = 4; SCl = 5;
//const uint8_t kLedPin = 12;
const char hex_table[]{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

char mqtt_topic[50] = Base_Topic;
char msg[MSG_BUFFER_SIZE];
char scpicmd[MSG_BUFFER_SIZE];
char ui_buffer[UI_BUFFER_SIZE];
unsigned long previousMillis = 0;
long count = 0;
uint16_t value = 0;
float setcurr;

int debounce = 20; // ms debounce period to prevent flickering when pressing or releasing the button
int DCgap = 200; // max ms between clicks for a double click event
int holdTime = 2000; // ms hold period: how long to wait for press+hold event
int longHoldTime = 5000; // ms long hold period: how long to wait for press+hold event
             // Other button variables
boolean buttonVal = HIGH; // value read from button
boolean buttonLast = HIGH; // buffered value of the button's previous state
boolean DCwaiting = false; // whether we're waiting for a double click (down)
boolean DConUp = false; // whether to register a double click on next release, or whether to wait and click
boolean singleOK = true; // whether it's OK to do a single click
long downTime = -1; // time the button was pressed down
long upTime = -1; // time the button was released
boolean ignoreUp = false; // whether to ignore the button release because the click+hold was triggered
boolean waitForUp = false; // when held, whether to wait for the up event
boolean holdEventPast = false; // whether or not the hold event happened already
boolean longHoldEventPast = false;// whether or not the long hold event happened already

WiFiClient eClient;
PubSubClient client(mqtt_server, mqtt_port, eClient);

void setup() { 
    pinMode(kButtonPin, INPUT_PULLUP);
    pinMode(kLedPin, OUTPUT);
    defaultint();
    Serial1.begin(38400); 
    Serial.begin(9600);
    Log.begin(LOG_LEVEL, &Serial1, false);  //
    Wire.begin(SDA_PIN, SCL_PIN);
//  Wire.setClock(50000);    // Set the I2C clock(50kHz), default(100kHz);    
    digitalWrite(kLedPin, LOW);
    setWifiMqtt();  
    pmbus_devices_init();  
    if(serialflag)printhelp();
 }

void loop() {     
   mqttLoop();
   checkButton();
   checkSerial();
   checkSensors();
}
