#!/bin/bash
OPT=${1:-dev}
make clean
nodemon -e c,h -w source -w Makefile -L -x "clear && sleep 1 && make $OPT || exit 1"
