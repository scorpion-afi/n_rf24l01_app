This directory contains spidev_test program's sources.
spidev_test is user space program used spidev linux kernel driver to comunicate with n_rf24l01 transceiver, via n_rf24l01.c/n_rf24l01.h library.
It is test application to test spidev driver and simple work with n_rf24l01 transceiver. spidev_test programs n_rf24l01 to be transmitter and 
transmit one byte (character from string "hello world!!!") per second.

Also here are sources of app test that also tests some init work with n_rf24l01 transceiver.