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

#ifndef CONFIGREADER_H
#define CONFIGREADER_H

#include <vector>
#include <string>
#include <cstdint>

struct Configuration 
{
    const bool                      unique;
    const bool                      optional;
    const std::string               property;
    const std::vector<std::string>  values;
    const unsigned int              lineNo;
    
    //Create a new configuration from its property name, its vector of values, if can be defined once: unique is true, if can miss: optional is true, the line number at which is read in the input file
    //It can be used to create a rule, in that case unique and optional are used but not the line number
    //if it is used to create a configuration when it is read from configuration file, therefore unique and optional are not used but the line number is saved
    Configuration (const std::string& p, const std::vector<std::string>& vs, unsigned int ln = 0, bool u=false, bool o=false);
    
    //Create a new configuration from a vector of string, it is usually used as template for the lines available in the configuration file
    //For each property to be read a template must be defined
    //If the strings should unique the text !UNIQUE! as first member means the following property does not allow for duplicates, its property name, optionally its values
    //If the strings should be optional the text !OPTIONAL! as first member means the following property may not be redefined although a default is not present
    //During the configuration file the properties read are compared to those, the following events can happen:
    //A property defined unique is found more than once: an exception is thrown
    //A property with a name not present in definition is found: an exception is thrown 
    //A property defined without values is not found: an exception is thrown
    //A property defined with values is not found: this property is used for it 
    explicit Configuration (const std::vector<std::string>& pvs); 
    
    //check if the configuration property name is equal to the given string
    bool operator == (const std::string & prop) const;
    
    //check if the configurations property names are the same
    bool operator == (const Configuration & c) const; 
    
    //return a string representation of the configuration with linke number if > 1
    std::string toString () const;
};


class ConfigReader 
{
    public:
    
        //create a new config reader which are based on the following format:
        //property1 = val1, val2, val3
        //property2 = val4, val5
        //# a comment
        //requires a vector of configurations as template of all the valid properties and their default values and uniqueness read the Configuration constructor for more details 
        //if a property without default values is not found it throws an exception
        //if a property with default values is not found it is added from the initializer 
                                    ConfigReader (const std::vector <Configuration> & defaultAndAllowedConfigurations, const std::string & configFileName);
        
        //selects the first configuration with the given name, there may be more than one but the first is selected
        //throws an exception if the propert is not found
        //returns the object self to apply other methods
        const ConfigReader&         fromConfiguration (const std::string&) const;
        
        //selects the first property with the given name, it is always selected the first also if there are many 
        //returns true if it is found since more configurations with same name may exists, following call to the method may returns true again if they are found
        //if returns false, the search is reset, to a new select begins from the start 
        bool                        selectConfiguration (const std::string&) const;
        
        //print the string representing the current configuration
        std::string                 currentConfigurationToString () const;
        
        //select the next configuration with the same name of the current selection
        //if found returns true
        bool                        gotoNextConfiguration () const;
        
        //returns true if the current selection has more values
        bool                        hasMoreValues () const;
        
        //returns the number of values in the current selection
        size_t                      numberOfValues () const;
        
        //selects the given value, the first has index 0 
        //throws an exception if the index is out of range
        //any call to get next value will continue with the one after the selected
        //returns the object self to apply other methods
        const ConfigReader&         fromValue (const size_t) const;        
        
        //returns the string representing the next value, throw an exception if there is not more values 
        std::string                 getNextString () const;
        
        //returns the integer representing the next value, throw an execption if it is not an integer or there is not more values
        int                         getNextInt () const;
        
        //returns the unsigned integer representing the next value, throw an execption if it is not a positive integer or there is not more values
        unsigned int                getNextUnsignedInt () const;        
        
        //returns the byte representing the next value, throw an exception if it is not a positive integer smaller than 256 or there is not more values
        uint8_t                     getNextUnsignedInt8 () const;          
        
        //returns the bool representing the next value, throw an exception if it is not on/true/1 for true and off/false/0 for false or there is not more values
        bool                        getNextBool () const;

        //returns the next list of strings separated by dot 
        //If a non zero len is provided, the list size is compared to len and if different an exception is thrown
        std::vector <std::string>   getNextListOfString (size_t len = 0) const;        
                
        //returns the next list of integers separated by dot 
        //If a non zero len is provided, the list size is compared to len and if different an exception is thrown
        std::vector <int>           getNextListOfInt (size_t len = 0) const;        

        //returns the next list of bools separated by dot 
        //If a non zero len is provided, the list size is compared to len and if different an exception is thrown
        std::vector <bool>          getNextListOfBool (size_t len = 0) const;         
        
        //returns the next list of bools descibed indexes separated by dot (.)
        //the size of the list is given as argument as well as the index of the first element
        //the list values are all false with the exception of the indexes explicity mentioned 
        //a   means the index   at   a                                  is  set true
        //b-e means the indexes from b to e                 (inclusive) are set true
        //-e  means the indexes from the begin to e         (inclusive) are set true
        //b-  means the indexes from b to the end           (inclusive) are set true
        //-   means the indexes from the begin to the eend  (inclusive) are set true
        std::vector <bool>          getNextListOfBoolDescription (size_t length, size_t index0=1) const;    
             
    private:
        const std::vector<Configuration>    configurations;
        mutable size_t                      selectedConfig;
        mutable size_t                      selectedValue;
        mutable bool                        selected;
        
        void                                resetConfiguration () const;
        void                                setConfiguration (size_t) const;
        bool                                selectConfiguration (size_t, const std::string&) const;
        
        static std::vector<Configuration>   readConfigFile (const std::vector <Configuration> &, const std::string &);
        int                                 stringToInteger (const std::string&) const;
        bool                                stringToBool (const std::string&) const;
        void                                setTrue (std::vector <bool>&, const size_t beg, const size_t end) const;
};

#endif //CONFIGREADER_H
