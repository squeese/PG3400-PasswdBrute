#!/bin/bash
FLAGS="--tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes"
clear && \
  make client && \
  # valgrind $FLAGS ./client -d "misc/small.txt" \$1\$RvQQ2SJN\$Q80Nh4Ello9cx9Wllf5Nx/
  valgrind $FLAGS ./client -t 2 -d "misc/small.txt" \$1\$RvQQ2SJN\$Q80Nh4Ello9c_9Wllf5Nx/ # incorrect hash