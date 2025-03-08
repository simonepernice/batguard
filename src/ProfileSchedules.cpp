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
 
#include "ProfileSchedules.hpp"
#include <stdexcept>
#include <algorithm>
#include <cstdint>

HourMin::HourMin (const std::vector<int>& hm) :
    hour    {static_cast <int8_t> (hm [0])},
    min     {static_cast <int8_t> (hm [1])}
{
    if (hm.size () != 2) throw std::invalid_argument ("The time is defined by hour.minute instead it was received more than 2 elements");
    if (hm [0] < 0 or hm [0] > 23) throw std::invalid_argument ("The hour is outside the range 0-23 : " + std::to_string (hm [0]));
    if (hm [1] < 0 or hm [1] > 59) throw std::invalid_argument ("The minute is outside the range 0-59 : " + std::to_string (hm [1]));
}
    
bool HourMin::operator >= (const struct tm* t) const
{
    if (hour > t->tm_hour) return true;
    if (hour < t->tm_hour) return false;
    return min >= t->tm_min;
}

bool HourMin::operator <= (const struct tm* t) const
{
    if (hour < t->tm_hour) return true;
    if (hour > t->tm_hour) return false;
    return min <= t->tm_min;
}

bool HourMin::operator >= (const HourMin t) const
{
    if (hour > t.hour) return true;
    if (hour < t.hour) return false;
    return min >= t.min;
}

bool HourMin::operator <= (const HourMin t) const
{
    if (hour < t.hour) return true;
    if (hour > t.hour) return false;
    return min <= t.min;
}

std::string HourMin::toString () const
{
    return std::to_string (hour) + '.' + std::to_string (min);
}

ProfileSchedule::ProfileSchedule (const bool e, const std::vector<bool>& moy, const std::vector<bool>& dom, const std::vector<bool>& dow, const HourMin& f, const HourMin& t, const ChargeProfile* pi) :
    dayOfMonth      {dom},   //31 elements
    monthOfYear     {moy},  //12 elements
    dayOfWeek       {dow},    //7 elements
    profile         {pi},
    from            {f},  
    to              {t},
    enabled         {e}
{
    if (from >= to) throw std::invalid_argument ("The given from time should be lower than to time, instead it was found, from: " + from.toString () + ", to: " + to.toString ());
    if (dayOfMonth.size () != 31) throw std::invalid_argument ("On ProfileSchedule constructor the day of month does not have 31 elements, instead has :" + std::to_string (dayOfMonth.size ()));
    if (monthOfYear.size () != 12) throw std::invalid_argument ("On ProfileSchedule constructor the month of year does not have 12 elements, instead has :" + std::to_string (monthOfYear.size ()));
    if (dayOfWeek.size () !=  7) throw std::invalid_argument ("On ProfileSchedule constructor the day of week does not have 71 elements, instead has :" + std::to_string (dayOfWeek.size ()));
}

bool ProfileSchedule::doesInclude (const struct tm* t) const
{  
    return  enabled                     and
            monthOfYear [t->tm_mon]     and //0 - 11
            dayOfMonth [t->tm_mday-1]   and //1 - 31
            dayOfWeek [t->tm_wday]      and //0 - 6 since Sunday
            from <= t                   and
            to >= t;
}

bool ProfileSchedule::doesOverlap (const ProfileSchedule& s) const
{
    if (profile == s.profile or (not enabled) or (not s.enabled)) return false;
    
    size_t i;
    for (i = 0; i != 31; ++i) if (dayOfMonth [i] and s.dayOfMonth [i]) break;
    if (i == 31) return false;
    
    for (i = 0; i != 12; ++i) if (monthOfYear [i] and s.monthOfYear [i]) break;
    if (i == 12) return false;
    
    for (i = 0; i != 7; ++i) if (dayOfWeek [i] and s.dayOfWeek [i]) break;
    if (i == 7) return false;
    
    if ((s.from >= from and s.from <= to) or (s.to >= from and s.to <= to)) return true;
    
    return false;
}

