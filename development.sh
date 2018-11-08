#!/bin/bash
# clear
ARGS="-t 8 -b 512"
H1="\$1\$9779ofJE\$c.p.EwsI57yV2xjeorQbs1"                                 # 'Hi' permutation
# ./client $ARGS $H1 -l 2 -I a-z -I A-Z                                    # success

# ./client $ARGS $H1 -s 192.168.1.1:3000 -s :2000 -s /tmp/someplace -l 1 -i a

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

H4="\$1\$K4BfHkEl\$A1mF1S2ztQ7reX7GzWj7v0"                                 # 'How' permutation
# ./client $ARGS $H4 -l 3 -I a-z -I A-Z                                    # success

H5="\$1\$RvQQ2SJN\$Q80Nh4Ello9cx9Wllf5Nx/"                                 # 'about' dictionary
# ./client $ARGS $H5 -d misc/dictionary.txt                                # success

XX="\$1\$RvQQ2SJN\$Q80Nh4Ell_9cx9Wllf5Nx/"                                 # 'about' dictionary
#./client $ARGS $XX -d misc/small.txt -l 4
#./client $ARGS $XX -d misc/small.txt -d misc/dictionary.txt
./client $ARGS "\$1\$RvQQ2SJN\$Q80Nh4Ello9cx9Wllf5Nx/" -d misc/dictionary.txt    # about

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
# error in threads

# ./client $ARGS $H3 -l 1 \                                                # failure
#   -d misc/singles.org.txt \
#   -d misc/singles.org-withcount.txt \
#   -d misc/SplashData-2014.txt \
#   -d misc/SplashData-2015-1.txt \
#   -d misc/SplashData-2015-2.txt \
#   -d misc/ssh-betterdefaultpasslist.txt \
#   -d misc/Sucuri-Top-Wordpress-Passwords.txt \
#   -d misc/telnet-betterdefaultpasslist.txt \
#   -d misc/tomcat-betterdefaultpasslist.txt \
#   -d misc/top-20-common-SSH-passwords.txt \
#   -d misc/top-passwords-shortlist.txt \
#   -d misc/tuscl.txt \
#   -d misc/twitter-banned.txt \
#   -d misc/unkown-azul.txt \
#   -d misc/UserPassCombo-Jay.txt \
#   -d misc/vnc-betterdefaultpasslist.txt \
#   -d misc/windows-betterdefaultpasslist.txt \
#   -d misc/wordpress-attacks-july2014.txt \
#   -d misc/words.txt \
#   -d misc/worst-passwords-2017-top100-slashdata.txt \
#   -d misc/milw0rm-dictionary.txt \
#   -d misc/mirai-botnet.txt \
#   -d misc/Most-Popular-Letter-Passes.txt \
#   -d misc/mssql-betterdefaultpasslist.txt \
#   -d misc/multiplesources-passwords-fabian-fingerle.de.txt \
#   -d misc/muslimMatch.txt \
#   -d misc/dictionary.txt \
#   -d misc/muslimMatch-withCount.txt \
#   -d misc/000webhost.txt \
#   -d misc/10k-most-common.txt \
#   -d misc/10-million-password-list-top-1000000.txt \
#   -d misc/10-million-password-list-top-100000.txt \
#   -d misc/10-million-password-list-top-10000.txt \
#   -d misc/10-million-password-list-top-1000.txt \
#   -d misc/10-million-password-list-top-100.txt \
#   -d misc/10-million-password-list-top-500.txt \
#   -d misc/1337speak.txt \
#   -d misc/500-worst-passwords.txt \
#   -d misc/adobe100.txt \
#   -d misc/alleged-gmail-passwords.txt \
#   -d misc/Ashley-Madison.txt \
#   -d misc/best1050.txt \
#   -d misc/best110.txt \
#   -d misc/best15.txt \
#   -d misc/bible.txt \
#   -d misc/bible-withcount.txt \
#   -d misc/bt4-password.txt \
#   -d misc/cain-and-abel.txt \
#   -d misc/carders.cc.txt \
#   -d misc/cirt-default-passwords.txt \
#   -d misc/clarkson-university-82.txt \
#   -d misc/common-passwords-win.txt \
#   -d misc/conficker.txt \
#   -d misc/darkc0de.txt \
#   -d misc/darkweb2017-top10000.txt \
#   -d misc/darkweb2017-top1000.txt \
#   -d misc/darkweb2017-top100.txt \
#   -d misc/darkweb2017-top10.txt \
#   -d misc/db2-betterdefaultpasslist.txt \
#   -d misc/elitehacker.txt \
#   -d misc/elitehacker-withcount.txt \
#   -d misc/faithwriters.txt \
#   -d misc/faithwriters-withcount.txt \
#   -d misc/ftp-betterdefaultpasslist.txt \
#   -d misc/hak5.txt \
#   -d misc/hak5-withcount.txt \
#   -d misc/honeynet2.txt \
#   -d misc/honeynet.txt \
#   -d misc/honeynet-withcount.txt \
#   -d misc/hotmail.txt \
#   -d misc/izmy.txt \
#   -d misc/john-the-ripper.txt \
#   -d misc/Keyboard-Combinations.txt \
#   -d misc/korelogic-password.txt \
#   -d misc/Lizard-Squad.txt \
#   -d misc/md5decryptor.uk.txt \
#   -d misc/medical-devices.txt \
#   -d misc/myspace.txt \
#   -d misc/myspace-withcount.txt \
#   -d misc/mysql-betterdefaultpasslist.txt \
#   -d misc/openwall.net-all.txt \
#   -d misc/oracle-betterdefaultpasslist.txt \
#   -d misc/password-permutations.txt \
#   -d misc/passwords-youporn2012-raw.txt \
#   -d misc/passwords-youporn2012.txt \
#   -d misc/phpbb-nocount.txt \
#   -d misc/phpbb.txt \
#   -d misc/phpbb-withcount.txt \
#   -d misc/PHP-Magic-Hashes.txt \
#   -d misc/porn-unknown.txt \
#   -d misc/porn-unknown-withcount.txt \
#   -d misc/postgres-betterdefaultpasslist.txt \
#   -d misc/probable-v2-top12000.txt \
#   -d misc/probable-v2-top1575.txt \
#   -d misc/probable-v2-top207.txt \
#   -d misc/probable-v2-wpa-top447.txt \
#   -d misc/probable-v2-wpa-top4800.txt \
#   -d misc/probable-v2-wpa-top62.txt \
#   -d misc/rockyou-05.txt \
#   -d misc/rockyou-10.txt \
#   -d misc/rockyou-15.txt \
#   -d misc/rockyou-20.txt \
#   -d misc/rockyou-25.txt \
#   -d misc/rockyou-30.txt \
#   -d misc/rockyou-35.txt \
#   -d misc/rockyou-40.txt \
#   -d misc/rockyou-45.txt \
#   -d misc/rockyou-50.txt \
#   -d misc/rockyou-55.txt \
#   -d misc/rockyou-60.txt \
#   -d misc/rockyou-65.txt \
#   -d misc/rockyou-70.txt \
#   -d misc/rockyou-75.txt \
#   -d misc/rockyou.txt \