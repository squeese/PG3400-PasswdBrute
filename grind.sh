#!/bin/bash
FLAGS="--tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes"
clear && \
  make client && \
  valgrind $FLAGS ./client -d "misc/custom.txt" -d "misc/custom.txt" -L 1-3 -I 0-9 \$1\$9779ofJE\$MKAskbSv72cuWHNmBHTwX:
 # -d some/path.txt
