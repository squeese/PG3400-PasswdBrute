#!/bin/bash
nodemon -L -e c,h -w source -w Makefile -x 'clear && make devrun || exit 1'
