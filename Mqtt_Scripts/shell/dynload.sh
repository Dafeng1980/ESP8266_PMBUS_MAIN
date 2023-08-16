#!/bin/bash
# SCPI Dynamic Load Seting
#$1 $2 Set dynamic L1 and L2 load parameter (A)
#$3 $4 Set dynamic rise and fall slew rate (A/us)
#$5 $6 Set dynamic duration time (ms)
# History:
# 2022/5/24     Dafeng  First release
PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin:~/bin
export PATH

ll=${1:-'5'}
hl=${2:-'10'}
rt=${3:-'1'}
ft=${4:-'1'}
t1=${5:-'10'}
t2=${6:-'10'}

TOPIC="rrh/pmbus/set"
USER="dfiot"
PASSW="123abc"

mosquitto_pub -t ${TOPIC}  -u ${USER} -P ${PASSW} -m "%*rst%"
sleep 0.02
mosquitto_pub -t ${TOPIC}  -u ${USER} -P ${PASSW} -m "%mode 3%"
sleep 0.02
mosquitto_pub -t ${TOPIC}  -u ${USER} -P ${PASSW} -m "%curr:dyn:l1 ${ll}%"
sleep 0.02
mosquitto_pub -t ${TOPIC}  -u ${USER} -P ${PASSW} -m "%curr:dyn:l2 ${hl}%"
sleep 0.02
mosquitto_pub -t ${TOPIC}  -u ${USER} -P ${PASSW} -m "%curr:dyn:rise ${rt}%"
sleep 0.02
mosquitto_pub -t ${TOPIC}  -u ${USER} -P ${PASSW} -m "%curr:dyn:fall ${ft}%"
sleep 0.02
mosquitto_pub -t ${TOPIC}  -u ${USER} -P ${PASSW} -m "%curr:dyn:t1 ${t1}ms%"
sleep 0.02
mosquitto_pub -t ${TOPIC}  -u ${USER} -P ${PASSW} -m "%curr:dyn:t2 ${t2}ms%"
sleep 0.02
mosquitto_pub -t ${TOPIC}  -u ${USER} -P ${PASSW} -m "%load 1%"
