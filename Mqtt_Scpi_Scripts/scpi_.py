import paho.mqtt.client as mqtt 
import time

broker = '192.168.200.2'
port = 1883
username = 'dfiot'
password = '123abc'
Base_topic = "npicsu/"
pmbus_set = "pmbus/set"
scpi_set = "scpi/set"
#path_now = "scpi_" + time.strftime("%m_%d_%H", time.localtime()) + "_log.txt"
#file = open (path_now, "a")
client = mqtt.Client()

def on_connect(client, userdata, flags, rc):
	if rc == 0:
		print("Connected success")
		# sub('scpi/#')
	else:
		print(f"Connected fail with code {rc}")

def pub(topic, msg):
    client.publish(Base_topic + topic, msg, qos=0, retain=False)

def sub(topic):
    client.subscribe(Base_topic + topic, qos=0)

def on_message(client, userdata, msg):
	print(msg.payload.decode())
	#file.write(msg.payload.decode() + "\n")
	
def mqtt_init_start():
    client.on_connect = on_connect
    client.on_message = on_message
    client.username_pw_set(username, password)
    client.connect(broker, port, 60)
    client.loop_start()
    print('mqtt start')
def mqtt_stop_disconnect():
    client.loop_stop()
    client.disconnect()
    print('disconnect')
def sent_scpi_com(scpi_com):
    pub(scpi_set, "%" + scpi_com + "%")
    time.sleep(0.01)   

def set_dynload():   
    l1 = input('Input L1 Current (A): ')
    if(l1==''):
        l1 = 20   
    l2 = input('Input L2 Current (A): ')
    if(l2==''):
        l2 = 50 
    r1 = input('Input Rise rate (A/us): ')
    if(r1==''):
        r1 = 1   
    r2 = input('Input Fall rate (A/us): ')
    if(r2==''):
        r2 = 1 
    t1 = input('Input L1 Time (ms): ')
    if(t1==''):
        t1 = 10
    t2 = input('Input L2 rate (ms): ')
    if(t2==''):
        t2 = 10
    sent_scpi_com("*rst")
    sent_scpi_com("mode 3")
    sent_scpi_com("curr:dyn:l1 {:.2f}".format(float(l1)))
    sent_scpi_com("curr:dyn:l2 {:.2f}".format(float(l2)))
    sent_scpi_com("curr:dyn:rise {:.2f}".format(float(r1)))
    sent_scpi_com("curr:dyn:fall {:.2f}".format(float(r2)))
    sent_scpi_com("curr:dyn:t1 {:.2f}ms".format(float(t1)))
    sent_scpi_com("curr:dyn:t2 {:.2f}ms".format(float(t2)))
    sent_scpi_com("load 1")

def main():
    mqtt_init_start()
    pub(pmbus_set, '[AA 02 00]') # Disable Pmbus Polling
    set_dynload()
    pub(pmbus_set, '[AA 02 01]') # Enable Pmbus Polling
    time.sleep(2)
    mqtt_stop_disconnect()
#file.write(" \n")
if __name__ == '__main__':
    main()