from MQTT2Twi_scpi import MQTT2Twi
import time

m2t = MQTT2Twi("192.168.200.2", "npib/")

def source_4500il_init():
    m2t.scpi_com_send('*rst')
    time.sleep(0.1)
    m2t.scpi_com_send('*cls')
    time.sleep(0.5)
    if m2t.scpi_com_send('*idn?').find("4500IL") > 0:
        time.sleep(0.1)
        m2t.scpi_com_send('SYST:REM')
        return 1
    else:
        return 0

def dropout():
    m2t.scpi_com_send('*rst')
    m2t.scpi_com_send('*cls')
    m2t.scpi_com_send('curr max')
    val = input("input dropout volt: ")
    m2t.scpi_com_send('volt {:f}'.format(float(val)))
    val = input("input dropout trig volt: ")
    m2t.scpi_com_send('volt:trig {:f}'.format(float(val)))
    m2t.scpi_com_send('volt:mode puls')
    val = input("input delay time ms:")
    m2t.scpi_com_send('puls:widt {:f}ms'.format(float(val)))
    val = input("input trig phas deg: ")
    m2t.scpi_com_send('trig:sync:phas {:f}'.format(float(val)))
    m2t.scpi_com_send('init:cont:seq on')
    m2t.scpi_com_send('output on')
    while True:
        sta = str(input("press 'T' to Trigger dorpout test; 'M' to Exit: "))
        if (sta == 'T' or sta == 't'):
            m2t.scpi_com_send('trig')
            print('Trigger Sent ')
            time.sleep(1)
        if (sta == 'M' or sta == 'm'):
            print('Exit dropout test')
            break
    time.sleep(1)
    m2t.scpi_com_send('output off')

def main():
    m2t.mqtt_init_start()
    m2t.sub('scpi/readback')
    time.sleep(1)
    if(source_4500il_init()):
        dropout()
    else:
        print('SOURCE 4500IL NOT FOUND!')
    time.sleep(1)
    m2t.mqtt_disconnect()

if __name__ == '__main__':
    main()