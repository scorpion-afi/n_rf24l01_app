#!/bin/bash

odroid_ip=10.42.0.116

app_name=spidev_test
app_sources="${app_name}.c init_n_rf24l01.c ../src/n_rf24l01_lib/src/n_rf24l01.c"

echo "reload ${app_name} app and it sources."

echo -n "remove ${app_name} directory..."
ssh odroid@${odroid_ip} "rm -rf ${app_name}" && echo "ok." || echo "fail." 

echo -n "create ${app_name} directory..."
ssh odroid@${odroid_ip} "mkdir ${app_name}" && echo "ok." || echo "fail." 

echo -n "copy ${app_name} to odroid..."
scp ${app_name} odroid@${odroid_ip}:/home/odroid/${app_name} && echo "ok." || echo "fail." 

# copy app's sources to use them with gdb
echo -n "copy ${app_sources} to odroid..."
scp ${app_sources} odroid@${odroid_ip}:/home/odroid/${app_name} && echo "ok." || echo "fail." 
