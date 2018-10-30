#!/bin/bash
OPT=${1:-dev}
make clean
if [ "$1" = "server" ]
then
  nodemon -e c,h -w source -w Makefile -L -x "clear && rm -f .port && make dev_server || exit 1"
elif [ "$1" = "client" ]
then
  nodemon -e c,h -w source -w Makefile -L -x "clear && make dev_client || exit 1"
else
  nodemon -e c,h -w source -w Makefile -L -x "clear && make $OPT || exit 1"
fi
