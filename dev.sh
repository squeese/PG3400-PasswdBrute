#!/bin/bash
OPT=${1:-dev}
make clean
nodemon -e c,h -w source -w Makefile -w "cron.sh" -L -x "clear && make $OPT || exit 1"
