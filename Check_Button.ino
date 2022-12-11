void checkButton() {
  if(buttonflag) { 
      key = 0;         
      buttonVal = digitalRead(kButtonPin);
                // Button pressed down
     if (buttonVal == LOW && buttonLast == HIGH && (millis() - upTime) > debounce) {
        downTime = millis();
        ignoreUp = false;
        waitForUp = false;
        singleOK = true;
        holdEventPast = false;
        longHoldEventPast = false;
    if ((millis() - upTime) < DCgap && DConUp == false && DCwaiting == true) DConUp = true;
    else DConUp = false;
    DCwaiting = false;
  }
  // Button released
  else if (buttonVal == HIGH && buttonLast == LOW && (millis() - downTime) > debounce) {
    if (not ignoreUp) {
      upTime = millis();
      if (DConUp == false) DCwaiting = true;
      else {
        key = 2;
        DConUp = false;
        DCwaiting = false;
        singleOK = false;
      }
    }
  }
  // Test for normal click event: DCgap expired
  if (buttonVal == HIGH && (millis() - upTime) >= DCgap && DCwaiting == true && DConUp == false && singleOK == true) {
    key = 1;
    ledflash();
    DCwaiting = false;
  }
  // Test for hold
  if (buttonVal == LOW && (millis() - downTime) >= holdTime) {
    // Trigger "normal" hold
    if (not holdEventPast) {
      key = 3;
      waitForUp = true;
      ignoreUp = true;
      DConUp = false;
      DCwaiting = false;
      //downTime = millis();
      holdEventPast = true;
    }
    // Trigger "long" hold
    if ((millis() - downTime) >= longHoldTime) {
      if (not longHoldEventPast) {
        key = 4;
        longHoldEventPast = true;
      }
    }
  }
  buttonLast = buttonVal;
 }
 
  if(subsmbusflag){                     //sent smbus command by subscribe("rrh/pmbus/set")
      if (smbus_data[0] >= 0 && smbus_data[0] <=9) smbus_command_sent(smbus_data[0]);   // send smbus command
      else if(smbus_data[0] == 0xAA){                                                   // set pmbus 
         if     (smbus_data[1] == 0) ps_i2c_address = smbus_data[2];           //[AA 00 XX] Modify the Pmbus device address
         else if(smbus_data[1] == 1) pmInterval = (smbus_data[2]<<8)  + smbus_data[3];  //[AA 01 XX XX] Set pmbus poll time /ms;
         else if(smbus_data[1] == 2) pmbusflagset(smbus_data[2]);         //[AA 02 00] Disable PMbus.
         else if(smbus_data[1] == 3) monitorstatus();
         else if(smbus_data[1] == 4) expandsensor = true;
         else if(smbus_data[1] == 5) i2cdetectsstatus();            //[AA 05] Scan Pmbus device.
         else if(smbus_data[1] == 6) standbystatus();               //[AA 06] Standby monitoring Enable/Disble.
         else if(smbus_data[1] == 7) expandengery = true;            //
         else if(smbus_data[1] == 8) expandengery = false;         
         else if(smbus_data[1] == 9) pecstatus();                   //[AA 09] PEC Enable/Disable.
         else if(smbus_data[1] == 0xAA) key = 4;                     //set default
         else if(smbus_data[1] ==0xBB) esprestar();                 //reset device
       }
     else if(smbus_data[0] == 0xCC){                               //send SCPI script command  
        if(smbus_data[1] == 0) currlh = true;
        else if(smbus_data[1] == 1) currlh = false;
        else if(smbus_data[1] == 2)setdynload(); 
     }
       subsmbusflag = false;
       buttonflag = true;
  }
  if(subscpiflag) {
    sendscpiread(scpicmd);
    subscpiflag = false;
    buttonflag = true;
  }
  if(setscpicurr) { 
    modifycurr(setcurr);
    setscpicurr = false;
    buttonflag = true;
  }
}

