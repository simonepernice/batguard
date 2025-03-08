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
 
#include "CapacityReader.hpp"
#include <fstream>
#include <stdexcept>

CapacityReader::CapacityReader (const std::string& path) :
    capacityPath {path}
{
    //to rise an exception in case the file does not exists or is wrong and initialize current capacity  for the variation calculation
    computeCapacity ();
    if (currCapacity < 0 or currCapacity > 100) throw std::invalid_argument ("The given battery file capacity value is not between 0 and 100 at " + capacityPath);
}

uint8_t CapacityReader::readCapacity ()
{
    computeCapacity ();
    return static_cast<uint8_t> (currCapacity);
}

int CapacityReader::deltaCapacity () const
{
    return currCapacity - prevCapacity;
}


std::string CapacityReader::toString () const
{
    return "The current battery capacity is: " + std::to_string (currCapacity) + '%';
}

void CapacityReader::computeCapacity ()
{
    std::ifstream capacityfstream (capacityPath);
    
    if (not capacityfstream.good ()) throw std::invalid_argument ("It was not possible to open the battery capacity file " + capacityPath);
    
    std::string capacitystring;
    std::getline (capacityfstream, capacitystring);
    prevCapacity = currCapacity;
    try
    {
        currCapacity = std::stoi (capacitystring);
    }
    catch (...)
    {
        throw std::invalid_argument ("Was expected a file containing the battery capacity, instead the content is " + capacitystring);
    }
}
