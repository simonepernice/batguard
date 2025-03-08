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
 
#ifndef PROFILESCHEDULES_H
#define PROFILESCHEDULES_H

#include <vector>
#include <string>
#include <time.h>
#include "ChargeProfiles.hpp"

//Define a struct containing hour and minutes comparable with itself and with time struct tm
struct HourMin
{
    int8_t hour;
    int8_t min;
    
    //Build a HourMin object from a vector of two integers: hour then minute. 
    //If there is an error throws an exception
    explicit        HourMin (const std::vector<int>&);
    
    //Compare a time.h struct tm (only hour and minute) to HourMin object
    bool            operator >= (const struct tm*) const;
    
    //Compare a time.h struct tm (only hour and minute) to HourMin object
    bool            operator <= (const struct tm*) const;
    
    //Compare a HourMin object to another
    bool            operator >= (const HourMin) const;
    
    //Compare a HourMin object to another
    bool            operator <= (const HourMin) const;
    
    //return a string representation of hour and time
    std::string     toString () const;
    
    
};
        
//Define a struct for scheduling when an event happens at the given day of week and month and year within two HourMin intervals
struct ProfileSchedule
{
    const std::vector <bool>  dayOfMonth;   //31 elements
    const std::vector <bool>  monthOfYear;  //12 elements
    const std::vector <bool>  dayOfWeek;    //7 elements
    const ChargeProfile*      profile;
    const HourMin             from;
    const HourMin             to;
    const bool                enabled;

    //create a new schedule knowing the profile name, day of month, month of year, day of week, from minute and hour, to minute and hour, enabled
    //throws an exception if from is smaller than to or the vectors are not of the proper sizes 
                        ProfileSchedule (const bool enabled, const std::vector<bool>& moy, const std::vector<bool>& dom, const std::vector<bool>& dow, const HourMin& from, const HourMin& to, const ChargeProfile* prof);    

    //return true if the given struct tm is bigger or equal than from and smaller or equal than to and
    //struct tm happens in a day of month, month of year and day of week allowed
    bool                doesInclude (const struct tm* ) const;
    
    //return true if the given schedule has some day of month, month of year, day of week in common and
    //the given schedule from or to is between this schedule from - to but they refer to different profiles therefore there is an ambiguity 
    bool                doesOverlap (const ProfileSchedule& ) const;
    
    //return a string representation of the schedule
    std::string         toString () const;                
    
    //return a string representing a vector of boolean 
    //it uses shortcuts as begin-end or n1.n2
    static std::string  vectorBoolToString (const std::vector <bool>&, size_t index0=1);
};

//Collect the enabledSchedules and verify if any of them is triggers avoid more than one trigger for the same schedule
class ProfileSchedules
{
    public:    
        //create a new ProfileSchedules object
        explicit                ProfileSchedules (); 
        
        //add a new schedule
        //throws an exception if it cannot be added due to profile name not found or overlapping with a different schedule 
        void                    addSchedule (const ProfileSchedule&);

        //return the triggered schedule 
        //returns nullptr if no schedule is triggered
        const ProfileSchedule*  getScheduleTriggered (time_t = 0) const;  
        
        //returns a string containing all the schedules
        std::string             toString () const;
        
        //set if the schedule is enable
        //if it is not, will always return false to checkIfAnyProfileScheduleTriggered without checking
        void                    setEnable (bool);
        
        //return the enable state
        bool                    isEnabled () const;
        
        //returns the number of enabledSchedules stored
        size_t                  getNumOfSchedules () const;
                
    private:
        std::vector <ProfileSchedule>   enabledSchedules;
        std::vector <ProfileSchedule>   disabledSchedules;
        bool                            enabled;
};

#endif //PROFILESCHEDULES_H
