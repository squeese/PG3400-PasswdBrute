#!/bin/bash
OPT=${1:-1}
LEN=${2:-3}
DIC=${3:-misc/dictionary.txt}
clear
make clean
make client
if [ "$OPT" = "1" ]
then
  ./client -l $LEN -d $DIC \$1\$9779ofJE\$c.p.EwsI57yV2xjeorQbs1
elif [ "$OPT" = "2" ]
then
  ./client -l $LEN -d $DIC \$1\$7tBjugEa\$h3cZLWYTXCwqikbFvQe7A/
elif [ "$OPT" = "3" ]
then
  ./client -l $LEN -d $DIC \$1\$ivGZShhu\$L/NLEkmbLWOSUTOm3cnmO/
elif [ "$OPT" = "4" ]
then
  ./client -l $LEN -d $DIC \$1\$K4BfHkEl\$A1mF1S2ztQ7reX7GzWj7v0
elif [ "$OPT" = "5" ]
then
  ./client -l $LEN -d $DIC \$1\$RvQQ2SJN\$Q80Nh4Ello9cx9Wllf5Nx/
elif [ "$OPT" = "6" ]
then
  ./client -l $LEN -d $DIC \$1\$CPqVGNrg\$YH26ye4.Cft6c9AWf0zUn1
elif [ "$OPT" = "7" ]
then
  ./client -l $LEN -d $DIC \$1\$btfQSNEr\$alX1tFUIDtW7bOfdDN2IK1
elif [ "$OPT" = "8" ]
then
  ./client -l $LEN -d $DIC \$1\$jgE1qlB8\$l8.Oicr89Ib8DPhV/8nZp1
elif [ "$OPT" = "9" ]
then
  ./client -l $LEN -d $DIC \$1\$ckvWM6T@\$H6H/R5d4a/QjpB02Ri/V01
elif [ "$OPT" = "10" ]
then
  ./client -l $LEN -d $DIC \$1\$ckvWM6T@\$b6VWGAz0QKG4CpU3YwJjW.
fi