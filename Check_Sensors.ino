void checkSensors(){
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= pmInterval && !subsmbusflag && !subscpiflag){
        previousMillis = currentMillis;
        if(scani2c) i2cdetects(0x03, 0x7F);
        if(readpmbusdata()){
            if(0 != pd.statusWord && statusflag && serialflag) pmbusStatus();                 
            if(wifistatus && mqttflag) publishPmbusData(pd);
            if(wifistatus && mqttflag && statusflag) pubPmbusStatus(); 
            if(0 == count%3 && serialflag) printpmbusData(pd);        
          }
          count++;
          buttonflag = true;                  
     } 
}

bool readpmbusdata(){   
      bool ret = true;
      if(!pmbusflag) return ret = false;  
      if(smbus_waitForAck(ps_i2c_address, 0x00) == 0) {  //0x00 PAGE read         
          if(wifistatus && mqttflag){
            if(count%6 == 0){
              ++value;     
              snprintf (msg, MSG_BUFFER_SIZE, "PMBUS Polling Fail Loop#%ld", value);
              pub("pmbus/status", msg);
            }
         }
          if(serialflag)Log.noticeln(F("PMBUS Polling Fail loop: %l, Type 'h' To Help"), count);
//          else          Log.noticeln(F("PMBUS Polling Fail loop: %l,"), count);
          delay(10);      
          return ret = false;
       }
     pd.i2cAddr = ps_i2c_address;
     pd.inputV = pmbus_readVin(ps_i2c_address);
     pd.inputA = pmbus_readIin(ps_i2c_address);
     pd.outputV = pmbus_readVout(ps_i2c_address);
     pd.outputA = pmbus_readIout(ps_i2c_address);
     pd.inputP = pmbus_readPin(ps_i2c_address);
     pd.outputP = pmbus_readPout(ps_i2c_address);
     pd.temp1 = pmbus_readOtemp(ps_i2c_address);        //temp sensor 0x8D                
     pd.statusWord = pmbus_readStatusWord(ps_i2c_address);
     
     if(expandengery){
     pd.inputE = pmbus_readEin(ps_i2c_address);            //Ein 0x86
     pd.outputE = pmbus_readEout(ps_i2c_address);         //Eout 0x87
     }
     if(expandsensor){
     pd.temp2 = pmbus_readItemp(ps_i2c_address);        //temp sensor 0x8E  
     pd.temp3 = pmbus_readMtemp(ps_i2c_address);        //temp sensor 0x8F  
     pd.fanSpeed = pmbus_readFanSpeed1(ps_i2c_address);
     }    
     if(standbyflag){
     pmbus_setPage(ps_i2c_address,1);                    //set Page to 1, read 12Vsb 
     pd.outputVsb = pmbus_readVout(ps_i2c_address);
     pd.outputAsb = pmbus_readIout(ps_i2c_address);
     pmbus_setPage(ps_i2c_address,0);
     }     
     delay(10);
     return ret;     
}

