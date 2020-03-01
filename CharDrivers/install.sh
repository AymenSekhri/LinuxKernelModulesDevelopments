#!/bin/sh
module="Main.ko"
device="myNewDevice"
majorNum=$(awk -F " "  '$2 == "myNewDevice" {print $1}' /proc/devices) # get major number

insmod $module
mknod /dev/${device}0 c $majorNum 0 
mknod /dev/${device}1 c $majorNum 1
mknod /dev/${device}2 c $majorNum 2
mknod /dev/${device}3 c $majorNum 3