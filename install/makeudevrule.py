#!/usr/bin/env python3

# 
# This file is part of batguard.
#
# batguard is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# batguard is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

import os.path
import subprocess
import json
import sys

print ('This script will learn which USB port is used for the relay so that only it can be used')
print ('1. Plug-in ONLY the USB-Relay into the USB port permanently dedicated to it')
print ('2. Remove all USB serial devices from the PC')

_ = input ('Press ENTER when done')

count = 0
while count < 3:
    count += 1
    
    if not os.path.exists ('/dev/ttyUSB0') : 
        print ('USB relay not found, ensure the relay is plugged in')
        _ = input ('Press ENTER when done')    
        continue
    
    if os.path.exists ('/dev/ttyUSB1'):
        print ('Too many USB serial devices linked to the computer, remove all other than the relay')
        _ = input ('Press ENTER when done')
        continue
        
    break
    
else: sys.exit (1)

data = []

try:
    relayjson = subprocess.run (['udevadm', 'info', '--no-pager', '--json=short', '-n', 'ttyUSB0'], capture_output = True, text = True, check = True)
except subprocess.CalledProcessError as e:
    print('Error trying to read the information of /dev/ttyUSB0')
    sys.exit (1)

relaydict = json.loads (relayjson.stdout)

if 'usb' not in relaydict['ID_MODEL'].lower () or 'serial' not in  relaydict['ID_MODEL'].lower ():
    print (f'Error the connected device does not look to be a relay, the ID model is {relaydict.ID_MODEL}')
    sys.exit (1)

if relaydict['ID_VENDOR'] != '1a86' :
    print (f'Warning, the vendor {relaydict['ID_VENDOR']} does not match the expected value (1d86), the rule will be created with the given vendor')

if relaydict['ID_MODEL_ID'] != '7523' :
    print (f'Warning, the model id {relaydict['ID_MODEL_ID']} does not match the expected value (7523), the rule will be created with the given model')

for p in reversed (relaydict['DEVPATH'].split ('/')) :
    if p [0].isalpha () : continue
    kernels = p.split (':')[0]
    break

with open ('90-batguard-relay.rules', 'w') as rule :
    rule.write (f'KERNEL=="ttyUSB*", KERNELS=="{kernels}", ATTRS{{idVendor}}=="{relaydict["ID_VENDOR_ID"]}", ATTRS{{idProduct}}=="{relaydict["ID_MODEL_ID"]}", SYMLINK+="ttyRELAY0"\n')

print ('Rule created correctly')
sys.exit (0)