void setWifiMqtt(){
  int k = 0;
  delay(10);
  Log.notice(CR );
  Log.noticeln("Connecting to %s", ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    k++;
    ledflash();
    Log.notice(".");
    if (digitalRead(kButtonPin) == 0){
          delay(10);
        if(digitalRead(kButtonPin) == 0){
              buttonflag = false;                           
              serialflag = true;
              wifistatus = false;
              Log.begin(LOG_LEVEL, &Serial, false);
              Log.noticeln (F("Skip Wifi, Only Serial mode"));
              delay(100);
              break;
          }
       }   
    if( k >= 30){
        wifistatus = false;
        serialflag = true;
        Log.begin(LOG_LEVEL, &Serial, false);
        break;
     }
  }
  
  if(WiFi.status() == 3) {
    wifistatus = true;
    Log.noticeln("WiFi Connected.");
    Log.noticeln("IP address: %s", WiFi.localIP().toString().c_str());
  }
  else {
    wifistatus =false;
    Log.noticeln("wifi Connected Failed");
  }
  delay(50);
 if(wifistatus){
      Log.begin(LOG_LEVEL, &Serial1, false); //
      randomSeed(micros());
      Log.notice(CR );
      client.setCallback(callback);
      String client_id;
      client_id = clientID + String(WiFi.macAddress());
      Log.noticeln("Client_id = %s", client_id.c_str());
      delay(100);
      if(client.connect(client_id.c_str(), mqtt_user, mqtt_password)) {
          mqttflag = true;
          Log.noticeln("MQTT Broker Connected.");
//          client.subscribe("rrh/pmbus/set");
          sub("pmbus/set/#");
          sub("scpi/set/#");
//          sub("pmbus/set/curr");
        } 
      else{
          mqttflag = false;        
          Log.noticeln("MQTT Broker Connected Failed");
      }
    Log.noticeln("");
    delay(100); 
  }
}

void subMQTT(const char* topic) {
   if(client.subscribe(topic)){
     Log.traceln(F("Subscription OK to the subjects %s"), topic);;
  } else {
    Log.traceln(F("Subscription Failed, rc=%d"), client.state());
  }
}

void sub(const char* topicori) {
  String topic = String(mqtt_topic) + String(topicori);
  subMQTT(topic);
}

void subMQTT(String topic) {
  subMQTT(topic.c_str());
}

void pubMQTT(const char* topic, const char* payload, bool retainFlag) {
  if (client.connected()) {
    if(serialflag) Log.traceln(F("[MQTT_publish] topic: %s msg: %s "), topic, payload);
    client.publish(topic, payload, retainFlag);
  } else {
    Log.traceln(F("Client not connected, aborting thes publication"));
  }
}

void pubMQTT(const char* topic, const char* payload) {
  pubMQTT(topic, payload, false);
}

void pubMQTT(String topic, const char* payload) {
  pubMQTT(topic.c_str(), payload);
}

void pub(const char* topicori, const char* payload) {
  String topic = String(mqtt_topic) + String(topicori);
  pubMQTT(topic, payload);
}

void callback(char* topic, byte* payload, unsigned int length) {
    String inPayload = "";
    String currtopic = String(mqtt_topic) + "scpi/set/curr";
//    byte* p = (byte*)malloc(length + 1);
//    memcpy(p, payload, length);
//    p[length] = '\0';
    Log.noticeln(F("Message arrived [ %s ]" ), topic);
   if(currtopic.compareTo(topic) == 0) {
      for (int i = 0; i < length; i++) {
           inPayload += (char)payload[i];
      }
      setcurr =  ((float) inPayload.toInt())*0.1; 
      setscpicurr = true;
      buttonflag = false;
      inPayload = "";
   }
  else if ((char)payload[0] == '[') {
      for(int i = 0; i < length; i++){   
          smbus_data[i] = tohex(payload[3*i+1])*16 + tohex(payload[3*i+2]); 
          if (payload[3*i+3] == ']'){
              subsmbusflag = true;
              buttonflag = false;
              break;
            }
          if (i >= 36) {
              Log.noticeln(F("Smbus Invalid format"));
              pub("pmbus/info", "Smbus Invalid format");
              subsmbusflag = false;
              delay(100);
              break; 
            }      
        }
  }
  else if ((char)payload[0] == '%') {              
      for (int i = 0; i < length; i++) {
          scpicmd[i] = payload[i + 1];
          if( '%' == payload[i + 1]){
            scpicmd[i] = '\0';
            subscpiflag = true;
            buttonflag = false;
            break;
          }
          if(i >= (length - 1)){
            Log.noticeln(F("Scpi Invalid format"));
            pub("scpi/info", "Scpi Invalid format");
            subscpiflag = false;
            delay(100);         
          }
       }
   }
  else if ((char)payload[0] == '0') key = 0;
  
  for (int i = 0; i < length; i++) {
    Log.notice("%c", (char)payload[i]);
  }
  Log.noticeln("");
}

void mqttLoop(){
  if(wifistatus){
    if (!client.connected()) {
            reconnect();
            sub("pmbus/set/#");
            sub("scpi/set/#");          
        }
    if (mqttflag){    
          client.loop();      
        }
    }
  else {
      if(!serialflag) setWifiMqtt();
    }
}

