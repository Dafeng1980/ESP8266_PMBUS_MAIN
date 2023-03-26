void checkSerial(){
    char readval;
    if (Serial.available() && serialflag) {                //! Serial input read
      readval = read_char();                             //! Reads the user command
      if ((char)readval == '0')      key = 4;
      else if ((char)readval == '1') key = 1;
      else if ((char)readval == '2') expandsensor = true;
      else if ((char)readval == '3') expandengery = true;
      else if ((char)readval == '4') standbystatus();
      else if ((char)readval == '5') i2cdetectsstatus();
      else if ((char)readval == '6') Log.noticeln("TBD");
      else if ((char)readval == '7') Log.noticeln("TBD");    
      else if ((char)readval == '8') Log.noticeln("TBD");
      else if ((char)readval == '9') pecstatus();
      else if ((char)readval == 'c'  || (char)readval == 'C') serial_smbus_commands();
      else if ((char)readval == 'e'  || (char)readval == 'E') setIntervaltime();   
      else if ((char)readval == 'r'  || (char)readval == 'R') reset_address();
      else if ((char)readval == 'z'  || (char)readval == 'Z') esprestar();
      else if ((char)readval == 'h'  || (char)readval == 'H') {
            printhelp();
            delay(1500);
            while (!digitalRead(kButtonPin)) delay(1);  // wait for button unpress                           
        }
      else {
          Log.notice("%x", readval);
          Log.noticeln("  unknown command");
          Log.noticeln("type \'h\' for help");     
        }
    buttonflag = true;
    ledflash();
    delay(10);
  }
   if(key == 1) monitorstatus();   //Button click
   if(key == 2) {                 //Button D click
      if(serialflag) serial_smbus_commands();
      else pecstatus();  
   }
   if(key == 3) i2cdetectsstatus(); //Button hold on
   if(key == 4) defaultint();       //Button Long hold on
}

uint8_t tohex(uint8_t val){
  uint8_t hex;
  if ( val - '0' >= 0 && val - '0' <=9){
     hex = val - '0';
  }
  else if( val == 'a' || val == 'A') hex = 10;
  else if( val == 'b' || val == 'B') hex = 11;
  else if( val == 'c' || val == 'C') hex = 12;
  else if( val == 'd' || val == 'D') hex = 13;
  else if( val == 'e' || val == 'E') hex = 14;
  else if( val == 'f' || val == 'F') hex = 15;
  else {
    Log.noticeln(F("Invalid Hex Data" ));
    dataflag = false;
    delay(10);
    return 0;
  }
  dataflag = true;
  return hex;
}

uint8_t smbus_sent(){
  uint8_t count = 0;
  char c[5];
  Log.noticeln(F(" "));
  read_data();
  if (ui_buffer[0] == 'm') return('m');
  if (ui_buffer[0] == 'h') return('h');
  if (ui_buffer[0] != '['){
    Log.noticeln(F("No frist start '[' "));
    delay(10);
    return 0xAF;
  }
//  smbus_data[0] = tohex(ui_buffer[1])*16 + tohex(ui_buffer[2]);
  for(int i = 0; i < 50; i++){
    count = i+1;
    smbus_data[i] = tohex(ui_buffer[3*i+1])*16 + tohex(ui_buffer[3*i+2]);
    if (!dataflag) return 0xAF;
    if (ui_buffer[3*i+3] == ']') {
      Log.notice(F("[" ));
      sprintf(c, "%02x", smbus_data[0]);
      Log.notice("%s", c); 
      for(int i = 1; i < count; i++){
      sprintf(c, " %02x", smbus_data[i]);
      Log.notice("%s", c);
      }
      Log.noticeln(F("]" ));
      break;
    }
    if (i >= 49) {
      Log.noticeln(F("No last end ']' in 32 Data"));
      delay(10);
      return 0xAF; 
    }
  } 
  if (smbus_data[0] >= 0 && smbus_data[0] <=9) return smbus_data[0];
  return 0xAF;
}

