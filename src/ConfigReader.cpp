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

#include "ConfigReader.hpp"
#include "stringtools.hpp"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>

Configuration::Configuration (const std::string& p, const std::vector<std::string>& vs, unsigned int ln, bool u, bool o) : 
    unique      {u}, 
    optional    {o}, 
    property    {p}, 
    values      {vs},
    lineNo      {ln} 
{}

Configuration::Configuration (const std::vector<std::string>& pvs) : 
    unique      {pvs[0] == "!UNIQUE!"}, 
    optional    {pvs[0] == "!OPTIONAL!"}, 
    property    {pvs[0 + unique + optional]}, 
    values      {pvs.begin () + 1 + unique + optional, pvs.end ()},
    lineNo      {0} 
{}

bool Configuration::operator == (const std::string & prop) const 
{
    return property == prop;
}

bool Configuration::operator == (const Configuration & c) const 
{
    return property == c.property;
}

std::string Configuration::toString () const
{
    std::string res;
    if (lineNo) res += " at the line number: " + std::to_string (lineNo) + ',';
    else res += ", this internal error on default setting: contact the author, ";
    res += " with text: '" + property + " = ";
    char sep = '\0';
    std::for_each (values.begin (), values.end (), [&] (const std::string& val) {if (sep) res += sep; else sep = ','; res += val;});
    res += '\'';
    return res;
}

std::vector <Configuration> ConfigReader::readConfigFile (const std::vector <Configuration> & defconfs, const std::string & configfilename)
{
    std::ifstream configfile (configfilename);        
    if (not configfile.good ()) throw std::invalid_argument ("The configuration file '" + configfilename + "' was not found or is not readable");
    
    std::vector<Configuration> cfgs;
    
    unsigned int lineNumber = 0;
    for (std::string& line : split (configfile, '\n')) 
    {
        ++ lineNumber;
        
        if (isSkippable (line)) continue;        
        trim (line);
        
        std::vector <std::string> propertyvalues;
        split (line, propertyvalues, '=');
        
        if (propertyvalues.size () != 2) throw std::invalid_argument ("Invalid format, the format required is: left argument = right argument at the line number: " + std::to_string (lineNumber) + ", with text :" + line);
        
        std::string property = propertyvalues [0];
        trim (property);
        if (not isValidName (property)) throw std::invalid_argument ("The property name shall contain only alphanumeric characters instead found: '" + property + "', at the line number: " + std::to_string (lineNumber) + ", with text :" + line);
        
        auto defcon = std::find (defconfs.begin (), defconfs.end (), property); 
        if (defcon == defconfs.end ()) throw std::invalid_argument ("The following configuration '" + property + "' is not among the ones allowed at the line number: " + std::to_string (lineNumber) + ", with text :" + line);
        
        if (defcon -> unique and std::find (cfgs.begin (), cfgs.end (), *defcon) != cfgs.end ()) throw std::invalid_argument ("The property '" + property + "' should be unique but was found twice, last at the line number: " + std::to_string (lineNumber) + ", with text :" + line);
        
        std::vector <std::string> values;
        split (propertyvalues [1], values, ',');

        if (values.size () == 0) throw std::invalid_argument ("The following configuration '" + property + "' has not any value at the line number: " + std::to_string (lineNumber) + ", with text :" + line);
        trim (values);
        
        cfgs.push_back ({property, values, lineNumber});        
    }
    
    for (const Configuration & dc : defconfs)
    {
        if (dc.optional) continue;
        
        auto cf = std::find (cfgs.begin (), cfgs.end (), dc);
        
        if (cf == cfgs.end ()) 
        {
            if (dc.values.size () == 0) throw std::invalid_argument ("The following required configuration '" + dc.property + "' was not found");
            else cfgs.push_back (dc);
        }
        else if (dc.values.size () and cf -> values.size () != dc.values.size ()) throw std::invalid_argument ("The configuration " + cf -> toString () + " has the wrong number of values");
    }
        
    return cfgs;
}

ConfigReader::ConfigReader (const std::vector <Configuration> & props, const std::string & fname) :
    configurations    {readConfigFile (props, fname)} 
{
    resetConfiguration ();
}

void ConfigReader::setConfiguration (size_t i) const
{
    selected = true;
    selectedConfig = i;
    selectedValue = 0;
}

void ConfigReader::resetConfiguration () const
{
    selectedConfig = 0;
    selectedValue = 0;
    selected = false;
}

const ConfigReader& ConfigReader::fromConfiguration (const std::string& c) const
{
    if (not selectConfiguration (c)) throw std::runtime_error ("ConfigReader fromConfiguration called with a not existing configuration");
    return *this;
}

bool ConfigReader::selectConfiguration (size_t beg, const std::string& prop) const
{
    auto sel = std::find (configurations.begin () + beg, configurations.end (), prop);
    if (sel == configurations.end ())
    {
        resetConfiguration ();
        return false;
    }
    setConfiguration (std::distance (configurations.begin (), sel));
    return true;
}

bool ConfigReader::selectConfiguration (const std::string& prop) const
{
    return selectConfiguration (0, prop);
}

bool ConfigReader::gotoNextConfiguration () const
{
    if (not selected) return false;
    return selectConfiguration (selectedConfig + 1, configurations [selectedConfig].property);
}
    
