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

#ifndef STRINGTOOLS_H
#define STRINGTOOLS_H

#include <string>
#include <istream>
#include <vector>

//delete heading and trailing spaces from the given string which is modified
void                        trim (std::string&);

//delete heading and trailing spaces from each of the string contained in the given vector which are modified
void                        trim (std::vector <std::string>&);

//check if a line is empty or a comment, returns true if it can be ignored
bool                        isSkippable (const std::string&);

//check if the sting contains only alphanumeric chars
bool                        isValidName (const std::string&);

//split the inputstream at the separator putting the values in the outputwords
//if more separators are found close to each other an empty string is returned
void                        split (std::istream& inputstream, std::vector <std::string>& outputwords, const char separator);

//split the inputstring at the separator putting the values in the outputwords
//if more separators are found close to each other an empty string is returned
void                        split (const std::string& inputstring, std::vector <std::string>& outputwords, const char separator);

//split the inputstream at the separator returning the values as a vector of strings
//if more separators are found close to each other an empty string is returned
std::vector <std::string>   split (std::istream& inputstream, const char separator);

//split the inputstring at the separator returning the values as a vector of strings
//if more separators are found close to each other an empty string is returned
std::vector <std::string>   split (const std::string& inputstring, const char separator);

//split the inputstream at the separator putting the values in the outputwords
//separators close to each other or at the end are ignored
void                        splitMulti (std::istream& inputstream, std::vector <std::string>& outputwords, const char separator);

//split the inputstring at the separator putting the values in the outputwords
//separators close to each other or at the end are ignored
void                        splitMulti (const std::string& inputstring, std::vector <std::string>& outputwords, const char separator);

//split the inputstream at the separator returning the values as a vector of strings
//separators close to each other or at the end are ignored
std::vector <std::string>   splitMulti (std::istream& inputstream, const char separator);

//split the inputstring at the separator returning the values as a vector of strings
//separators close to each other or at the end are ignored
std::vector <std::string>   splitMulti (const std::string& inputstring, const char separator);

#endif //STRINGTOOLS_H
