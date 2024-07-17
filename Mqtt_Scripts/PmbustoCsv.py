"""
Created on 2023

@author: Dafeng
"""
import time
import paho.mqtt.client as mqtt
import sys
import json
import csv
client = mqtt.Client()
path_now = 'pmbus_record_' + time.strftime('%m_%d_%H_%M', time.localtime()) + '.csv'
data_file = open(r'./log/' + path_now ,  'w', newline='' )
# data_file = open('pmbusdata_' + time.strftime("%m_%d_%H_%M", time.localtime()) + '.csv', 'w', newline='')
csv_writer = csv.writer(data_file)
class MQTTtoCsv:    
    def __init__(self, mqtt_server):
        self.mqtt_server = mqtt_server
        self.records = 600 
        self.Base_topic = "npi/"
        self.username = 'dfiot'
        self.password = '123abc'
        self.mqtt_read = ''
        self.mqtt_topic = ''
        self.json_c = 0
        self.csv_c = 0    
        self.count = 0
        
    def on_connect(self,client, userdata, flags, rc):
        print("Connected with result code "+str(rc))
        if rc == 0:
            print("Connected success")
            self.sub('pmbus/jsoninfo')
        else:
            print(f"Connected fail with code {rc}")       

    def set_records(self,number):
        self.records = number

    def set_addr(self, addr):
        addr = int(addr, 16)
        if(addr >= 0x03 and addr <= 0x7F):
            self.pub("pmbus/set", "[AA 00 {:02x}]".format(addr))
            time.sleep(0.1)
    
    def playload_read(self):
        return self.mqtt_read

    def topic_read(self):
        return self.mqtt_topic

    def pub(self, topic, msg):
        client.publish(self.Base_topic + topic, msg, qos=0, retain=False)

    def sub(self, topic):
        client.subscribe(self.Base_topic + topic, qos=0)

    def on_message(self, client, userdata, msg):
        data = json.loads(msg.payload)
        print(data)
        data.update({'timestamp' : time.strftime("%H:%M:%S", time.localtime())})
        if self.csv_c == 0:
            header = data.keys()
            csv_writer.writerow(header)
            self.csv_c +=1
        csv_writer.writerow(data.values())      

    def start(self):
        # client = mqtt.Client()
        client.on_connect = self.on_connect
        client.on_message = self.on_message
        client.username_pw_set(self.username, self.password)
        client.connect(self.mqtt_server, 1883, 60)
        client.loop_start()
        print('mqtt start')
        time.sleep(0.1)    
        
    def mqttloop(self):
        #client.loop_forever()
        client.loop()
    
    def mqtt_disconnect(self):
        client.loop_stop()
        client.disconnect()
        print('disconnect')
        sys.exit("Complete ")

def main():
    m2c = MQTTtoCsv("192.168.2.200")
    if len(sys.argv) == 2:
        logtime = float(sys.argv[1]) * 60  #input script argument time(min)
    else: logtime = 600.0                  #default time 10min
    m2c.start()
    time.sleep(0.1)
    counter = 0
    while True:
        counter+=1
        if(counter >= logtime):
            counter = 0
            m2c.mqtt_disconnect()
        time.sleep(1)
        # m2c.mqttloop()       
if __name__ == '__main__':
    main()

