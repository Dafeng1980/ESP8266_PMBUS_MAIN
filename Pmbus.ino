float L11_to_float(uint16_t input_val)
{
// extract exponent as MS 5 bits
int8_t exponent = input_val >> 11;
// extract mantissa as LS 11 bits
int16_t mantissa = input_val & 0x7ff;
// sign extend exponent from 5 to 8 bits
if( exponent > 0x0F ) exponent |= 0xE0;
// sign extend mantissa from 11 to 16 bits
if( mantissa > 0x03FF ) mantissa |= 0xF800;
// compute value as mantissa * 2^(exponent)
return mantissa * pow(2.0,exponent);
}

float L16_to_float_mode(uint8_t expvmode, uint16_t input_val)
{
  // Assume Linear 16, pull out 5 bits of exponent, and use signed value.
  // int8_t exponent = (int8_t) vout_mode & 0x1F;
  int8_t exponent = expvmode;   
  float mantissa = (float)input_val;
  // sign extend exponent
  if( exponent > 0x0F ) exponent |= 0xE0;
  // Convert mantissa to a float so we can do math.
  // sign extend mantissa
  // if( mantissa > 0x03FF ) mantissa |= 0xF800;
  // compute value as mantissa * 2^(exponent)
  return mantissa * pow(2.0,exponent);
}

uint16_t float_to_lin11(float input_val)
{
      // set exponent to -16
    int exponent = -16;
    // extract mantissa from input value
    int mantissa = (int)(input_val / pow(2.0, exponent));
    // Search for an exponent that produces
    // a valid 11-bit mantissa
   do
    {
   if((mantissa >= -1024) && (mantissa <= +1023))
    {
      break; // stop if mantissa valid
    }
      exponent++;
      mantissa = (int)(input_val / pow(2.0, exponent));
    } 
  while (exponent < +15);  // Format the exponent of the L11
  uint16 uExponent = exponent << 11;  // Format the mantissa of the L11
  uint16 uMantissa = mantissa & 0x07FF;  // Compute value as exponent | mantissa
  return uExponent | uMantissa;
}

uint16_t float_to_L16_mode(uint8_t vout_mode, float input_val)
{
  // Assume Linear 16, pull out 5 bits of exponent, and use signed value.
  int8_t exponent = vout_mode & 0x1F;   // Sign extend exponent from 5 to 8 bits
  if (exponent > 0x0F) exponent |= 0xE0;  // Scale the value to a mantissa based on the exponent
  uint16_t mantissa = (uint16_t)(input_val / pow(2.0, exponent));
  return mantissa;
}


void pmbus_setPage(uint8_t address, uint8_t page)
{
  // Set the page of the device to desired_page
  smbus_writeByte(address, 0x00, page);  //PAGE 0x00;
}

uint8_t pmbus_getPage(uint8_t address)
{
   return smbus_readByte(address, 0x00);  //PAGE = 0x00;
}

void pmbus_clearFaults(uint8_t address)
{
  smbus_sendByte(address, 0x03);  //CLEAR_FAULTS = 0x03;
}

void pmbus_writeProtect(uint8_t address, uint8_t val)  // val = 0x00 to disable Write-Protect, 0x80 enablle.
{
  smbus_writeByte(address, 0x10, val);  //WRITE_PROTECT CMD 0x10;
}

float pmbus_readVout(uint8_t address)
{
  int8_t vout_mode;
  uint16_t vout_L16;
  // Read the output voltage as an L16
    vout_L16 = smbus_readWord(address, 0x8B);  //READ_VOUT = 0x8B
    vout_mode = (unsigned int)(smbus_readByte(address, 0x20) & 0x1F);  //Read VOUT_MODE(0x20) & 0x1F
    return L16_to_float_mode(vout_mode, vout_L16);
  }

float pmbus_readVin(uint8_t address)
{
  uint16_t vin_L11;
  // read the input voltage as an L11
    vin_L11 = smbus_readWord(address, 0x88);  //READ_VIN = 0x88;
  // convert L11 value to floating point value
  return L11_to_float(vin_L11);
}

float pmbus_readIin(uint8_t address)
{
  uint16_t iin_L11;
    iin_L11 = smbus_readWord(address, 0x89);  //READ_IIN = 0x89;
  return L11_to_float(iin_L11);
}

float pmbus_readIout(uint8_t address)
{
  uint16_t iout_L11;
    iout_L11 = smbus_readWord(address, 0x8C);  //READ_IOUT = 0x8C;
  return L11_to_float(iout_L11);
}

float pmbus_readPin(uint8_t address)
{
  uint16_t pin_L11;
    pin_L11 = smbus_readWord(address, 0x97);  //READ_PIN = 0x97;
  return L11_to_float(pin_L11);
}

float pmbus_readPout(uint8_t address)
{
  uint16_t pout_L11;
    pout_L11 = smbus_readWord(address, 0x96);  //READ_POUT = 0x96;
  return L11_to_float(pout_L11);
}

