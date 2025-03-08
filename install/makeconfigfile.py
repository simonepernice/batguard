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
import sys


count = 0
while not os.path.exists (f'/sys/class/power_supply/BAT{count}/capacity') : 
    count += 1
    if count > 9 :
        print ('Not possible to find the path to the battery capacity file')
        sys.exit (1)

if count > 0:
    print ('Adding the battey capacity path to the configuration file because it is not the default one')
    with open ('config', 'a') as config:
        config.write ('\n')
        config.write ('#The following line is automatically added by the installation script\n')
        config.write (f'batterypath = /sys/class/power_supply/BAT{count}/capacity\n')
        config.write ('\n')
    print ('Configuration file updated correctly')
    sys.exit (0)
    
print ('There is not need to update the configuration file for the battery')
sys.exit (0)
