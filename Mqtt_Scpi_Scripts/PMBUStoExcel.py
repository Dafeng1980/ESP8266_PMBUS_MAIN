"""
Created on 2022

@author: Dafeng
"""
import time
import paho.mqtt.client as mqtt
import xlwings as xw
import sys

client = mqtt.Client()
class MQTTtoExcel:    
    def __init__(self, mqtt_server):
        self.filepath = r'D:\My Documents\pmbus\pmbus_excel.xlsx'
        self.app=xw.App(visible=True,add_book=False)
        self.display_alerts=False
        self.screen_updating=True
        self.wb = self.app.books.open(self.filepath)
        self.mqtt_server = mqtt_server
        #self.filename = " "
        self.records = 600 
        self.Base_topic = "npicsu/"
        self.username = 'dfiot'
        self.password = '123abc'
        self.ws = self.wb.sheets['sheet1']
        #self.ws1 = self.wb.sheets['sheet2']      
        self.count = 0
        self.eep_c = 14
        
    def on_connect(self,client, userdata, flags, rc):
        print("Connected with result code "+str(rc))
        if rc == 0:
            print("Connected success")
            #client.subscribe(self.Base_topic + "pmbus/#")
            self.sub('pmbus/#')
        else:
            print(f"Connected fail with code {rc}")       

    def set_records(self,number):
        self.records = number

    def set_polltime(self, minisec):
        if(minisec >= 100 and minisec <= 60000):
            high = (int(minisec) >> 8) & 0xFF
            low = int(minisec) & 0xFF
            self.pub("pmbus/set", "[AA 01 {:02x} {:02x}]".format(high, low))
            time.sleep(0.1)
            #print("pmbus/set", "[AA 01 {:02x} {:02x}]".format(high, low))
    def set_addr(self, addr):
        addr = int(addr, 16)
        if(addr >= 0x03 and addr <= 0x7F):
            self.pub("pmbus/set", "[AA 00 {:02x}]".format(addr))
            time.sleep(0.1)

    def pub(self, topic, msg):
        client.publish(self.Base_topic + topic, msg, qos=0, retain=False)

    def sub(self, topic):
        client.subscribe(self.Base_topic + topic, qos=0)

    def on_message(self, client, userdata, msg):
        if(msg.topic.find('word') > 0):
            print("Status Word(0x79): 0x%04x" % int(msg.payload[2:6], 16))
            self.ws.range('H2').value = ("%02x" % int(msg.payload[2:4], 16))
            self.ws.range('I2').value = ("%02x" % int(msg.payload[4:6], 16))

        if(msg.topic.find('all') > 0):
           print(msg.payload.decode())
           self.ws.range('I8').value = ("%02x" % int(msg.payload[6:8], 16))   # CML
           self.ws.range('I6').value = ("%02x" % int(msg.payload[16:18], 16))  #temperature
           self.ws.range('I7').value = ("%02x" % int(msg.payload[25:27], 16))  # Fan
           self.ws.range('I3').value = ("%02x" % int(msg.payload[35:37], 16))  # Vout
           self.ws.range('I4').value = ("%02x" % int(msg.payload[45:47], 16))  # Iout
           self.ws.range('I5').value = ("%02x" % int(msg.payload[56:58], 16))  #input

        if(str(msg.payload).find('PMBUS') > 0):
            print(time.strftime("%d-%m-%Y %H:%M:%S", time.localtime()))
            self.ws.range('P3').value = ("0x%0x" % int(msg.payload[14:16], 16))
            print("Refresh-:%d" % int(msg.payload[25:]))
            self.ws.range('P1').value = ("Refresh:%d" % int(msg.payload[25:]))
            self.count = self.count + 1
            if(self.count >= self.records):
                self.wb.save()   
                self.wb.close()
                self.app.quit()
                sys.exit("Complete ")

    def topic_output_callback(self,lient, userdata, msg):
        if(msg.topic.find('curr') > 0):
            print("Onput Current: %.3fA" % float(msg.payload))
            self.ws.range('B3').value=float(msg.payload)
        if(msg.topic.find('volt') > 0):
            print("Onput Voltage: %.4fV" % float(msg.payload))
            self.ws.range('B2').value=float(msg.payload)
        if(msg.topic.find('power') > 0):
            print("Onput Power: %.2fW" % float(msg.payload))
            self.ws.range('B7').value=float(msg.payload)
    
    def topic_input_callback(self, lient, userdata, msg):
        if(msg.topic.find('curr') > 0):
            print("Input Current: %.3fA" % float(msg.payload))
            self.ws.range('B5').value=float(msg.payload)
        if(msg.topic.find('volt') > 0):
            print("Input Voltage: %.2fV" % float(msg.payload))
            self.ws.range('B4').value=float(msg.payload)
        if(msg.topic.find('power') > 0):
            print("Input Power: %.2fW" % float(msg.payload))
            self.ws.range('B6').value=float(msg.payload)    
    
    def topic_sensor_callback(self, lient, userdata, msg):
        if(msg.topic.find('temp1') > 0):
            print("Inlet Temp(0x8D): %.1fC" % float(msg.payload))
            self.ws.range('B8').value=float(msg.payload)
        if(msg.topic.find('temp2') > 0):
            print("Pri Temp(0x8E): %.1fC" % float(msg.payload))
            self.ws.range('B9').value=float(msg.payload)
        if(msg.topic.find('temp3') > 0):
            print("Sec Temp(0x8F): %.1fC" % float(msg.payload))
            self.ws.range('B10').value=float(msg.payload)
        if(msg.topic.find('fan') > 0):
            print("Fan Speed: %.1fRPM" % float(msg.payload))
            self.ws.range('B11').value=float(msg.payload)
    
    def topic_info_callback(self, lient, userdata, msg):
        if(msg.topic.find('eread') > 0):
            print(msg.payload.decode())
            self.ws.range('A'+ "{:d}".format(self.eep_c)).value= msg.payload.decode()
            self.eep_c = self.eep_c +1
            if(self.eep_c > 29):
                self.eep_c = 14
        elif(msg.topic.find('read') > 0):
            print(msg.payload.decode())
            self.ws.range('A13').value= msg.payload.decode()
        elif(msg.topic.find('write') > 0):
            #print(msg.payload.decode())
            self.ws.range('E11').value= msg.payload.decode()
        else:
            self.ws.range('G11').value= msg.payload.decode()

    def eeprom_com(self, offset, addr = 0x50):
        self.pub("pmbus/set", "[08 {:02x} {:02x} {:02x} 10 01]".format(addr, (offset >> 8) & 0xFF, (offset & 0xff)))

    def start(self):
        # client = mqtt.Client()
        client.on_connect = self.on_connect
        client.on_message = self.on_message
        client.username_pw_set(self.username, self.password)
        client.message_callback_add(self.Base_topic + "pmbus/output/#", self.topic_output_callback)
        client.message_callback_add(self.Base_topic + "pmbus/input/#", self.topic_input_callback)
        client.message_callback_add(self.Base_topic + "pmbus/sensor/#", self.topic_sensor_callback)
        client.message_callback_add(self.Base_topic + "pmbus/info/#", self.topic_info_callback)
        #client.username_pw_set('dfiot', '123abc')
        #client.connect("192.168.200.2", 1883, 60)
        client.connect(self.mqtt_server, 1883, 60)
        #client.publish(self.Base_topic + "pmbus/set", "[AA 04]", qos=0, retain=False)
        time.sleep(0.1)    
        
    def mqttloop(self):
        #client.loop_forever()
        client.loop()

