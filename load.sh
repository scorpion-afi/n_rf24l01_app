#!/bin/bash

binary_name=n_rf24l01_test
sources="main.cpp"

echo "(re)load ${binary_name} binary and it sources..."

ssh odroid@${ODROID_IP} "mkdir -p ${ODROID_N_RF_DIR}/${binary_name}" 
scp ${binary_name} odroid@${ODROID_IP}:${ODROID_N_RF_DIR}/${binary_name} 
scp ${sources} odroid@${ODROID_IP}:${ODROID_N_RF_DIR}/${binary_name} 

binary_name=util
sources="util.cpp"

echo "(re)load ${binary_name} binary and it sources..."

ssh odroid@${ODROID_IP} "mkdir -p ${ODROID_N_RF_DIR}" 
scp ${binary_name} odroid@${ODROID_IP}:${ODROID_N_RF_DIR}
scp ${sources} odroid@${ODROID_IP}:${ODROID_N_RF_DIR}

echo "ok"
