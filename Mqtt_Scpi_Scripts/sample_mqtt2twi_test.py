from MQTT2Twi import MQTT2Twi
import time
import struct

m2t = MQTT2Twi("192.168.200.2", "npicsu/", 0x58)
unlock = [0xFF, 0xAA, 0x55, 0xAA, 0x55, 0x00]
read_dac = [0x18, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00]

def float2Bytes(f): 
    bs = struct.pack("f", f)
        #return ([bs[3],bs[2],bs[1],bs[0]]) # MSB - LSB
    return ([bs[0],bs[1],bs[2],bs[3]])  # LSB - MSB

def main():  
    m2t.mqtt_init_start()
    time.sleep(1)
    m2t.pub("pmbus/status", "1056")
    time.sleep(0.2)
    m2t.setTwiAddress(0x5E)
    m2t.i2cwrite(unlock) 
    m2t.smread_byte(0x00)
    m2t.smread_word(0x79)
    m2t.smread_blocks(0x03, 0x10)
    m2t.smsent_byte(0x03)
    m2t.smwrite_byte(0x00, 0x01)
    m2t.smwrite_word(0xF3, 0x55, 0xaa)
    m2t.setEepromAddress(0x53)
    m2t.smwriteread_blocks(0xff, read_dac, 0x0b, True)
    m2t.eeprom_reads(0x10, 0x10)
    time.sleep(1)
    m2t.mqtt_disconnect()

if __name__ == '__main__':
    main()


