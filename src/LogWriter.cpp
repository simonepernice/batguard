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
 
#include "LogWriter.hpp"
#include <ctime>
#include <cstdio> 

LogWriter::LogWriter (const std::string& path, uint8_t mel, uint8_t fl, unsigned int mtfln, unsigned int mln) :
    dateCharArray   {},
    logFileName     {path},
    maxErrLvl       {mel},
    maxFlushLvl     {fl},
    maxToFlsLns     {mtfln},
    toFlsLns        {0}, 
    maxLogLns       {mln},
    logLns          {countLogLines ()}       
{
    if (maxErrLvl == 0) return;
    
    if (maxErrLvl > FULL + 1) throw std::invalid_argument ("The log level required: " + std::to_string (maxErrLvl) + " exceeds the maximum: " + std::to_string (FULL + 1));
    
    if (maxFlushLvl > FULL + 1) throw std::invalid_argument ("The flush level required: " + std::to_string (maxFlushLvl) + " exceeds the maximum: " + std::to_string (FULL + 1));
            
    open ();
    
    if (not logFile.good ()) throw std::invalid_argument ("It is not possible to create the log file " + path);
}

LogWriter::~LogWriter ()
{
    if (logFile.is_open ()) logFile.close ();
}

void LogWriter::computeDate ()
{
    time_t now = time (nullptr);
    const tm* localTime = localtime (& now);
    strftime (dateCharArray, sizeof (dateCharArray), "%Y-%m-%d %H:%M:%S", localTime);
}

bool LogWriter::writeMessage (LogWriter::Level level, const std::string& message)
{
    if (level >= maxErrLvl) return false;
    
    computeDate ();
    
    logFile << dateCharArray << " | level: ";
    
    switch (level)
    {
        case ERROR: 
            logFile << "ERROR";
            break;
        case BASIC:
            logFile << "BASIC";
            break;
        case FULL:
            logFile << "FULL ";
            break;
    }
    
    logFile << " | message: " << message << '\n';
    
    ++ toFlsLns;    
    if (toFlsLns >= maxToFlsLns or level < maxFlushLvl) flushMessages ();
    
    ++ logLns;
    if (logLns >= maxLogLns) 
        if (splitLogFile ()) 
            logFile << dateCharArray << " | level: ERROR | message: It was not possible to split the log file although reached the max length\n";
    
    return true;
}   

void LogWriter::flushMessages ()
{
    logFile << std::flush;
    toFlsLns = 0;
}

unsigned int LogWriter::getNumLines () const
{
    return logLns;
}

unsigned int LogWriter::countLogLines ()
{
    unsigned int lns = 0;
    
    std::ifstream lf (logFileName);
    
    if (not lf.is_open ()) return 0;
    
    std::string _;
    while (getline (lf, _)) ++ lns;
    
    lf.close ();
    
    return lns;
}

void LogWriter::open () 
{
    if (maxErrLvl == 0 or logFile.is_open ()) return;
    
    logFile.open (logFileName, std::ios::app);
    
    //if the file does not exists, it is created
    if (not logFile.is_open ()) logFile.open (logFileName);
}
    
bool LogWriter::splitLogFile () 
{    
    logFile.close ();
    
    std::string newFileName;
    
    {
        int freeIndex = 1;
        while (true) 
        {
            newFileName = logFileName + '.' + std::to_string (freeIndex);
            std::ifstream nlf (newFileName);
            if (nlf.is_open ()) nlf.close ();
            else break;
            ++ freeIndex;
        }
    }
    
    bool error = static_cast <bool> (std::rename (logFileName.c_str (), newFileName.c_str ()));
    
    if (not error)
    {
        logLns = 0;
        toFlsLns = 0;    
    }
    
    open ();
    
    return error;
}
