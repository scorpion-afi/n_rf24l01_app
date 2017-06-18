#!/bin/bash


app_name=event_fd
app_sources="src/spidev_test.cpp"

echo "reload ${app_name} app and it sources."

echo -n "remove ${app_name} directory..."
ssh odroid@${ODROID_IP} "rm -rf ${app_name}" && echo "ok." || echo "fail." 

echo -n "create ${app_name} directory..."
ssh odroid@${ODROID_IP} "mkdir ${app_name}" && echo "ok." || echo "fail." 

echo -n "copy ${app_name} to odroid..."
scp ${app_name} odroid@${ODROID_IP}:/home/odroid/${app_name} && echo "ok." || echo "fail." 

# copy app's sources to use them with gdb
echo -n "copy ${app_sources} to odroid..."
scp ${app_sources} odroid@${ODROID_IP}:/home/odroid/${app_name} && echo "ok." || echo "fail." 