float pmbus_readOtemp(uint8_t address)
{
  uint16_t temp_L11;
  temp_L11 = smbus_readWord(address, 0x8D);  //temp sensor 0x8D  
  return L11_to_float(temp_L11);
}

float pmbus_readItemp(uint8_t address)
{
  uint16_t temp_L11;
  temp_L11 = smbus_readWord(address, 0x8E);  //temp sensor 0x8E  
  return L11_to_float(temp_L11);
}

float pmbus_readMtemp(uint8_t address)
{
  uint16_t temp_L11;
  temp_L11 = smbus_readWord(address, 0x8F);  //temp sensor 0x8F  
  return L11_to_float(temp_L11);
}

float pmbus_readFanSpeed1(uint8_t address)  //Fan speed1
{
  uint16_t temp_L11;
  temp_L11 = smbus_readWord(address, 0x90);
  return L11_to_float(temp_L11);
}

float pmbus_readFanSpeed2(uint8_t address)  //Fan speed2
{
  uint16_t temp_L11;
  temp_L11 = smbus_readWord(address, 0x91);
  return L11_to_float(temp_L11);
}

uint8_t pmbus_readStatusByte(uint8_t address)
{
  uint8_t status_byte;
  status_byte = smbus_readByte(address, 0x78);  //STATUS_BYTE = 0x78;
  return status_byte;
}

uint16_t pmbus_readStatusWord(uint8_t address)
{
  uint16_t status_word;
  status_word = smbus_readWord(address, 0x79);  //STATUS_WORD = 0x79;
  return status_word;
}

uint8_t pmbus_readStatusVout(uint8_t address)
{
  uint8_t status_byte;
  status_byte = smbus_readByte(address, 0x7A);  //STATUS_VOUT = 0x7A;
  return status_byte;
}

uint8_t pmbus_readStatusIout(uint8_t address)
{
  uint8_t status_byte;
  status_byte = smbus_readByte(address, 0x7B);  //STATUS_IOUT = 0x7B;
  return status_byte;
}

uint8_t pmbus_readStatusInput(uint8_t address)
{
  uint8_t status_byte;
  status_byte = smbus_readByte(address, 0x7C);  //STATUS_INPUT = 0x7C;
  return status_byte;
}

uint8_t pmbus_readStatusTemp(uint8_t address)
{
  uint8_t status_byte;
  status_byte = smbus_readByte(address, 0x7D);  //STATUS_TEMP = 0x7D;
  return status_byte;
}

uint8_t pmbus_readStatusCml(uint8_t address)
{
  uint8_t status_byte;
  status_byte = smbus_readByte(address, 0x7E);  //STATUS_CML = 0x7E;
  return status_byte;
}

uint8_t pmbus_readStatusFan(uint8_t address)
{
  uint8_t status_byte;
  status_byte = smbus_readByte(address, 0x81);  //STATUS_FAN = 0x81;
  return status_byte;
}

static  unsigned long lastEnergy_in,lastSample_in,currentEnergy_in,currentSample_in;
static  unsigned long lastEnergy_out,lastSample_out,currentEnergy_out,currentSample_out;
void pmbus_Ein_Eout_read()
{
  uint8_t data[6];
  unsigned long currentread = millis();
    if(energyflag)
      {
        smbus_readBlock(ps_i2c_address, 0x86, data, 6);            //READ_EIN = 0x86;
        lastEnergy_in = data[0] + data[1]*0x100 + data[2]*0x7FFF;
        lastSample_in = data[3] + data[4]*0x100 + data[5]*0x10000;
        smbus_readBlock(ps_i2c_address, 0x87, data, 6);            //READ_EOUT = 0x87;
        lastEnergy_out = data[0] + data[1]*0x100 + data[2]*0x7FFF;
        lastSample_out = data[3] + data[4]*0x100 + data[5]*0x10000;
        currentread = millis();
        previousMillis_energy = currentread;
        energyflag = false;
      }
    if (currentread - previousMillis_energy >= 1000 && !energyflag)
    {
        smbus_readBlock(ps_i2c_address, 0x86, data, 6);
        currentEnergy_in = data[0] + data[1]*0x100 + data[2]*0x7FFF;
        currentSample_in = data[3] + data[4]*0x100 + data[5]*0x10000;
        smbus_readBlock(ps_i2c_address, 0x87, data, 6);
        currentEnergy_out = data[0] + data[1]*0x100 + data[2]*0x7FFF;
        currentSample_out = data[3] + data[4]*0x100 + data[5]*0x10000;
        pd.inputE = (float)(currentEnergy_in - lastEnergy_in)/(float)(currentSample_in - lastSample_in);
        pd.outputE = (float)(currentEnergy_out - lastEnergy_out)/(float)(currentSample_out - lastSample_out);
        energyflag =true;
    }    
}