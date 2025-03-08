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

#include "stringtools.hpp"
#include <sstream>
#include <algorithm>

bool isSkippable (const std::string& line)
{
    if (line.length () == 0) return true;
    
    for (char c : line) 
    {
        if (c == ' ' or c == '\t') continue;
        if (c == '#') return true;
        return false;
    }
    
    return true;
}    

void trim (std::string& str)
{
    str.erase (0, str.find_first_not_of (" \t\n"));
    str.erase (str.find_last_not_of (" \t\n") + 1);
}

void trim (std::vector <std::string>& strvct)
{
    for (size_t i = 0; i < strvct.size (); ++ i) trim (strvct [i]);
}

std::vector <std::string>   splitMulti (std::istream& inputstream, const char separator);

void split (std::istream& is, std::vector <std::string>& out, const char sep)
{
    bool reachedEOF = true;
    std::string word;
    while (getline (is, word, sep)) 
    {
        reachedEOF = is.eof ();
        out.push_back (word);
    }
    
    if (not reachedEOF) out.push_back (""); //the inputstream ended by a separator
}

void split (const std::string& is, std::vector <std::string>& out, const char sep)
{
    std::istringstream iss (is);
    split (iss, out, sep);
}
std::vector <std::string> split (std::istream& is, const char sep)
{
    std::vector <std::string> words;
    split (is, words, sep);
    return words;
}

std::vector <std::string> split (const std::string& is, const char sep)
{
    std::istringstream iss (is);
    return split (iss, sep);
}

void splitMulti (std::istream& is, std::vector <std::string>& out, const char sep)
{
    std::string word;
    while (getline (is, word, sep)) if (word.size ()) out.push_back (word);
}    

void splitMulti (const std::string& is, std::vector <std::string>& out, const char sep)
{
    std::istringstream iss (is);
    splitMulti (iss, out, sep);
}    

std::vector <std::string> splitMulti (std::istream& is, const char sep)
{
    std::vector <std::string> words;
    splitMulti (is, words, sep);
    return words;
}

std::vector <std::string> splitMulti (const std::string& is, const char sep)
{
    std::istringstream iss (is);
    return splitMulti (iss, sep);
}

bool isValidName (const std::string& s)
{
    return std::all_of (s.begin (), s.end (), [](char c) {return std::isalnum (c) or c == '_';});
}
