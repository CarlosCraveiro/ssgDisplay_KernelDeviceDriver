#!/bin/bash

count=0
for i in $(seq 15)
do
    let count++ 
    echo $count > /proc/my-led
    sleep 1 
done
