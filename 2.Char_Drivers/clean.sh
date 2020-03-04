#!/bin/sh
module="Main.ko"
device="myCharDevice"

rmmod $module
rm /dev/${device}0
rm /dev/${device}1
rm /dev/${device}2
rm /dev/${device}3