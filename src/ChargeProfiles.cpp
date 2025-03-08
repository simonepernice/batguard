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
 
#include "ChargeProfiles.hpp"
#include <algorithm>
#include <stdexcept>

ChargeProfile::ChargeProfile (const std::string& n, uint8_t min, uint8_t max, bool st) :
    name        {n},
    minCharge   {min},
    maxCharge   {max},
    startState  {st}
{
        if (min >= max) throw std::invalid_argument ("The minimum charge value shall be lower than the maximum instead they were found min: " + std::to_string (min) + ", max: " + std::to_string (max)); 
        if (min > 100) throw std::invalid_argument ("The minimum charge value cannot exceed 100 instead it was found min: " + std::to_string (min)); 
        if (max > 100) throw std::invalid_argument ("The maximum charge value cannot exceed 100 instead it was found max: " + std::to_string (max)); 
}

std::string ChargeProfile::toString () const
{
    return "(name: " + name + ", min charge: " + std::to_string (minCharge) + ", max charge: " + std::to_string (maxCharge) + ", start state: " + (startState ? "on)" : "off)");
}

bool ChargeProfile::operator == (const std::string& n) const 
{ 
    return name == n;
} 

bool ChargeProfile::operator == (const ChargeProfile& cp) const 
{ 
    return (name == cp.name) or (minCharge == cp.minCharge and maxCharge == cp.maxCharge);
} 

ChargeProfiles::ChargeProfiles ()
{
}

void ChargeProfiles::addProfile (const ChargeProfile& cp)
{
        if (profiles.size () == SIZE_MAX) throw std::invalid_argument ("Too many profiles were added, batguard supports up to " + std::to_string (SIZE_MAX) + " profiles");
        
        const auto& dup = std::find (profiles.begin (), profiles.end (), cp);
        if (dup != profiles.end ()) throw std::invalid_argument ("A charge profile with the same name and/or thresholds of the charge " + cp.toString () + " already exists as: " + dup->toString());
                
        profiles.push_back (cp);
}

const ChargeProfile* ChargeProfiles::getProfileWithName (const std::string& n) const
{
    auto ind = std::find (profiles.begin (), profiles.end (), n);
    if (ind == profiles.end ()) return nullptr;
    return & (*ind);
}

const ChargeProfile* ChargeProfiles::getProfileWithIndex (size_t index) const
{
    if (index >= profiles.size ()) return nullptr;
    return & (profiles [index]);
}

std::string ChargeProfiles::toString () const
{
    std::string cprofs;
    std::for_each (profiles.begin(), profiles.end(), [&](const ChargeProfile &cp){ cprofs += cp.toString () + '\n'; } );
    return cprofs;
}

size_t ChargeProfiles::numberOfProfiles () const
{
    return profiles.size ();
}
