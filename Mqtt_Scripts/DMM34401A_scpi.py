from MQTT2Twi_scpi import MQTT2Twi
import time

m2t = MQTT2Twi("192.168.200.2", "npi/")

def dmm_read_raw():
    return m2t.scpi_com_send('READ?')

def dmm_read_float():
    return float(m2t.scpi_com_send('READ?'))

def dmm_34401A_init():
    m2t.scpi_com_send('*rst')
    time.sleep(0.1)
    m2t.scpi_com_send('*cls')
    time.sleep(0.5)
    if m2t.scpi_com_send('*idn?').find("34401A") > 0:
        time.sleep(0.1)
        m2t.scpi_com_send('SYST:REM')
        return 1
    else:
        return 0

def main():
    m2t.mqtt_init_start()
    m2t.sub('scpi/readback')
    time.sleep(1)
    if(dmm_34401A_init()):
        m2t.scpi_com_send('CONF:RES AUTO')
        time.sleep(0.5)
        print(dmm_read_float())
    else:
        print('DMM 34401A NOT FOUND!')
    # print(float(m2t.scpi_com_send('MEAS:VOLT:DC? 10,0.003')))
    time.sleep(1)
    m2t.mqtt_disconnect()

if __name__ == '__main__':
    main()