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

#include "StateFile.hpp"
#include "stringtools.hpp"
#include <fstream>
#include <stdexcept>

void StateFile::resetState ()
{
    charger = LAST;
    scheduler = LAST;
    profile = nullptr;
    logger = false;
}

StateFile::StateFile (const std::string& fn, const ChargeProfiles& cp) :
    fileName        {fn},
    chargeProfiles  {cp}
{
    resetState ();
}   

std::string StateFile::toString () const
{
    return "Profile: " + (profile ? profile->toString () : std::string ("not defined")) + "; charger: " + stateToString (charger) + "; scheduler: " + stateToString (scheduler) + (logger ? ", logger to flush" : "");
}

StateFile::Error StateFile::read ()
{    
    charger = LAST;
    scheduler = LAST;
    profile = nullptr;
    logger = false;
        
    std::ifstream runfile (fileName);
    
    if (not runfile.good ()) return NOTFUND;
    
    std::vector <std::string> values = splitMulti (runfile, ' ');  
    trim (values); //to remove new lines
    if (values.size () == 0) return RFEMPTY;
            
    size_t index = 0;
    while (index < values.size ())
    {
        if (values [index].size ()) 
        {
            if (values [index][0] == '#')
            {
                if      (values [index] == "#chargeron")    
                {
                    if (charger != LAST) return (resetState (), MUCHSET);
                    charger = ON;
                }
                else if (values [index] == "#chargeroff")   
                {
                    if (charger != LAST) return (resetState (), MUCHSET);
                    charger = OFF;
                }
                else if (values [index] == "#scheduleron")    
                {
                    if (scheduler != LAST) return (resetState (), MUSHSET);
                    scheduler = ON;
                }
                else if (values [index] == "#scheduleroff")   
                {
                    if (scheduler != LAST) return (resetState (), MUSHSET);
                    scheduler = OFF;
                }
                else if (values [index] == "#loggerflush")   
                {
                    if (logger == true) return (resetState (), MULOSET);
                    logger = true;
                }
                else                                
                {
                    return (resetState (), UNKNSTA);
                }
            }
            else
            {            
                if (profile != nullptr) return (resetState (), MUPRSET);
                
                profile = chargeProfiles.getProfileWithName (values [index]);
                
                if (profile == nullptr) return (resetState (), PROFILE);
            }
        }
        
        ++ index;
    }
    
    return NO;
}

StateFile::Error StateFile::write (const ChargeProfile* profile, State cha, State sch) const
{
    std::ofstream runfile (fileName);
    
    if (not runfile.good ()) return NOTWRIT;
    
    if (profile != nullptr) runfile << profile->name << ' ';
    
    if (cha != LAST) runfile << (cha == ON ? "#chargeron" : "#chargeroff") << ' ';
    
    if (sch != LAST) runfile << (sch == ON ? "#scheduleron" : "#scheduleroff") << ' ';
    
    runfile << '\n';
    
    return NO;
}

StateFile::State StateFile::chargerInit () const
{
    return charger;
}

StateFile::State StateFile::schedulerInit () const
{
    return scheduler;
}

const ChargeProfile* StateFile::profileInit () const
{
    return profile;
}

bool StateFile::loggerInit () const
{
    return logger;
}

bool StateFile::isChangedRespectTo (const ChargeProfile* pi, StateFile::State ch, StateFile::State sc) const
{
    return profile != pi or charger != ch or scheduler != sc or logger != false;
}

std::string StateFile::errorToString (StateFile::Error e)
{
    switch (e)
    {
        case NO:        return "no error";
        case PROFILE:   return "profile not recognized";
        case UNKNSTA:   return "charger/scheduler command not recognized";
        case RFEMPTY:   return "state file is empty";
        case NOTFUND:   return "state file is not present";
        case NOTWRIT:   return "state file is not writable";
        case MUCHSET:   return "there are several settings for the charger";
        case MUSHSET:   return "there are several settings for the scheduler";
        case MUPRSET:   return "there are several settings for the profile";
        case MULOSET:   return "there are several settings for the logger";
    }
    throw std::runtime_error ("Internal error on StateFile::errorToString");
}

bool StateFile::stateToBool (StateFile::State c)
{
    if (c == ON)        return true;
    if (c == OFF)       return false;
    throw std::runtime_error ("Internal error on StateFile::chargerToBool");
}

std::string StateFile::stateToString (StateFile::State c)
{
    if (c == ON)        return "on";
    if (c == OFF)       return "off";
    if (c == LAST)      return "last";
    throw std::runtime_error ("Internal error on StateFile::chargerToString");
}

StateFile::State StateFile::boolToState (bool s)
{
    if (s) return ON;
    return OFF;
}