void publishPmbusData(struct PowerPmbus busData){
  ledflash();   
 if (count%6 == 0) {
      ++value;     
      snprintf (msg, MSG_BUFFER_SIZE, "PMBUS_Addr: 0x%02x Refresh#%ld", busData.i2cAddr, value);
      pub("pmbus/status", msg);
      Log.noticeln("PMBUS_PUBLISH_REFRESH:%d", value);
    }
  snprintf (msg, MSG_BUFFER_SIZE, "%3.2f", busData.inputV);
//  client.publish("rrh/pmbus/input/volt", msg);
  pub("pmbus/input/volt", msg);
  snprintf (msg, MSG_BUFFER_SIZE, "%4.3f", busData.inputA);
  pub("pmbus/input/curr", msg);
  snprintf (msg, MSG_BUFFER_SIZE, "%3.2f", busData.inputP);
  pub("pmbus/input/power", msg);
  
  snprintf (msg, MSG_BUFFER_SIZE, "%5.4f", busData.outputV);
  pub("pmbus/output/volt", msg);
  snprintf (msg, MSG_BUFFER_SIZE, "%4.3f", busData.outputA);
  pub("pmbus/output/curr", msg);
  snprintf (msg, MSG_BUFFER_SIZE, "%3.2f", busData.outputP);
  pub("pmbus/output/power", msg);
  
  snprintf (msg, MSG_BUFFER_SIZE, "%2.1f", busData.temp1);
  pub("pmbus/sensor/temp1", msg);
  snprintf (msg, MSG_BUFFER_SIZE, "0x%02x%02x", busData.statusWord >> 8, busData.statusWord & 0xFF);
  pub("pmbus/status/word", msg);
  
  if(expandengery){
    snprintf (msg, MSG_BUFFER_SIZE, "%3.2f", busData.inputE);
    pub("pmbus/input/energy", msg);
    snprintf (msg, MSG_BUFFER_SIZE, "%3.2f", busData.outputE);
    pub("pmbus/output/energy", msg);
  }

  if(expandsensor){
    snprintf (msg, MSG_BUFFER_SIZE, "%2.1f", busData.temp2);
    pub("pmbus/sensor/temp2", msg);
    snprintf (msg, MSG_BUFFER_SIZE, "%2.1f", busData.temp3);
    pub("pmbus/sensor/temp3", msg);
    snprintf (msg, MSG_BUFFER_SIZE, "%2.1f", busData.fanSpeed);
    pub("pmbus/sensor/fan", msg);
  }
  
  if(standbyflag){
    snprintf (msg, MSG_BUFFER_SIZE, "%5.4f", busData.outputVsb);
    pub("pmbus/output/vsb", msg);
    snprintf (msg, MSG_BUFFER_SIZE, "%4.3f", busData.outputAsb);
    pub("pmbus/output/csb", msg);
  } 
}

void pubPmbusStatus() {
       uint8_t io,in,tm,fa,vo,cm;
       tm = pmbus_readStatusTemp(ps_i2c_address);    //0x7D
       fa = pmbus_readStatusFan(ps_i2c_address);     //0x81
       cm = pmbus_readStatusCml(ps_i2c_address);     //0x7E
       vo = pmbus_readStatusVout(ps_i2c_address);    //0x7A Status_Vout
       io = pmbus_readStatusIout(ps_i2c_address);    //0x7B 
       in = pmbus_readStatusInput(ps_i2c_address);   //0x7C
       snprintf (msg, MSG_BUFFER_SIZE, "Cml:0x%02x Temp:0x%02x Fan:0x%02x Vout:0x%02x Iout:0x%02x Input:0x%02x", cm, tm, fa, vo, io, in);
       pub("pmbus/status/all", msg);
}

void printpmbusData(struct PowerPmbus busData){
    char volt[10];
    char curr[9];
    char others[8];
    Log.noticeln(F("========== PMBUS DATA =========="));
    Log.noticeln(F("PMBUS ADDRESS: 0x%x"), busData.i2cAddr);    //F function enable to decrease sram usage
    Log.noticeln(" ");
    Log.noticeln(F("INPUT: "));
    sprintf(curr, "%4.3f", busData.inputA);
    Log.noticeln(F("Volt:%F,  Curr:%s,  Pow:%F "),busData.inputV, curr, busData.inputP);
   
    Log.noticeln(F("OUTPUT:"));
    sprintf(volt, "%5.4f", busData.outputV);
    sprintf(curr, "%4.3f", busData.outputA);
    Log.noticeln(F("Volt:%s,  Curr:%s,  Pow:%F "),volt, curr, busData.outputP);     
    Log.noticeln(F(" "));
    
    if(expandengery) {
    Log.noticeln(F("INPUT_Eny:%F,  OUTPUT_Eny:%F"),busData.inputE, busData.outputE );
    Log.noticeln(F(" "));      
    }       
    if(standbyflag){
    sprintf(volt, "%5.4f", busData.outputVsb);
    sprintf(curr, "%4.3f", busData.outputAsb);  
    Log.noticeln(F("STANDBY  Volt:%s,   Curr:%s"), volt, curr);
    Log.noticeln(F(" "));
    }
    if(expandsensor) {
      sprintf(others, "%1.0f", busData.fanSpeed);
      Log.noticeln(F("TP_8D:%F, TP_8E:%F, TP_8F:%F, FS:%s"), busData.temp1, busData.temp2, busData.temp3, others);
      Log.noticeln(F(" "));
    }    
    Log.noticeln(F("STATUS WORD: %X,  %B"), busData.statusWord, busData.statusWord);
    ledflash();
}

