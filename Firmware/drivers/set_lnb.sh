#!/bin/bash

if [ $1 = 0 ];then
	echo "Turning LNB Off"
	raspi-gpio set 3 op
	raspi-gpio set 3 dl
fi

if [ $1 = 1 ];then
	echo "Turning LNB On"
	raspi-gpio set 3 op
	raspi-gpio set 3 dh
fi

if [ $2 = 0 ];then
	echo "Switching to H/CL Polarization"
	raspi-gpio set 2 op
	raspi-gpio set 2 dl
fi

if [ $2 = 1 ];then
	echo "Switching to V/CR Polarization"
	raspi-gpio set 2 op
	raspi-gpio set 2 dh
fi
