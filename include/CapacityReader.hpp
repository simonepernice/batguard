/*
 * batguard a laptop battery charge manager
 * 
 * Copyright (C) 2025 Simone Pernice pernice@libero.it 
 * 
 * This file is part of batguard.
 *
 * batguard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * batguard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */
 
#ifndef CAPACITYREADER_H
#define CAPACITYREADER_H

#include <string>
#include <cstdint>

class CapacityReader
{
    public:
        //Create a CapacityReader, requires the path to the battery
        //It is usually: /sys/class/power_supply/BAT0/capacity
        explicit            CapacityReader (const std::string& path);
        
        //returns the capacity from 0 to 100 as integer
        uint8_t             readCapacity ();
        
        //returns the charge difference between last two readings
        int                 deltaCapacity () const;
        
        //returns a string representing the current battery charge
        std::string         toString () const;
        
    private:
        const std::string   capacityPath;
        int                 prevCapacity;
        int                 currCapacity;
        
        void                computeCapacity ();
        
};

#endif //CAPACITYREADER_H
