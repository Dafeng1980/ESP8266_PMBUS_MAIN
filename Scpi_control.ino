void sendscpiread(char *msg) {
  if(cont_str(msg) == 0xAF){
      Log.noticeln(F("SCPI Commands, Fail"));
      pub("scpi/info", "SCPI Commands, Fail");
      delay(10);
      return;
   }
  Serial.println(msg);
  String scpi_str = msg;
  if(scpi_str.lastIndexOf("?") > 0){
        if(read_scpi() != 0xAF) {                
                 Log.noticeln("%s", ui_buffer);
                 pub("scpi/readback", ui_buffer);  
             }
        else {
                Log.noticeln(F("SCPI Read Time Out, Fail"));
                pub("scpi/readback", "SCPI Read Time Out, Fail");
              }
   }
    pub("scpi/info", msg);
    Log.noticeln("%s", msg);
    delay(10);
}

void modifycurr(float val){
  char currval[32];
  sprintf(currval, "curr:stat:l1 %2.1f", val );
  Serial.println("*rst");
  delay(20);
  Serial.println("mode 1");
  delay(20);
  Serial.println(currval);
  delay(20);
  Serial.println("load 1");
  delay(10);
  pub("scpi/info", currval);
}

int cont_str(char *str) {
  int i = 0;
  while (str[i++] != '\0')
  if (i >= 128) return 0xAF;
    return i;  
}

uint8_t read_scpi()
{
  uint8_t index = 0; 
  int c; 
  unsigned long currentread = millis();
  previousMillis = currentread;
  while (index < UI_BUFFER_SIZE-1)
  {
    ESP.wdtFeed();      
    currentread = millis();
    c = Serial.read();      //read one character
    if (((char) c == '\r') || ((char) c == '\n')) break;  // if carriage return or linefeed, stop and return data
    if (c >= 0) ui_buffer[index++]=(char) c;   // put character into ui_buffer   
    if (currentread - previousMillis >= 3000) return 0xAF;
  }
  ui_buffer[index]='\0';  // terminate string with NULL
  if ((char) c == '\r')    // if the last character was a carriage return, also clear linefeed if it is next character
  {
      delay(10);             // allow 10ms for linefeed to appear on serial pins
    if (Serial.peek() == '\n') Serial.read(); // if linefeed appears, read it and throw it away
  }
//  pub("scpi/readback", "SCPI Readblack, comleate");
  return index; // return number of characters, not including null terminator
}
