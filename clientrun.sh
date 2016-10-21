#!/bin/bash

### CONFIG ###
ADDRESS=127.0.0.1
PORT=1234
##############

make
clear
./client $ADDRESS $PORT