void serial_smbus_commands(){    
  uint8_t user_command;
  Log.noticeln(F("Input Smbus Commands: (Hex Data Format)"));
  Log.noticeln(F("Example Syntax: [03 58 00 01] MSB:03(Smbus Function:) Wirte Byte"));
  Log.noticeln(F("Smbus Function (0 to 7) Info:"));
  Log.noticeln(F("  0-Read Byte (RB)"));
  Log.noticeln(F("  1-Read Word (RW)"));
  Log.noticeln(F("  2-Read Block (RBL)"));
  Log.noticeln(F("  3-Write Byte (WB)"));
  Log.noticeln(F("  4-Write Word (WW)"));
  Log.noticeln(F("  5-Write Block (WBL)"));
  Log.noticeln(F("  6-Write Read Block (WRB)"));
  Log.noticeln(F("  7-Sent Byte (SBY)"));
  Log.noticeln(F("  8-EEPROM Read Bytes (ERS)"));
  Log.noticeln(F("  9-Generic I2C W/R Bytes (GI2C)"));
  Log.noticeln(F("[03 58 00 01]: 58 I2C_Address; 00 Smbus Command; 01 Data Byte;"));
  Log.noticeln(F("Send (03)writeByte: smbus_writeByte(0x58, 0x00, 0x01)."));
  subsmbusflag = false;
  delay(100);             
  do{  
      Log.noticeln(F("\n0-RB; 1-RW; 2-RBL; 3-WB; 4-WW; 5-WBL; 6-WRB"));
      Log.noticeln(F("7-SByte; 8-ERS; 9-GI2C; 'h'-Help; 'm'-Main Menu."));
      Log.noticeln(F("Enter Smbus Function: [xx xx xx xx xx]"));  
      user_command = smbus_sent();                              //! Reads the user command
      if (user_command == 'h' || user_command == 'm') { // Print m if it is entered
        Log.noticeln(F("%c"), user_command);
      }
      if (user_command != 0xAF) {
        smbus_command_sent(user_command);
      }
      else Log.noticeln(F("Smbus Invalid format" ));
//   else  Serial.println(user_command);                       // Print user command         
  }
  while (user_command != 'm');
}

void SyntaxHelp(){
      Log.noticeln(F("Command Syntax: "));
      Log.noticeln(F("0-Read Byte(RB)   [Smbus_Fn=00 I2C_Addr Smbus_CMD]"));
      Log.noticeln(F("1-Read Word(RW)   [Smbus_Fn=01 I2C_Addr Smbus_CMD Format]"));
      Log.noticeln(F("Parameters Foramt = 00, HEX-data return, 01, L11; 02, L16"));
      Log.noticeln(F("2-Read Block(RBL) [Smbus_Fn=02 I2C_Addr Smbus_CMD BlockSize]"));
      Log.noticeln(F("3-Write Byte(WB)  [Smbus_Fn=03 I2C_Addr Smbus_CMD DataByte]"));
      Log.noticeln(F("4-Write Word(WW)  [Smbus_Fn=04 I2C_Addr Smbus_CMD DataMSB DataLSB]"));
      Log.noticeln(F("5-Write Block(WBL)      [Smbus_Fn=05 I2C_Addr Smbus_CMD BlockSize DataMSB .. DataLSB]"));
      Log.noticeln(F("6-Write Read Block(WRB) [Smbus_Fn=06 I2C_Addr Smbus_CMD BlockSize DataMSB .. DataLSB ReadBlockSize]"));
      Log.noticeln(F("7-Sent Byte(SBY)        [Smbus_Fn=07 I2C_Addr Smbus_CMD]"));
      Log.noticeln(F("8-EEPROM Read Byte(ERS)   [Fn=08 I2C_Addr OffsetMSB OffsetLSB Qty Size]"));
      Log.noticeln(F("if eeprom data size < 0x100,Parameters size = 01, Instead = 00"));
      Log.noticeln(F("9-Generic I2C W/R Bytes) [Fn=09 I2C_Addr Qty Wbytes Qty Rbytes]"));    
      Log.noticeln(F("'h' Display This Help."));
      Log.noticeln(F("'m' Main Menu."));
}

