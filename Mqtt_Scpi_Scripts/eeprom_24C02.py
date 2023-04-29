import paho.mqtt.client as mqtt 
import time
import sys
#import datetime
broker = '192.168.200.2'
port = 1883
username = 'dfiot'
password = '123abc'
Base_topic = "npicsu/"
pmbus_set = "pmbus/set"
eepromaddr = 0x50
command = "[08 {:02x} {:02x} {:02x} 10 01]" 
#now  = datetime.datetime.now().strftime("%m_%d_%H")
path_now = "eep24c02_" + time.strftime("%m_%d_%H", time.localtime()) + ".txt"

file = open (path_now, "a")
client = mqtt.Client()

def on_connect(client, userdata, flags, rc):
	if rc == 0:
		print("Connected success")
		sub('pmbus/info/eread')
	else:
		print(f"Connected fail with code {rc}")

def pub(topic, msg):
    client.publish(Base_topic + topic, msg, qos=0, retain=False)

def sub(topic):
    client.subscribe(Base_topic + topic, qos=0)

def strmat(msg):
	if(msg.find('Fail') > 0):
		return "EEPROM Read Fail"
	str1 = msg[8:15]
	first = msg.find('[')
	end = msg.find(']')
	if(msg[first+1] == ' '):
		str = msg[(first+2):end]
	else:
		str = msg[(first+1):end]
	list = str1 +" " +str
	return list

def on_message(client, userdata, msg):
	if(msg.topic.find('info/eread') > 0):
		print(strmat(str(msg.payload)))
		file.write(strmat(str(msg.payload)) + "\n")
# sub('pmbus/info/eread')
def connect2mqtt():
	client.on_connect = on_connect
	client.on_message = on_message
	client.username_pw_set(username, password)
	client.connect(broker, port, 60)
	client.loop_start()
def disconect():
	client.loop_stop()			    
	print('disconnect')
	client.disconnect()
def sentcom(offset, addr=0x50):
	msb = offset >> 8
	lsb = offset & 0xff
	pub(pmbus_set, command.format(addr, msb, lsb))
	#print('%s' % command.format(addr, msb, lsb))
def readeeprom():
	file.write(" \n")
	pub(pmbus_set, '[AA 02 00]') # Disable Pmbus Polling
	time.sleep(1)
	for i in range(16):    #read EEPROM 16*16 = 256, 24C02
		sentcom(i*16, eepromaddr)
		time.sleep(0.05)
	time.sleep(1)
	pub(pmbus_set, '[AA 02 01]') # Enable Pmbus Polling
	file.write(" \n")

def main():
	global eepromaddr
	global Base_topic
	if len(sys.argv) == 2:
		Base_topic = (sys.argv[1] + '/')
	if len(sys.argv) == 3:	
		Base_topic = (sys.argv[1] + '/')
		eepromaddr = int(sys.argv[2], 16)
	connect2mqtt()
	readeeprom()
	disconect()
if __name__ == '__main__':
    main()
