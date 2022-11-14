#!/bin/bash

count=0
for i in $(seq 15)
do
    let count++ 
    echo "echo $count > /sys/class/7segment_display/decimal_value"
    sleep 1 
done
