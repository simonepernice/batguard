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

#ifndef STATEFILE_H
#define STATEFILE_H

#include "ChargeProfiles.hpp"
#include <string>

class StateFile 
{
    public:
        //represent all the possible chargerInit state
        enum State
        {
            OFF  = 0x00,    //force off 
            ON   = 0x01,    //force on  
            LAST,           //keep last value 
        };
        
        //represents all the possible lastError during the runfile scan
        enum Error
        {
            NO = 0,
            PROFILE,        //profile not recognized
            UNKNSTA,        //charger/scheduler state not recognized
            RFEMPTY,        //state file is empty or not exists
            NOTFUND,        //state file is not present
            NOTWRIT,        //state file is not writable
            MUCHSET,        //multiple settings for charger
            MUSHSET,        //multiple settings for scheduler
            MUPRSET,        //multiple settings for profile
            MULOSET,        //multiple settings for logger
        };
        
        //create a StateFile working on fileName and updating the given charge profile
                                StateFile (const std::string& fileName, const ChargeProfiles&);
        
        //read the runfile and update the internal state: profile index, charger state, scheduler state
        //return NO the file is correctly written
        //return the ERROR found if there runfile was not well formed
        //if an error is found also the correct data read is ignored 
        Error                   read ();
        
        //write on the runfile the name of the given profile index if not SIZE_MAX
        //then write the charger status if not LAST
        //eventually write the scheduler state if not LAST
        Error                   write (const ChargeProfile* profile = nullptr, State charger = LAST, State scheduler = LAST) const;
        
        //returns the charger forced status if  if an optional on/off follows the state name
        //the runfile is updated removing the chargerInit value 
        State                   chargerInit () const;

        //returns the scheduler forced status if  if an optional on/off follows the state name
        //the runfile is updated removing the chargerInit value 
        State                   schedulerInit () const;
        
        //return the index of the profile read
        //if no profile was read or if it was misspelled return nullptr
        const ChargeProfile*    profileInit () const;        
        
        //return true if the logger requires to be flushed 
        bool                    loggerInit () const;           
        
        //returns a string representing the internal state of the StateFile
        std::string             toString () const;
        
        //returns true if the state file is changed respect to last profile, charger or scheduler states or there was a log flush 
        bool                    isChangedRespectTo (const ChargeProfile* profile = nullptr, State charger = LAST,  State scheduler = LAST) const;  
        
        //convert an error in a readable string
        static std::string      errorToString (Error);
        
        //convert a charger valie in a readable string
        static std::string      stateToString (State);
        
        //convert a charger to a boolean
        static bool             stateToBool (State);
        
        //convert a bool to a state 
        static State            boolToState (bool);        
        
    private:
        const std::string       fileName;
        const ChargeProfiles&   chargeProfiles;
        State                   charger;
        State                   scheduler;
        const ChargeProfile*    profile;
        bool                    logger;
        
        void                    resetState ();
};

#endif //STATEFILE_H