void reconnect() {
  // Loop until we're reconnected
  //client.connect(clientID, mqtt_user, mqtt_password);
  int k = 0;
  while (!client.connected()) {
     Log.notice("Attempting MQTT connection..."); // Attempt to connect   
//     String clientId = "ESP8266Client-";
//     clientId += String(random(0xffff), HEX);
      String client_id;
      client_id = clientID + String(WiFi.macAddress());
    if (client.connect(client_id.c_str(), mqtt_user, mqtt_password)) {
      Log.noticeln("connected to broker");
      pub("outTopic", "Broker reconnected");  // Once connected, publish an announcement...
      mqttflag = true;   
    }
    else {
      Log.notice("Failed, rc= %d", client.state());
//      Serial1.print(client.state());
      Log.noticeln(" try again in 2 seconds");
      k++;
      delay(2000);   // Wait 2 seconds before retrying
        if( k >= 5){
                mqttflag = false;
                wifistatus = false;
                Log.noticeln(F("WiFi connect Failed!!"));
                delay(100);
                break;
            }                   
        }
    }
}

void pmbus_devices_init(){
  ps_i2c_address = PS_I2C_ADDRESS;
  ps_patner_address = PS_PARTNER_ADDRESS;
  pecflag = PEC_DISABLE;
  i2cdetects(0x03, 0x7F);
  int i = 3;
  int n = 0;
  bool scanpsu = true;    
  while(scanpsu){
    ledflash();
    n = pmbusdetects();
    delay(100);
    i--;
    if(n > 1)  scanpsu = false;
    if(i <= 0) scanpsu = false;
    }
    if(n == 0) {
          ps_i2c_address = PS_I2C_ADDRESS;
          ps_patner_address = PS_PARTNER_ADDRESS;
      }
    Log.noticeln("PMBUSADDRESS 0x%x:", ps_i2c_address);
    n = 0;
    scanpsu = false;
    eeprom_address = 0x50 + (ps_i2c_address & 0x07);
    delay(100);
 }

int pmbusdetects() {
  uint8_t n = 0, address, rerror;
  char c[6];
  Log.notice("   ");                   // table header
  for (int i = 8; i < 16; i++) {
    sprintf(c, "%3x",  i);
    Log.notice("%s", c);
  }
  Log.notice(CR "%x:", 0x50);
  for (address = 88; address <= 95; address++){
        Wire.beginTransmission(address);
        rerror = Wire.endTransmission();
        if (rerror == 0) {                  // device found        
          n++;
          if     (n == 1) ps_i2c_address = address;
          if(n == 2) ps_patner_address = address;
          sprintf(c, " %02x", address);
          Log.notice("%s", c);       
        } 
        else if (rerror == 4) {              // other error        
            Log.notice(" XX");
        } 
        else {                             // error = 2: received NACK on transmit of address        
            Log.notice(" --");             // error = 3: received NACK on transmit of data        
        }
        delay(10);
    }
    Log.noticeln(CR"");
    Log.noticeln("%x",n);
    return n;
}

void i2cdetects(uint8_t first, uint8_t last) {
  uint8_t i, address, rerror;
  int q = 0;
  char c[6];
  char addr[35];
  Log.notice("   ");            // table header
  for (i = 0; i < 16; i++) {
    sprintf(c, "%3x",  i);
    Log.notice("%s", c);
  }
  for (address = 0; address <= 127; address++) {             // addresses 0x00 through 0x77
    if (address % 16 == 0) {                            // table body
          sprintf(c, "0x%02x:", address & 0xF0);
          Log.notice(CR "%s", c);
      }
    if (address >= first && address <= last) {
        Wire.beginTransmission(address);
        rerror = Wire.endTransmission();
        delay(10);
        if (rerror == 0) {                           // device found
          sprintf(c, " %02x", address);
          Log.notice("%s", c);
          addr[3*q] = hex_table[address >> 4];
          addr[3*q + 1] = hex_table[address & 0x0f];
          addr[3*q + 2] = ' ';
          q++;
      } else if (rerror == 4) {    // other error      
        Log.notice(" XX");
      } else {                   // error = 2: received NACK on transmit of address              
        Log.notice(" --");    // error = 3: received NACK on transmit of data
      }
    } else {                 // address not scanned      
      Log.notice("   ");
    }
    if(q > 10) break;
  }
  addr[3*q] = '\0';
  Log.noticeln("" CR);
  snprintf (msg, MSG_BUFFER_SIZE, "Scan addr at:0x%s", addr);
  pub("pmbus/info", msg); 
}