void pmbusStatus(){
    uint16_t w_val;
    uint8_t msb,lsb,io,in,tm,fa,vo,cm;
    Log.noticeln(F("========= READ ALL STATUS =========="));
    Log.noticeln(F(" "));
    w_val = pd.statusWord;
    Log.noticeln(F("STATUS WORD:%X,  %B"), w_val, w_val);
//    Serial.printf("%04x\n", w_val);
    msb = w_val >> 8;
    lsb = w_val & 0xFF;
    Log.noticeln("HIGH:0x%x,  %B", msb, msb);
    Log.noticeln("LOW: 0x%x,  %B", lsb, lsb);
   
    if(msb & 0x80){
        vo = pmbus_readStatusVout(ps_i2c_address);
        Log.noticeln(F("STATUS_VOUT: %B,  0x%x"), vo, vo);
        if(vo & 0x80)
        Log.noticeln(F("STATUS_VOUT_OV_FAULT !! "));
        if(vo & 0x10)
        Log.noticeln(F("STATUS_VOUT_UV_FAULT !! "));     
    }

    if(msb & 0x40){
        io = pmbus_readStatusIout(ps_i2c_address);
        Log.noticeln(F("STATUS_IOUT: %B,  0x%x"), io, io);
        if(io & 0x80)
        Log.noticeln(F("STATUS_IOUT_OC_FAULT !! "));
        if(io & 0x20)
        Log.noticeln(F("STATUS_IOUT_OC_WARNING !! "));
        if(io & 0x02)
        Log.noticeln(F("STATUS_POUT_OP_FAULT !! "));
        if(io & 0x01)
        Log.noticeln(F("STATUS_POUT_OP_WARNING !! ")); 
    }

    if(msb & 0x20){
        in = pmbus_readStatusInput(ps_i2c_address);
        Log.noticeln(F("STATUS_INPUT: %B,  0x%x"), in, in);
        if(in & 0x20)
        Log.noticeln(F("STATUS_VIN_UV_WARNING !! "));
        if(in & 0x10)
        Log.noticeln(F("STATUS_VIN_UV_FAULT !! "));
        if(in & 0x08)
        Log.noticeln(F("STATUS_UNIT_OFF_FOR_INSUFFICIENT_INPUT !! "));
        if(in & 0x02)
        Log.noticeln(F("STATUS_IIN_OVER_CURRENT_WARNING !! ")); 
        if(in & 0x01)
        Log.noticeln(F("STATUS_PIN_OVER_POWER_WARNING !! ")); 
    }

    if(msb & 0x08)  Log.noticeln(F("STATUS_POWER_GOOD#_FAULT !! "));

    if(msb & 0x04){
      fa = pmbus_readStatusFan(ps_i2c_address);
      Log.noticeln(F("STATUS_FAN: %B,  0x%x"), fa, fa);
        if(fa & 0x80)
        Log.noticeln(F("STATUS_FAN_1_FAULT !! "));
        if(fa & 0x40)
        Log.noticeln(F("STATUS_FAN_2_FAULT !! "));
        if(fa & 0x20)
        Log.noticeln(F("STATUS_FAN_1_WARNING !! "));
        if(fa & 0x10)
        Log.noticeln(F("STATUS_FAN_1_WARNING !! "));
    }

    if(msb & 0x02)  Log.noticeln(F("STATUS_OTHERS_WARNING !! "));   
    if(lsb & 0x40)  Log.noticeln(F("STATUS_PS_OFF !! "));
    if(lsb & 0x20)  Log.noticeln(F("STATUS_MAIN_OUTPUT_OV_FAULT !! "));

    if(lsb & 0x10){
        io = pmbus_readStatusIout(ps_i2c_address);
        Log.noticeln(F("STATUS_IOUT: %B,  0x%x"), io, io);
        if(io & 0x80)
        Log.noticeln(F("STATUS_IOUT_OC_FAULT !! "));
        if(io & 0x40)
        Log.noticeln(F("STATUS_IOUT_OC_LV_FAULT !! "));  //When low voltage due to OC is detected.
        if(io & 0x20)
        Log.noticeln(F("STATUS_IOUT_OC_WARNING !! "));
        if(io & 0x08)
        Log.noticeln(F("STATUS_ISHARE_FAULT !! "));
        if(io & 0x02)
        Log.noticeln(F("STATUS_POUT_OP_FAULT !! "));
        if(io & 0x01)
        Log.noticeln(F("STATUS_POUT_OP_WARNING !! ")); 
    }
    
    if(lsb & 0x08){
        in = pmbus_readStatusInput(ps_i2c_address);
        Log.noticeln(F("STATUS_INPUT: %B,  0x%x"), in, in);
        if(in & 0x80)
        Log.noticeln(F("STATUS_VIN_OV_FAULT !! "));
        if(in & 0x40)
        Log.noticeln(F("STATUS_VIN_OV_WARNING !! "));
        if(in & 0x20)
        Log.noticeln(F("STATUS_VIN_UV_WARNING !! "));
        if(in & 0x10)
        Log.noticeln(F("STATUS_VIN_UV_FAULT !! "));
        if(in & 0x08)
        Log.noticeln(F("STATUS_UNIT_OFF_FOR_INSUFFICIENT_INPUT !! "));
        if(in & 0x02)
        Log.noticeln(F("STATUS_IIN_OVER_CURRENT_WARNING !! ")); 
        if(in & 0x01)
        Log.noticeln(F("STATUS_PIN_OVER_POWER_WARNING !! ")); 
    }

    if(lsb & 0x04){
        tm = pmbus_readStatusTemp(ps_i2c_address);
        Log.noticeln(F("STATUS_TEMPERATURE: %B,  0x%x"), tm, tm);
        if(tm & 0x80)
        Log.noticeln(F("STATUS_OT_FAULT !! "));
        if(tm & 0x40)
        Log.noticeln(F("STATUS_OT_WARNING !! "));
    }
    
    if(lsb & 0x02){
      cm = pmbus_readStatusCml(ps_i2c_address);
      Log.noticeln(F("STATUS_CML: %B,  0x%x"), cm, cm);
//      Serial.print(F("STATUS_CML 0B "));
//      printBits(cm);
//      Serial.print(F("    : 0x"));
//      Serial.printf("%02x\n", cm);
      if(cm & 0x80)
      Log.noticeln(F("STATUS_CML_InvalidCMD !! "));
      if(cm & 0x40)
      Log.noticeln(F("STATUS_CML_InvalidData !! "));
      if(cm & 0x20)
      Log.noticeln(F("STATUS_CML_PEC_Fault !! "));
      if(cm & 0x08) 
      Log.noticeln(F("STATUS_CML_Processor_Fault !! "));
      if(cm & 0x02) 
      Log.noticeln(F("STATUS_CML_COMM_Fault !! "));
      if(cm & 0x01) 
      Log.noticeln(F("STATUS_CML_MEM_Logic_Fault !! "));
    }
    Log.noticeln(F(" "));
//    if(wifistatus && mqttflag){
//       tm = pmbus_readStatusTemp(ps_i2c_address);
//       fa = pmbus_readStatusFan(ps_i2c_address);
//       cm = pmbus_readStatusCml(ps_i2c_address);
//       vo = pmbus_readStatusVout(ps_i2c_address);
//       io = pmbus_readStatusIout(ps_i2c_address);
//       in = pmbus_readStatusInput(ps_i2c_address);
//       snprintf (msg, MSG_BUFFER_SIZE, "Cml:0x%02x Temp:0x%02x Fan:0x%02x Vout:0x%02x Iout:0x%02x Input:0x%02x", cm, tm, fa, vo, io, in);
//       pub("pmbus/status/all", msg);
//    }
}

void printFru(uint8_t first, uint8_t last, uint8_t *values) {
      char c[6];
      Log.notice("    ");
      for (int i = 0; i < 16; i++) {
              sprintf(c, "%3x",  i);
              Log.notice("%s", c);
           }
      for (int address = 0; address <= 255; address++) {   
            if (address % 16 == 0) {
                  sprintf(c, "0x%02x:", address & 0xF0);
                  Log.notice(CR "%s", c);
                }              
            if (address >= first && address <= last) {
                  sprintf(c, " %02x", values[address]);
                  Log.notice("%s", c);
              }
            else Log.notice("   ");
         }
     Log.noticeln("");
}
