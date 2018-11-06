#!/bin/bash
ARGS="-t 12 -b 1024"

clear

H1="\$1\$9779ofJE\$c.p.EwsI57yV2xjeorQbs1"                                 # 'Hi' permutation
# ./client $ARGS $H1 -l 2 -I a-z -I A-Z                                    # success

H2="\$1\$7tBjugEa\$h3cZLWYTXCwqikbFvQe7A/"                                 # 'Great' dictionary
# ./client $ARGS $H2 -d misc/dictionary.txt                                # success

H3="\$1\$ivGZShhu\$L/NLEkmbLWOSUTOm3cnmO/"                                 # ???
# ./client $ARGS $H3 -d misc/dictionary.txt -l 4 -I a-z -I A-Z -I 0-9      # failure
# ./client $ARGS $H3 -l 5 -I a-z                                           # failure
# ./client $ARGS $H3 -L 1-6 -I 0-9                                         # failure
# ./client $ARGS $H3 -L 1-5 -I A-Z #                                       # failure
# ./client $ARGS $H3 -L 1-3 -C 32-126                                      # failure
# ./client $ARGS $H3 -L 1-8 -I 0-4                                         # failure
# ./client $ARGS $H3 -L 1-8 -I 1-5                                         # failure
# ./client $ARGS $H3 -L 1-8 -I 2-6                                         # failure
# ./client $ARGS $H3 -L 1-8 -I 3-7                                         # failure
# ./client $ARGS $H3 -L 1-8 -I 4-8                                         # failure
# ./client $ARGS $H3 -L 1-8 -I 5-9                                         # failure
# ./client $ARGS $H3 -d misc/rockyou.txt                                   # failure
# ./client $ARGS $H3 -d misc/words.txt -l 1                                # failure
# ./client $ARGS $H3 -l 1 \                                                # failure
#   -d ./../SecLists/Passwords/cirt-default-passwords.txt \
#   -d ./../SecLists/Passwords/clarkson-university-82.txt \
#   -d ./../SecLists/Passwords/darkc0de.txt \
#   -d ./../SecLists/Passwords/darkweb2017-top10000.txt \
#   -d ./../SecLists/Passwords/darkweb2017-top1000.txt \
#   -d ./../SecLists/Passwords/darkweb2017-top100.txt \
#   -d ./../SecLists/Passwords/darkweb2017-top10.txt \
#   -d ./../SecLists/Passwords/Keyboard-Combinations.txt \
#   -d ./../SecLists/Passwords/openwall.net-all.txt \
#   -d ./../SecLists/Passwords/PHP-Magic-Hashes.txt \
#   -d ./../SecLists/Passwords/probable-v2-top12000.txt \
#   -d ./../SecLists/Passwords/probable-v2-top1575.txt \
#   -d ./../SecLists/Passwords/probable-v2-top207.txt \
#   -d ./../SecLists/Passwords/twitter-banned.txt \
#   -d ./../SecLists/Passwords/unkown-azul.txt \
#   -d ./../SecLists/Passwords/UserPassCombo-Jay.txt

H4="\$1\$K4BfHkEl\$A1mF1S2ztQ7reX7GzWj7v0"                                 # 'How' permutation
# ./client $ARGS $H4 -l 3 -I a-z -I A-Z                                    # success

H5="\$1\$RvQQ2SJN\$Q80Nh4Ello9cx9Wllf5Nx/"                                 # 'about' dictionary
# ./client $ARGS $H5 -d misc/dictionary.txt                                # success

H6="\$1\$CPqVGNrg\$YH26ye4.Cft6c9AWf0zUn1"                                 # 'something' dictionary
# ./client $ARGS $H6 -d misc/dictionary.txt                                # success

H7="\$1\$btfQSNEr\$alX1tFUIDtW7bOfdDN2IK1"                                 # ???
# ./client $ARGS $H7 -d misc/dictionary.txt -L 1-2 -C 32-126               # failure
# ./client $ARGS $H7 -l 3 -C 32-126                                        # failure
# ./client $ARGS $H7 -l 5 -I a-z                                           # failure
# ./client $ARGS $H7 -L 1-6 -I 0-9                                         # failure
# ./client $ARGS $H7 -L 1-5 -I A-Z                                         # failure
# ./client $ARGS $H7 -d misc/words.txt -l 1                                # failure
# ./client $ARGS $H7 -d misc/rockyou.txt -l 1                              # failure
# ./client $ARGS $H7 -l 6 -i hardeHARDE                                    # failure
# ./client $ARGS $H7 -l 6 -i longerLONGER                                  # failure
# ./client $ARGS $H7 -d misc/custom.txt -l 1                               # failure
# ./client $ARGS $H7 -l 1 \                                                # failure
#  -d ./../SecLists/Passwords/bt4-password.txt \
#  -d ./../SecLists/Passwords/cirt-default-passwords.txt \
#  -d ./../SecLists/Passwords/clarkson-university-82.txt \
#  -d ./../SecLists/Passwords/darkc0de.txt \
#  -d ./../SecLists/Passwords/darkweb2017-top10000.txt \
#  -d ./../SecLists/Passwords/darkweb2017-top1000.txt \
#  -d ./../SecLists/Passwords/darkweb2017-top100.txt \
#  -d ./../SecLists/Passwords/darkweb2017-top10.txt \
#  -d ./../SecLists/Passwords/Keyboard-Combinations.txt \
#  -d ./../SecLists/Passwords/openwall.net-all.txt \
#  -d ./../SecLists/Passwords/PHP-Magic-Hashes.txt \
#  -d ./../SecLists/Passwords/probable-v2-top12000.txt \
#  -d ./../SecLists/Passwords/probable-v2-top1575.txt \
#  -d ./../SecLists/Passwords/probable-v2-top207.txt \
#  -d ./../SecLists/Passwords/twitter-banned.txt \
#  -d ./../SecLists/Passwords/unkown-azul.txt \
#  -d ./../SecLists/Passwords/UserPassCombo-Jay.txt

H8="\$1\$jgE1qlB8\$l8.Oicr89Ib8DPhV/8nZp1"                                 # 'pa55w0rd1' rockyou
# ./client $ARGS $H8 -d misc/dictionary.txt -l 2 -C 32-126                 # failure
# ./client $ARGS $H8 -L 1-4 -I a-z -I A-Z                                  # failure
# ./client $ARGS $H8 -d misc/words.txt -l 1                                # failure
# /client $ARGS $H8 -d misc/rockyou.txt                                    # success

H9="\$1\$ckvWM6T@\$H6H/R5d4a/QjpB02Ri/V01"                                 # 'Congratulations' rockyou
# ./client $ARGS $H9 -d misc/dictionary.txt -l 2 -C 32-126                 # failure
# ./client $ARGS $H9 -L 1-4 -I a-z -I A-Z                                  # failure
# ./client $ARGS $H9 -d misc/words.txt -l 1                                # failure
# ./client $ARGS $H9 -d misc/rockyou.txt                                   # success

# server
# multiple dictionaries, defaults to none
# skip triple equal letters in permutation -S
# make sure every line in a file is being able to be matched?
# same for permutations?
# documentation in args
# open directory segmentation fault