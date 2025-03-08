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
 
#ifndef CHARGEPROFILES_H
#define CHARGEPROFILES_H

#include <vector>
#include <string>
#include <cstdint>

struct ChargeProfile
{
    const std::string name;
    const uint8_t     minCharge;
    const uint8_t     maxCharge;
    const bool        startState;            

    //add a new charge profile with given name, min and max charge thresholds, start level
    //throw an exception if min or max are out of range or swapped
                    ChargeProfile (const std::string& n, uint8_t min, uint8_t max, bool st);

    //compare the profile name to a string
    bool            operator == (const std::string& n) const;
    
    //compare the profile to anoter profile 
    bool            operator == (const ChargeProfile& n) const;
    
    //return a representation of the profile
    std::string     toString () const;
    
};

class ChargeProfiles
{
    public:    
        //create a new ChargeProfiles object
                                ChargeProfiles (); 
        
        //add a new charge profile 
        //throw an exception if there is some problem inhibiting the addition of the new profile 
        void                    addProfile (const ChargeProfile&);
        
        //return a pointer to the profile with the given name
        //return nullptr if it does not exists
        const ChargeProfile*    getProfileWithName (const std::string& name) const;        
        
        //return a pointer to the profile with the given index
        //return nullptr if the index is out of size 
        const ChargeProfile*    getProfileWithIndex (size_t index) const;        
        
        
        //returns a string containing all the charge profiles
        std::string             toString () const;
        
        //returns the number of profiles stored
        size_t                  numberOfProfiles () const;        
                
    private:        
        std::vector <ChargeProfile> profiles;
};

#endif //CHARGEPROFILES_H
