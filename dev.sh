#!/bin/bash
make clean
nodemon -e c,h -w source -w Makefile -L -x 'clear && make devrun || exit 1'