std::string ProfileSchedule::vectorBoolToString (const std::vector <bool>& vb, size_t index0)
{
    std::string res;
    size_t b = 0, e = 0;
    bool inside = false;
    char sep = '\0';
    for (size_t i = 0; i <= vb.size (); ++ i) //need to check one element after the end to detect an eventual interval going to the end
    {
        if (i < vb.size () and vb [i]) 
        {
            if (inside) 
            {
                e = i;
            }
            else 
            {
                b = i;
                e = i;
                inside = true;
            }
        }
        else if (inside)
        {
            inside = false;
            
            if (sep) res += sep;
            else sep = '.'; 
            
            if (b == e) res += std::to_string (b + index0);
            else
            {
                if (b > 0) res += std::to_string (b + index0);
                res +=  '-';
                if (e < vb.size () - 1) res += std::to_string (e + index0);            
            }                
        }    
    }
    return res;
}

std::string ProfileSchedule::toString () const
{
    return std::string ("(Enabled: ") + (enabled ? "on, " : "off, ") + "MOY: " + vectorBoolToString (monthOfYear) + ", DOM: " + vectorBoolToString (dayOfMonth) + ", DOW: " + vectorBoolToString (dayOfWeek) + ", from: " + from.toString () + ", to: " + to.toString () + ", profile: " + profile->toString () + ')';
}

ProfileSchedules::ProfileSchedules () :
    enabled         {false}
{
} 

void ProfileSchedules::addSchedule (const ProfileSchedule& ns)
{   
    if ((ns.enabled and enabledSchedules.size () == SIZE_MAX) or (not ns.enabled and disabledSchedules.size () == SIZE_MAX)) throw std::invalid_argument ("Too many enabledSchedules were added, batguard supports up to " + std::to_string (SIZE_MAX - 1) + " enabledSchedules");
            
    if (ns.profile == nullptr) throw std::invalid_argument ("The given profile is not defined");        
    
    const ProfileSchedule* overlap;
    if (std::any_of (enabledSchedules.begin (), enabledSchedules.end (), [&] (const ProfileSchedule& s) {return (ns.doesOverlap (s) ? (overlap = &s, true) : false); } ) ) throw std::invalid_argument ("The profile schedule: " + ns.toString () + " timing overlap to the schedule: " + overlap -> toString () + " and have different profiles");
    
    //Although a not-enabled schedule would work because doesInclude would work, it would extend the search time without any benefit 
    //The only drawback is that not-enabled enabledSchedules will not appear in the list of enabledSchedules
    if (ns.enabled) enabledSchedules.push_back (ns);
    else disabledSchedules.push_back (ns);
}

const ProfileSchedule* ProfileSchedules::getScheduleTriggered (time_t nowraw) const
{
    if (not enabled or enabledSchedules.size () == 0) return nullptr;
    
    if (not nowraw) nowraw = time (nullptr);
    
    const struct tm* now = localtime (& nowraw);

    const auto& selectedScheduleIt = std::find_if (enabledSchedules.begin (), enabledSchedules.end (), [&](const ProfileSchedule & ps) {return ps.doesInclude (now);});
    
    if (selectedScheduleIt != enabledSchedules.end ()) return & (*selectedScheduleIt); 
    
    return nullptr;
}

std::string  ProfileSchedules::toString () const
{
    std::string schels;
    std::for_each (enabledSchedules.begin(), enabledSchedules.end(), [&](const ProfileSchedule & sche){ schels += sche.toString () + '\n'; } );
    std::for_each (disabledSchedules.begin(), disabledSchedules.end(), [&](const ProfileSchedule & sche){ schels += sche.toString () + '\n'; } );
    return schels;
}

void ProfileSchedules::setEnable (bool e)
{
    enabled = e;
}

size_t ProfileSchedules::getNumOfSchedules () const
{
    return enabledSchedules.size () + disabledSchedules.size ();
}

bool ProfileSchedules::isEnabled () const
{
    return enabled;
}
