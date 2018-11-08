#!/bin/bash
OPT=${1:-development}
make clean
nodemon -e c,h -w source -w Makefile -w "development.sh" -L -x "clear && make $OPT || exit 1"
