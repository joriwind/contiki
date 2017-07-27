log=size.log

echo "New measurement: $1" >> $log
date >> $log
msp430-size $2 >> $log
echo "" >> $log