def main():
    ptoe = MQTTtoExcel("192.168.200.2")
    # ptoe.set_records(16)
    ptoe.start()
    ptoe.set_polltime(1000)
    ptoe.pub("pmbus/set", "[AA 04]") #Enable sensor read&pub
    time.sleep(0.1)
    while True:
        if(ptoe.ws.range('F10').value == "Enable"):
            if(ptoe.ws.range('G10').value == "Send_Comm_"):
                ptoe.pub("pmbus/set", ptoe.ws.range('H11').value)
                time.sleep(0.1)
            elif(ptoe.ws.range('G10').value == "EEprom_"):
                ptoe.pub("pmbus/set", '[AA 02 00]')
                for i in range(16):
                    ptoe.eeprom_com(i*16)
                    time.sleep(0.05)
                ptoe.pub("pmbus/set", '[AA 02 01]')
                time.sleep(0.1) 
            elif(ptoe.ws.range('G10').value == "Set_Poll_"):
                ptoe.set_polltime(ptoe.ws.range('H10').value)
                #ptoe.pub("pmbus/set", '[AA 02 00]')                                    
            elif(ptoe.ws.range('G10').value == "I2c_Scan_"):
                ptoe.pub("pmbus/set", '[AA 05]')
                time.sleep(0.1)
            elif(ptoe.ws.range('G10').value == "Set_Addr_"):
                ptoe.set_addr(ptoe.ws.range('H10').value)
            elif(ptoe.ws.range('G10').value == "Shut_Down_"):
                time.sleep(0.2)
                ptoe.ws.range('F10').value = "Disable"
                ptoe.wb.save()
                ptoe.wb.close()
                ptoe.app.quit()
                sys.exit("Complete ")
            ptoe.ws.range('F10').value = "Disable"
        ptoe.mqttloop()

if __name__ == '__main__':
    main()