const ConfigReader& ConfigReader::fromValue (const size_t i) const
{
    if (not selected or i >= configurations [selectedConfig].values.size ()) throw new std::runtime_error ("ConfigReader fromValue called with an index out of bound or before a profile selection");
    
    selected = true;
    selectedValue = i;
    
    return *this;
}

bool ConfigReader::hasMoreValues () const
{
    return selected and selectedValue < configurations [selectedConfig].values.size ();
}

std::string ConfigReader::getNextString () const
{
    if (hasMoreValues ()) return configurations [selectedConfig].values [selectedValue ++];
    throw std::invalid_argument ("It was expected one more parameter " + configurations [selectedConfig].toString ());
}

int ConfigReader::getNextInt () const
{
    return stringToInteger (getNextString ());
}

unsigned int ConfigReader::getNextUnsignedInt () const
{
    int ni = getNextInt ();
    if (ni < 0) throw std::invalid_argument ("Was expected an unsigned integer instead found: " + configurations [selectedConfig].values [selectedValue-1] + configurations [selectedConfig].toString ());
    return static_cast<unsigned int> (ni);
} 

uint8_t ConfigReader::getNextUnsignedInt8 () const
{
    int ni = getNextUnsignedInt ();
    if (ni > 255) throw std::invalid_argument ("Was expected a value smaller than 256 instead found: " + configurations [selectedConfig].values [selectedValue-1] + configurations [selectedConfig].toString ());
    return static_cast<uint8_t> (ni);
}        
                
bool ConfigReader::getNextBool () const
{
    return stringToBool (getNextString ());
}

size_t ConfigReader::numberOfValues () const
{
    if (selected) return configurations [selectedConfig].values.size ();
    return 0;
} 

std::vector <std::string> ConfigReader::getNextListOfString (size_t len) const
{
    std::vector <std::string> res = split (getNextString (), '.');  
      
    if (len and res.size () != len) throw std::invalid_argument ("It was expected a list of " + std::to_string (len) + " size, instead it was found: " + configurations [selectedConfig].values [selectedValue-1] + configurations [selectedConfig].toString ());
    trim (res);
    return res;
}

std::vector <int> ConfigReader::getNextListOfInt (size_t len) const
{
    std::vector <std::string> ss = getNextListOfString (len);
    std::vector <int> res (ss.size ());

    std::transform (ss.begin (), ss.end (), res.begin (), [&] (const std::string& s) {return stringToInteger (s);});

    return res;
}

std::vector <bool> ConfigReader::getNextListOfBool (size_t len) const
{
    std::vector <std::string> ss = getNextListOfString (len);
    std::vector <bool> res (ss.size ());

    std::transform (ss.begin (), ss.end (), res.begin (), [&] (const std::string& s) {return stringToBool (s);});

    return res;
}

void ConfigReader::setTrue (std::vector <bool>& vb, const size_t beg, const size_t end) const
{
    for (size_t i = beg; i <= end; ++ i)
    {
        if (vb [i]) throw std::invalid_argument ("There is an overlap among the selected indexes, in the interval " + std::to_string (beg) + " - " + std::to_string (end) + " the index " + std::to_string (i) + " was already selected on"+ configurations [selectedConfig].values [selectedValue-1] + configurations [selectedConfig].toString ());
        vb [i] = true;
    }
}

std::string ConfigReader::currentConfigurationToString () const
{
    return configurations [selectedConfig].toString ();
}

std::vector <bool> ConfigReader::getNextListOfBoolDescription (size_t size, size_t index0) const
{
    std::vector <bool> res (size, false);
    
    std::vector <std::string> ranges = getNextListOfString ();    
    for (const std::string& range : ranges)
    {
        std::vector <std::string> extremes = split (range, '-'); 
        if (extremes.size () > 2) throw std::invalid_argument ("It was expected an interval of two elements separated by -, instead it was found " + range + " on " + configurations [selectedConfig].values [selectedValue-1] + configurations [selectedConfig].toString ());
        
        trim (extremes);
        size_t indexes [2];
        if (extremes.size () == 1) 
        {
            indexes [0] = indexes [1] = stringToInteger (extremes [0]) - index0;
        }
        else
        {
            indexes [0] = 0;
            indexes [1] = size - 1;
            for (size_t i = 0; i < 2; ++ i)
            {
                if (extremes [i].size () == 0) continue;
                indexes [i] = stringToInteger (extremes [i]) - index0;
            }
        }
        
        setTrue (res, indexes [0], indexes [1]);
    }
    
    return res;
}
            

int ConfigReader::stringToInteger (const std::string& s) const
{
    try
    {
        return std::stoi (s);
    }
    catch (...)
    {
        throw std::invalid_argument ("Was expected a number instead found: " + s + configurations [selectedConfig].toString ());
    }    
}

bool ConfigReader::stringToBool (const std::string& ns) const
{
    if      (ns == "on"  or ns == "true"  or ns == "1") return true;
    else if (ns == "off" or ns == "false" or ns == "0") return false;
    
    throw std::invalid_argument ("It was expected a boolean value on/off/ture/false/1/0 instead was found '" + ns + "' " + configurations [selectedConfig].toString ());
}
