#!/bin/bash

MOUNT=/mnt/sentri-auto-wifi-config
FILE=$MOUNT/sentri-wifi-config

mkdir -p $MOUNT
mount $DEVNAME $MOUNT

if [ -e $FILE ]; then
  SSID=$(sed -n '1p' < $FILE)
  PW=$(sed -n '2p' < $FILE)
  #set up wifi
  #check if a connection with this SSID already exists, if so, delete it
  if [ -n "$(nmcli connection | grep $SSID)" ]; then
    nmcli connection delete $SSID
  fi
  nmcli device wifi rescan
  #return feedback to .result file on given block device
  echo $(nmcli device wifi connect $SSID password $PW) > $FILE.result
  umount $MOUNT
  rm -rf $MOUNT
fi