void pmbusflagset(uint8_t val){
  if(val == 0) {
    pmbusflag = false;
    Log.noticeln(F("pmbusflag Disable"));
    pub("pmbus/enable", "0");
  }
  else {
    pmbusflag = true;
    Log.noticeln(F("pmbusflag Enable"));
    pub("pmbus/enable", "1");
  }
  delay(100);
}

void pecstatus(){
    pecflag = !pecflag;
    if(pecflag) {
      Log.noticeln(F("PEC Enable"));
      pub("pmbus/pec", "1");
    }
    else {
      Log.noticeln(F("PEC Disable"));
      pub("pmbus/pec", "0");
    }
    delay(100);
}

void i2cdetectsstatus(){
  scani2c = !scani2c;
  if(scani2c) {
    Log.noticeln(F("I2C Detect Device Enable"));
    pub("pmbus/scan", "1");
    pmbusflag = false;
  }
  else {
    Log.noticeln(F("I2C Detect Device Disable"));
    pub("pmbus/scan", "0");
    pmbusflag = true;
  }
  delay(100);
}

void setIntervaltime() {
      Log.noticeln("Current Interval Time(ms):%d", pmInterval);
      Log.noticeln(F("Input New Interval Time(ms) < 60S :"));
      pmInterval = read_int();     
      Log.noticeln("New Interval Time(ms):%d", pmInterval);
      delay(100);
}

void reset_address(){
      Log.noticeln("Current PSU Address:0x%x; Patner PSU Address:0x%x.", ps_i2c_address, ps_patner_address);
      Log.noticeln(F("Input PSU address: (Can recognize Hex, Decimal, Octal, or Binary)"));
      Log.noticeln(F("Example: Hex: 0x11 (0x prefix) Octal: O21 (letter O prefix) Binary: B10001" ));
      ps_i2c_address = read_int();     
      Log.noticeln("New PSU address: 0x%x", ps_i2c_address);
      Log.noticeln(F("Input Patner PSU address:"));
      ps_patner_address = read_int();
      Log.noticeln("New Patner PSU address: 0x%x", ps_patner_address);
      delay(200);
}

void ledflash(){
  ledstatus = !ledstatus;
  if(ledstatus) digitalWrite(kLedPin, HIGH);
  else digitalWrite(kLedPin, LOW);
}

void monitorstatus(){
  statusflag = !statusflag;
  if(statusflag) {
    Log.noticeln(F("Status Monitor Enable"));
    pub("pmbus/monitor", "1");
  }
  else {
      Log.noticeln(F("Status Monitor Disable"));
      pub("pmbus/monitor", "0");
  }
  delay(100);
 }

void standbystatus(){
  standbyflag = !standbyflag;
  if(standbyflag) {
    Log.noticeln(F("Standby Enable"));
    pub("pmbus/standby", "1");
  }
  else {
    Log.noticeln(F("Standby Disable"));
    pub("pmbus/standby", "0");
  }
  delay(100);
 }

void esprestar(){
    Log.noticeln(F("delay 3S Reset "));
    for(int i = 0; i < 6; i++){
      delay(500);
      Log.notice(" .");
    }
    ESP.restart();
}

void printhelp(){
      Log.noticeln(F("************* WELCOME TO PMbus & SCPI Tools **************"));
      Log.notice(F("Here are commands can be used." CR));
      Log.notice(F(" 1 > Moitor PSU All Status On/Off" CR));
      Log.notice(F(" 2 > Add sensors Detect" CR));
      Log.notice(F(" 3 > Add engery Detect" CR));
      Log.notice(F(" 4 > Add Standby(Page 1)" CR));     
      Log.notice(F(" 5 > Scan PSU Address At PMbus" CR));      
      Log.notice(F(" 9 > Enable/Disable PEC" CR));
      Log.notice(F(" 0 > Set All to Default" CR));
      Log.notice(F(" e > Set the PMbus Interval Timing" CR));
      Log.notice(F(" c > Enter The SMbus Command Function & Read EEPROM" CR));
      Log.notice(F(" r > Modify the PMbus Device Address" CR));
      Log.notice(F(" z > Reset the ESP device" CR));
      Log.notice(F(" h > Display This Help" CR));
      Log.notice("" CR);
      Log.notice("Enter a command: " CR);      
      delay(100);  
}

void defaultint(){
            expandengery = false;
            expandsensor = false;
            standbyflag = false;
            pecflag = false;
            scani2c = false;
            subsmbusflag = false;
            subscpiflag = false;
            pmbusflag = true;
            statusflag = true;
            buttonflag = true;
            smbuscomun = true;
            Protocol = true;            
            pmInterval = 1000;
            key = 0;
            Log.noticeln("Set to Default");
}
