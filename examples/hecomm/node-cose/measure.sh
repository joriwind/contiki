log=size_v2.log

echo "New measurement: $1" >> $log
date >> $log
msp430-size node-cose.z1 >> $log
echo "" >> $log
