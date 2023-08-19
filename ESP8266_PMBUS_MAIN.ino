/*   ESP8266 model: ESP-01S Board SDA = 2; SCL = 0; ESP-12F Board SDA = 4; SCl = 0;
 *               HEKR 1.1 Board  SDA = 14; SCL = 0; LED = 4 Button = 13;
 *               Using the ESP8266 HEKR 1.1 Board (Purple);               
 *   https://play.google.com/store/apps/details?id=com.app.vetru.mqttdashboard  Mqtt Dashboard For  Android Iot APP
 *
 * Publish the topic "npi/pmbus/set" using the MQTT protocol with the message content "[-- XX XX XX XX]".
 *  The square brackets "[]" indicate the beginning and end of the HEX data string in the message. (There is a space " " between each data)
 *  The byte "--" in the beginning represents the type of command instruction being sent. 
 *  The value of "--" from 0x00 to 0x09 indicates the communication instructions of I2C/Smbus format and the I2C_EEPROM reading instructions. 
 *  The value of "--" as 0xAA represents the custom function.
 *  "XX" refers to the byte content of the command being sent.
 *
 *  For example, when publishing the topic "rrh/pmbus/set":
 *      1) The message content is "[03 58 00 01]" (Can be used to set the PAGA value in Pmbus to 0x01.)
 *              0x03 represents calling the smbus write byte format.
 *              0x58 represents the I2C address for smbus communication.
 *              0x00 represents the command of write byte.
 *              0x01 represents the parameter of the command.
 *
 *    2) The message content is "[AA 00 59]"
 *              0xAA represents calling the custom instruction.
 *              0x00 indicates resetting the address of Pmbus.
 *              0x59 represents the value of the address.
 *  
 * Publish the topic "npi/scpi/set" using the MQTT protocol with the message content "%*IDN?%".
 *  "%" , "%" are the starting and ending marks for sending SCPI string format messages. Used for communication with SCPI serial measuring instruments.
 *   Author Dafeng 2021
*/
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ArduinoLog.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

#define DEVICE_ID_Topic "npi/"
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
  uint8_t s_tm;
  uint8_t s_fa;
  uint8_t s_cm;
  uint8_t s_vo;
  uint8_t s_io;
  uint8_t s_in;
  uint8_t i2cAddr;  
}pd;

struct eeprom
{
  uint8_t host;
  char ssid[32];
  char password[16];
  char mqtt_broker[64];
}eep;

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
const char* mqtt_user = "dfiot";      
const char* mqtt_password = "123abc";
const char* mqtt_server = "192.168.200.2"; //Raspberry Local MQTT Broker
const uint16_t mqtt_port =  1883;
const char* clientID = "zhsnpi1fdevices001";
//const char *mqtt_user = "emqx";    // Free Public MQTT broker 
//const char *mqtt_password = "public";    //There is no privacy protection for public access broker.
//const char *mqtt_server = "broker.emqx.io";  
//const int mqtt_port = 1883;    //Any device can publish and subscribe to topics on it.
                              
const int SDA_PIN = 14;         //ESP-01S Board SDA = 2; SCL = 0;   
const int SCL_PIN = 0;         //ESP8266 HEKR 1.1 Board  SDA = 14; SCL = 0
const uint8_t kLedPin = 4;     // ESP-12F Board SDA = 4; SCl = 0;
const uint8_t kButtonPin = 13; // ESP-12F kLedPin = 12      
const char hex_table[]{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

char mqtt_topic[50] = DEVICE_ID_Topic;
char msg[MSG_BUFFER_SIZE];
char scpicmd[MSG_BUFFER_SIZE];
char ui_buffer[UI_BUFFER_SIZE];
unsigned long previousMillis = 0;
long count = 0;
uint16_t value = 0;
float setcurr;

WiFiClient eClient;
PubSubClient client(mqtt_server, mqtt_port, eClient);

void setup() { 
    pinMode(kButtonPin, INPUT_PULLUP);
    pinMode(kLedPin, OUTPUT);
    defaultint();
    Serial1.begin(38400); 
    Serial.begin(9600);
    EEPROM.begin(512);
    Log.begin(LOG_LEVEL, &Serial, false);  
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
