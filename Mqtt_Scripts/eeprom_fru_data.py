from MQTT2Twi import MQTT2Twi
import time

m2t = MQTT2Twi("192.168.200.2", "npi/", 0x58)
path_now = "fru_data_" + time.strftime("%m_%d_%H", time.localtime()) + ".txt"
file = open (path_now, "a")

def fru_checksun(listdata):
    checksum = 0
    for i in range(0x30, 0xA7):
        checksum = checksum + listdata[i]       
    return (0x10000 - checksum) & 0xff

def str2list(stra):
    listdata = []
    for i in range(16):
        listdata.append(int(stra[3*i:3*i+2], 16))
    return listdata

def fru_read():
    fru_data=[]
    read_offset = 0x00
    m2t.pub_pmbus_set('[AA 02 00]')
    time.sleep(0.1)
    
    for i in range(16):
        read_offset = i*16
        m2t.eeprom_reads(read_offset, 0x10)
        time.sleep(0.2)    
        timecount = 0
        while True:
            if m2t.get_sentstatus() == False:
                print('0x%02X: ' % read_offset, end=''),
                print(m2t.playload_read().decode()[13:]) 
                file.write('%04X: ' % read_offset)
                file.write(str(m2t.playload_read().decode()[14:-1]) + "\n")
                fru_data.extend(str2list(m2t.playload_read().decode()[14:-1]))                                   
                break
            timecount = timecount + 1
            time.sleep(0.5)
            if(timecount >= 60):
                timecount = 0
                print('read time out ..')
                break
    m2t.pub_pmbus_set('[AA 02 01]')
    time.sleep(0.1)
    print('checksum: 0x%02X' % fru_checksun(fru_data))
    file.write('Calculate Checksum: 0x%02X' % fru_checksun(fru_data) + "\n")
    file.write(" \n")

def main():
    m2t.mqtt_init_start()
    m2t.sub('pmbus/info/eread')
    time.sleep(1)
    fru_read()
    m2t.mqtt_disconnect()

if __name__ == '__main__':
    main()