void smbus_command_sent(uint8_t com){      
      uint16_t offset;
      uint8_t ps_i2c_address_;
      uint8_t actual_size;
      int count;
      char d[800];
//   const char hex_table[]{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
      struct smbusCommand
        {      
            uint8_t commands;
            uint8_t vmode;
            uint8_t databyte;
            uint8_t writebytes;
            uint8_t readbytes;
            uint16_t dataword;
            uint8_t datablock[256];
            uint16_t blocksize;
            uint8_t datablock_b[256];
            uint16_t blocksize_b;
            char msg[32];              
        }sm;
        smbuscomun = true;
  switch (com)
    {
      case 0:
        Log.noticeln(F("Smbus Read Byte:"));
        ps_i2c_address_ = smbus_data[1];
        sm.commands = smbus_data[2];
        sm.databyte = smbus_readByte(ps_i2c_address_, sm.commands);    
        if(smbuscomun) {
          snprintf (msg, MSG_BUFFER_SIZE, "%02X: [%02X]", sm.commands, sm.databyte);
          pub("pmbus/info/read", msg);
        }
       else  pub("pmbus/info/write", "Read Word Fail.");
//       if(mqttflag) f("rrh/pmbus/info/read", msg);       
        Log.noticeln("%s", msg);               
        delay(10);       
      break;
           
      case 1:
        Log.noticeln(F("Smbus Read Word:"));
        ps_i2c_address_ = smbus_data[1];
        sm.commands = smbus_data[2];
        sm.dataword = smbus_readWord(ps_i2c_address_, sm.commands);
        if (smbus_data[3] == 0 && smbuscomun) snprintf (msg, MSG_BUFFER_SIZE, "%02X: [%02X %02X]", sm.commands, sm.dataword >> 8, (uint8_t)sm.dataword);
        else if(smbus_data[3] == 1 && smbuscomun) snprintf (msg, MSG_BUFFER_SIZE, "%02X: [%4.3f]", sm.commands, L11_to_float(sm.dataword));
        else if(smbus_data[3] == 2 && smbuscomun) {
            sm.vmode = smbus_readByte(ps_i2c_address_, 0x20) & 0x1F;
            snprintf (msg, MSG_BUFFER_SIZE, "%02X: [%5.4f]", sm.commands, L16_to_float_mode(sm.vmode, sm.dataword));            
        }
        else pub("pmbus/info/write", "Read Word Fail.");         
        pub("pmbus/info/read", msg);
        Log.noticeln("%s", msg);     
        delay(10);
      break;
        
      case 2:
        Log.noticeln(F("Smbus Read Block:"));
        ps_i2c_address_ = smbus_data[1];
        sm.commands = smbus_data[2];
        sm.blocksize_b = smbus_data[3]; //Max size 32 block
        if(sm.blocksize_b > 255) {
             Log.errorln(F("Read Blocks: Fail size too big."));
             pub("pmbus/info/read", "Read Blocks: Fail size too big.");
             break; 
        }
        actual_size = smbus_readBlock(ps_i2c_address_, sm.commands, sm.datablock, sm.blocksize_b);
        if(actual_size > 255) {
             Log.errorln(F("Read Blocks: Fail Actual size too big."));
             pub("pmbus/info/read", "Read Blocks: Fail Actual size too big.");
             break; 
        }
        for (int n = 0; n < actual_size; n++){
          d[3*n] = ' ';
          d[3*n + 1] = hex_table[sm.datablock[n] >> 4];
          d[3*n + 2] = hex_table[sm.datablock[n] & 0x0f];
        }
        d[3*actual_size] = '\0';
        if(smbuscomun) {
          snprintf (msg, MSG_BUFFER_SIZE, "%02X: [%02X%s]", sm.commands, actual_size, d);
          pub("pmbus/info/read", msg);
        }
        else pub("pmbus/info/write", "Read Blocks Fail.");
              
        if(serialflag) {
                Log.noticeln("%s", msg); 
                // Log.errorln(F("Read Blocks Size:%x"), actual_size);
                // printFru(0, actual_size-1, sm.datablock);
        }                   
        delay(10);
      break;
            
      case 3:
        Log.noticeln(F("Smbus Write Byte:"));
        ps_i2c_address_ = smbus_data[1];
        sm.commands = smbus_data[2];
        sm.databyte = smbus_data[3];
        smbus_writeByte(ps_i2c_address_, sm.commands, sm.databyte);
        if(smbuscomun) snprintf (msg, MSG_BUFFER_SIZE, "WB Done.");
        else snprintf (msg, MSG_BUFFER_SIZE, "Write Byte Fail.");
        pub("pmbus/info/write", msg);
        Log.noticeln("%s", msg);      
        delay(10); 
        break;
                
      case 4:
        Log.noticeln(F("Smbus Write Word:"));
        ps_i2c_address_ = smbus_data[1];
        sm.commands = smbus_data[2];
        sm.dataword = smbus_data[3] << 8 | smbus_data[4];
        smbus_writeWord(ps_i2c_address_, sm.commands, sm.dataword);
        if(smbuscomun) snprintf (msg, MSG_BUFFER_SIZE, "WW Done.");
        else snprintf (msg, MSG_BUFFER_SIZE, "Write Word Fail.");
        pub("pmbus/info/write", msg);
        Log.noticeln("%s", msg);      
        delay(10);
        break;
                
      case 5:
        Log.noticeln(F("Smbus Write Block:"));
        ps_i2c_address_ = smbus_data[1];
        sm.commands = smbus_data[2];
        sm.blocksize = smbus_data[3];   // 
        if(sm.blocksize > 127) {
          Log.errorln(F("Write Blocks: Fail size too big."));
          pub("pmbus/info/write", "Write Blocks: Fail size too big.");
          break;
        } 
          for(int i = 0; i < sm.blocksize; i++) {
            sm.datablock[i] = smbus_data[4+i];;   
          }
          smbus_writeBlock(ps_i2c_address_, sm.commands, sm.datablock, sm.blocksize);
          if(smbuscomun) snprintf (msg, MSG_BUFFER_SIZE, "WB Done.");
          else snprintf (msg, MSG_BUFFER_SIZE, "Write Blocks Fail.");
          pub("pmbus/info/write", msg);
          Log.noticeln("%s", msg);      
          delay(10);     
        break;
      
       case 6:
        Log.noticeln(F("Smbus Write Read Blocks:"));
        ps_i2c_address_ = smbus_data[1];
        sm.commands = smbus_data[2];
        sm.blocksize = smbus_data[3];
        if(sm.blocksize > 64) {
         Log.errorln(F("Write Blocks: Fail size too big."));
         pub("pmbus/info/write", "Write Blocks: Fail size too big.");
         break;
        }
        for(int i = 0; i < sm.blocksize; i++) {
          sm.datablock[i] = smbus_data[i+4];
//          Serial.printf("Block n=%02X Data:%02X\n", i, sm.datablock[i]);    
        }
        sm.blocksize_b = smbus_data[sm.blocksize+4];
        if(sm.blocksize_b > 127) {
             Log.errorln(F("Read Blocks: Fail size too big."));
             pub("pmbus/info/write", "Read Blocks: Fail size too big.");
             break; 
        }
        actual_size = smbus_writeReadBlock (ps_i2c_address_, sm.commands, sm.datablock, sm.blocksize, sm.datablock_b, sm.blocksize_b);      
//        Serial.printf("%02X:", sm.blocksize_b);
        if(actual_size > 127) {
             Log.errorln(F("Read Blocks: Fail Actual size too big."));
             pub("pmbus/info/write", "Read Blocks: Fail Actual size too big.");
             break; 
        }              
        for (int n = 0; n < actual_size; n++){
          d[3*n] = ' ';
          d[3*n + 1] = hex_table[sm.datablock_b[n] >> 4];
          d[3*n + 2] = hex_table[sm.datablock_b[n] & 0x0f];
        }
        d[3*actual_size] = '\0';
        if(smbuscomun) {
          snprintf (msg, MSG_BUFFER_SIZE, "[%02X%s]",actual_size, d);
          pub("pmbus/info/read", msg);
        }
        else pub("pmbus/info/write", "Write Read Blocks Fail.");
        
        if(actual_size >16) {
                Log.errorln(F("Read Blocks Size:%x"), actual_size);
                printFru(0, actual_size-1, sm.datablock_b);
        }
        else Log.noticeln("%s", msg);      
        delay(10);     
        break;

       case 7:
        Log.noticeln(F("Smbus Sent Byte:"));
        ps_i2c_address_ = smbus_data[1];
        sm.commands = smbus_data[2];
        smbus_sendByte(ps_i2c_address_, sm.commands);
        Log.noticeln(F("SentByte Done."));
        if(smbuscomun) snprintf (msg, MSG_BUFFER_SIZE, "SentByte Done.");
        else snprintf (msg, MSG_BUFFER_SIZE, "SentByte Fail.");
        pub("pmbus/info/write", msg);      
        delay(10);       
          break;
         
      case 8:
//        Log.noticeln(F("EEPROM Read Bytes"));
        eeprom_address = smbus_data[1];
        offset = smbus_data[2] << 8 | smbus_data[3];
        count = smbus_data[4];
        if(smbus_data[5] == 1) eepromsize = false;
        else eepromsize = true;
        if(eepromreadbytes(eeprom_address, offset, count, eepbuffer))
            {
              Log.errorln(F("EEPROM Read Bytes: Fail."));
              pub("pmbus/info/eread", "EEPROM Read Bytes: Fail.");
              delay(1);
              break;
            }
        for (int n = 0; n < count; n++){
          
          d[3*n] = hex_table[eepbuffer[n] >> 4];
          d[3*n + 1] = hex_table[eepbuffer[n] & 0x0f];
          d[3*n + 2] = ' ';
        }
        d[3*count - 1] = '\0';      
        snprintf (msg, MSG_BUFFER_SIZE, "offset0x%04X:[%s]",offset, d);
        pub("pmbus/info/eread", msg);
        if(serialflag) {
            Log.noticeln(F("EEPROM Read Bytes"));
            Log.noticeln("offset%X:", offset);
            printFru(0, count-1, eepbuffer);
          }
        delay(1);
        break;
       
       case 9:
        Log.noticeln(F("Generic I2C W/R Bytes"));
        ps_i2c_address_ = smbus_data[1];
        sm.writebytes = smbus_data[2];
        if(sm.writebytes > 127) {
         Log.errorln(F("Write Blocks: Fail size too big."));
         pub("pmbus/info/write", "Write Blocks: Fail size too big.");
         break;
        }
        for(int i = 0; i < sm.writebytes; i++) {
          sm.datablock[i] = smbus_data[i+3];         
          d[3*i] = hex_table[sm.datablock[i] >> 4];
          d[3*i + 1] = hex_table[sm.datablock[i] & 0x0f];
          d[3*i + 2] = ' ';     
        }
        d[3*sm.writebytes - 1] = '\0'; 
        sm.readbytes = smbus_data[sm.writebytes+3];
        if(sm.readbytes == 0)
          snprintf (msg, MSG_BUFFER_SIZE, "Addr:0x%02x WriteQut:0x%02X [%s No Readback]", ps_i2c_address_, sm.writebytes, d, sm.readbytes);
        else snprintf (msg, MSG_BUFFER_SIZE, "Addr:0x%02x WriteQut:0x%02X [%s Read:0x%02X]", ps_i2c_address_, sm.writebytes, d, sm.readbytes);
        pub("pmbus/info/write", msg);
        Log.noticeln("%s", msg);
        if(sm.readbytes > 255) {
        Log.errorln(F("Read Blocks: Fail size too big."));
        pub("pmbus/info/read", "Read Blocks: Fail size too big.");
        break;
        }
        if(i2c_blockWriteReadBlock(ps_i2c_address_, sm.writebytes, sm.datablock, sm.readbytes, sm.datablock_b)){
        Log.errorln(F("I2C Write/Read Block Fail."));
        pub("pmbus/info/write", "Write/Read Block Fail.");
        break;
        }
        if(sm.readbytes != 0)
        {
          for (int n = 0; n < sm.readbytes; n++){           
            d[3*n] = hex_table[sm.datablock_b[n] >> 4];
            d[3*n + 1] = hex_table[sm.datablock_b[n] & 0x0f];
            d[3*n + 2] = ' ';
          }
          d[3*sm.readbytes - 1] = '\0';       
          snprintf (msg, MSG_BUFFER_SIZE, "ReadQut:0x%02X [%s]", sm.readbytes, d);
          pub("pmbus/info/read", msg);
          Log.noticeln("%s", msg);
        }
        delay(5);
        break;
        
      case 'h':
         SyntaxHelp();
         delay(1000); 
         break;        
      default:
        if (com != 'm')
        Log.noticeln(F("Invalid Selection"));
        break;
    }
}
