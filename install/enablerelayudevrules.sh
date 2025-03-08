#!/bin/bash
echo "enabling the relay udev rules..."
udevadm control --reload 
udevadm trigger
echo
echo "batguard relay enabled"
echo
