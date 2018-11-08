#!/bin/bash
ARGS="-t 8 -b 512"
make clean
make client
clear
./client $ARGS "\$1\$9779ofJE\$c.p.EwsI57yV2xjeorQbs1" -l 2 -I a-z -I A-Z        # Hi
./client $ARGS "\$1\$7tBjugEa\$h3cZLWYTXCwqikbFvQe7A/" -d misc/dictionary.txt    # Great
# hash 3 missing
./client $ARGS "\$1\$K4BfHkEl\$A1mF1S2ztQ7reX7GzWj7v0" -l 3 -I a-z -I A-Z        # How
./client $ARGS "\$1\$RvQQ2SJN\$Q80Nh4Ello9cx9Wllf5Nx/" -d misc/dictionary.txt    # about
./client $ARGS "\$1\$CPqVGNrg\$YH26ye4.Cft6c9AWf0zUn1" -d misc/dictionary.txt    # something
# hash 7 missing
./client $ARGS "\$1\$jgE1qlB8\$l8.Oicr89Ib8DPhV/8nZp1" -d misc/rockyou.txt       # pa55w0rd1
./client $ARGS "\$1\$ckvWM6T@\$H6H/R5d4a/QjpB02Ri/V01" -d misc/rockyou.txt       # Congratulations
