#!/bin/bash
# SCPI AC line Dropout 0V Seting, AC Source California Instruments 4500iL 
#$1 Set  AC input (V)
#$2 Set Pusl Widt Time (ms)
#$3 Set Phas  (degree)
# History:
# 2022/7/18     Dafeng  First release
PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin:~/bin
export PATH

volt=${1:-'230'}
time=${2:-'10'}
phas=${3:-'0'}

TOPIC="npi/scpi/set"
USER="dfiot"
PASSW="123abc"

mosquitto_pub -t ${TOPIC}  -u ${USER} -P ${PASSW} -m "%*rst%"
sleep 0.02
echo "*RST"
mosquitto_pub -t ${TOPIC}  -u ${USER} -P ${PASSW} -m "%curr max%"
sleep 0.02
echo "CURR MAX"
mosquitto_pub -t ${TOPIC}  -u ${USER} -P ${PASSW} -m "%volt ${volt}%"
sleep 0.02
echo "VOLT" ${volt}
mosquitto_pub -t ${TOPIC}  -u ${USER} -P ${PASSW} -m "%volt:trig 0%"
sleep 0.02
echo "VOLT:TRIG 0"
mosquitto_pub -t ${TOPIC}  -u ${USER} -P ${PASSW} -m "%volt:mode puls%"
sleep 0.02
echo "VOLT:MODE PULS"
mosquitto_pub -t ${TOPIC}  -u ${USER} -P ${PASSW} -m "%puls:widt ${time}ms%"
sleep 0.02
echo "PULS:WIDT ${time}ms"
mosquitto_pub -t ${TOPIC}  -u ${USER} -P ${PASSW} -m "%trig:sync:phas ${phas}%"
sleep 0.02
echo "TRIG:SYNC:PHAS ${phas}"
mosquitto_pub -t ${TOPIC}  -u ${USER} -P ${PASSW} -m "%init:cont:seq on%"
sleep 0.02
echo "INIT:CONT:SEQ ON"
sleep 0.2
echo "AC Source 4500iL set Volt:${volt} drop time:${time} phas:${phas} Complete"