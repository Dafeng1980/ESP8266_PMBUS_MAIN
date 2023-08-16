from MQTT2Twi_scpi import MQTT2Twi
import time

m2t = MQTT2Twi("192.168.200.2", "npi/")

def main():
    m2t.mqtt_init_start()
    m2t.sub('scpi/readback')
    time.sleep(1)
    m2t.scpi_com_send('*rst')
    m2t.scpi_com_send('*cls')
    m2t.scpi_com_send('SYST:REM')
    print(m2t.scpi_com_send('*idn?'))
    time.sleep(1)
    m2t.mqtt_disconnect()

if __name__ == '__main__':
    main()