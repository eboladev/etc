#!/bin/sh
#
# bragdaemon.sh - Run brag continously; get command line options 
# from a config file. Reconnect on network errors.
#

idle=3600
retryOnError=3
config=~/brag/config

if [ ! -f $config ]; then
  echo "Missing configuration file ~/.brag/config"
  exit 1
fi

while true; do
  cnt=0
  while true; do
    brag `cat $config`
    if [ $? != 3 ]; then
      break
    fi
    cnt=`expr $cnt + 1`
    if [ $cnt = $retryOnError ]; then
      break
    fi
  done
  sleep $idle
done

