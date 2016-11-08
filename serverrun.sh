#!/bin/bash

### CONFIG ###
PORT=8808
##############

fuser -k $PORT/tcp
./serverThreaded $PORT
