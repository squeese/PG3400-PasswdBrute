#!/bin/bash
nodemon -L -e c,h -w source -x 'clear && make devrun || exit 1'
