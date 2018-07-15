This directory contains n_rf24l01_test program's sources.

n_rf24l01_test is a user space program (Linux only) which tests a libn_rf24l01 library and an n_rf24l01 transceiver.
n_rf24l01_test initializes the libn_rf24l01 library and then waits for input from a user and/or a data
from the transceiver. If there's some data on an input stream, program forwards them to the transceiver, if there's some
data on the transceiver program prints them out.
