"""
Created on 2023

@author: Dafeng
"""
import paho.mqtt.client as mqtt
import sys

class MQTT2Twi:

    def __init__(self, mqtt_server, topic_prefix, addr):
        self.client = mqtt.Client()
        self.mqtt_server = mqtt_server
        self.port = 1883
        self.topic_prefix = topic_prefix
        self.addr = addr
        self.pmbus_set = 'pmbus/set'
        self.username = 'dfiot'
        self.password = '123abc'
        self.no_read = 0 
        self.eeprom_addr = 0x50
        self.sm_sent_byte = "[07 {:02x} {:02x}]"
        self.sm_read_byte = "[00 {:02x} {:02x}]"
        self.sm_read_word = "[01 {:02x} {:02x} {:02x}]"
        self.sm_read_block = "[02 {:02x} {:02x} {:02x}]"
        self.sm_write_byte = "[03 {:02x} {:02x} {:02x}]"
        self.sm_write_word = "[04 {:02x} {:02x} {:02x} {:02x}]"
        self.eeprom_read = "[08 {:02x} {:02x} {:02x} {:02x} {:02x}]"
        # self.client = mqtt.Client()
        self.mqttsentstatus = False
        self.mqtt_read = ''
        self.mqtt_topic = ''

    def on_connect(self, client, userdata, flags, rc):
        if rc == 0:
            print("Connected success")
        else:
            print(f"Connected fail with code {rc}")

    def on_message(self, client, userdata, msg):
        if (self.mqttsentstatus):    
            self.mqtt_topic = msg.topic
            self.mqtt_read = msg.payload
            self.mqttsentstatus = False
        # print(f"{msg.topic}        {msg.payload}")

    def pub(self, topic, msg):
        self.client.publish(self.topic_prefix + topic, msg, qos=0, retain=False)
        self.mqttsentstatus = True
        print(msg)

    def pub_pmbus_set(self, msg):
        self.pub(self.pmbus_set, msg)

    def sub(self, topic):
        self.client.subscribe(self.topic_prefix + topic, qos=0)
    
    def sumcrc16(self, data):
        crc = 0xffff
        for pos in data:
            crc ^= pos
            for  i  in  range(8):
                lsb = crc & 0x0001
                crc >>= 1
                if lsb == 1:
                    crc ^= 0x8408	
        crc ^= 0xffff
        return crc

    def pecget(self, listdata):
        pec = 0x00
        for x in listdata:
            pec = pec ^ x
		#print("x=0x%02x" % x)
            for  _  in  range(0, 8):
                if((pec & 0x80) > 0x00):	
                    pec = ((pec << 1) ^ 0x07) & 0xff
                else:
                    pec = (pec << 1) & 0xff
		#print("pec=0x%02x" % pec)
        return pec

    def setTopicPrefix(self, topic_prefix): 
        self.topic_prefix = topic_prefix

    def setTwiAddress(self, addr):
        self.addr = addr

    def setEepromAddress(self, addr):
        self.eeprom_addr = addr

    def playload_read(self):
        return self.mqtt_read

    def topic_read(self):
        return self.mqtt_topic

    def get_sentstatus(self):
        return self.mqttsentstatus

    def mqtt_init_start(self):
        # client = mqtt.Client()
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        self.client.username_pw_set(self.username, self.password)
        self.client.connect(self.mqtt_server, self.port, 60)
        self.client.loop_start()
        print('mqtt start')

    def mqtt_disconnect(self):
        self.client.loop_stop()
        self.client.disconnect()
        print('disconnect')
        sys.exit("Complete ")

    def i2cwrite_read(self, listcom, rcount,):
        stra = self.i2ccom2hexstr(listcom, rcount)
        self.pub_pmbus_set(stra)

    def i2cwrite(self, listcom):
        self.i2cwrite_read(listcom, self.no_read)

    def i2cwrite_pec(self, listcom):
        pec = self.pecget([(self.addr<<1)]+listcom)
        self.i2cwrite(listcom+[pec])

    def i2ccom2hexstr(self, listcom, rcount):
        list_str = '['
        list_str += '09'
        list_str += ' {:02X}'.format(self.addr)
        list_str += ' {:02X}'.format(len(listcom))
        for x in listcom:
            list_str += ' {:02X}'.format(x)
        list_str += ' {:02X}'.format(rcount)
        list_str += ']'
        return list_str

    def smwrite_byte(self, com, val):
        stra = self.sm_write_byte.format(self.addr, com, val)
        self.pub_pmbus_set(stra)

    def smwrite_word(self, com, msb, lsb):
        stra = self.sm_write_word.format(self.addr, com, msb, lsb)
        self.pub_pmbus_set(stra)
    
    def smread_byte(self, com):
        stra = self.sm_read_byte.format(self.addr, com)
        self.pub_pmbus_set(stra)
        
    def smread_word(self, com, fort = 0):   #fort = 0, return smbus int,  =1, return pmbus L11, =2, returm pmbus L16
        stra = self.sm_read_word.format(self.addr, com, fort)
        self.pub_pmbus_set(stra)

    def smread_blocks(self, com, count):
        stra = self.sm_read_block.format(self.addr, com, count)
        self.pub_pmbus_set(stra)
    
    def smsent_byte(self, com):
        stra = self.sm_sent_byte.format(self.addr, com)
        self.pub_pmbus_set(stra)
    
    def smwriteread_block2hexstr(self, com, listcom, rcount, crc16):
        wcount = len(listcom)
        if crc16 == True:
            crc =  self.sumcrc16(listcom)
            lsb = (crc & 0xff)
            msb = (crc >> 8)
            wcount = wcount + 2
        list_str = '['
        list_str += '06'
        list_str += ' {:02X}'.format(self.addr)
        list_str += ' {:02X}'.format(com)
        list_str += ' {:02X}'.format(wcount)
        for x in listcom:
            list_str += ' {:02X}'.format(x)
        if crc16 == True:
            list_str += ' {:02X}'.format(lsb)
            list_str += ' {:02X}'.format(msb)
        list_str += ' {:02X}'.format(rcount)
        list_str += ']'
        return list_str
    
    def smwriteread_blocks(self, com, listcom, rcount, crc16 = False):
        stra = self.smwriteread_block2hexstr(com, listcom, rcount, crc16)
        self.pub_pmbus_set(stra)
    
    def eeprom_reads(self, offset, rcount, eepromtype = True):
        msb = offset >> 8
        lsb = offset & 0xff
        stra = self.eeprom_read.format(self.eeprom_addr, msb, lsb, rcount, eepromtype)
        self.pub_pmbus_set(stra)

            
         