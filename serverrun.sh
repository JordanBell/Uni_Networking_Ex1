#!/bin/bash

### CONFIG ###
PORT=1234
OUT_FILE=log.txt
##############

fuser -k $PORT/tcp # Kill the process on the port
make
clear
./serverSingle $PORT $OUT_FILE
