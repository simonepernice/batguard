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
 
#ifndef LOGWRITER_H
#define LOGWRITER_H

#include <string>
#include <fstream>
#include <cstdint>

class LogWriter
{
    public:
        enum Level 
        {
            ERROR = 0,
            BASIC = 1,
            FULL = 2
        };
        
        //Create a log file with the given path saving only messages with a level lower to maxErrLvl
        //Therefore if maxErrLvl is zero, the log file is not even created because nothing can be print
        //The log is flushed if the number of written lines since last flush reach maxToFlsLns or a message with a level lower than maxFlushLvl is written
        //The log file is splitted when it exceeds the maxLine number 
                        LogWriter (const std::string& path, uint8_t maxErrLvl, uint8_t maxFlushLvl, unsigned int maxToFlsLns, unsigned int maxLogLines);
        
        //The log file is permanently open, therefore it is closed in the distructor
                        ~LogWriter ();
        
        //Add a new log message to the tail
        //If the message level is higher than the current maxErrLvl is scrapped
        //returns true if the message has a level allowing its wrote 
        bool            writeMessage (Level level, const std::string& message);
        
        //Returns the current number of lines of the log file
        unsigned int    getNumLines () const;
        
        //flush the message queue into the drive
        void            flushMessages (); 
        
    private:
        char                dateCharArray [128];
        const std::string   logFileName;
        std::ofstream       logFile;
        const uint8_t       maxErrLvl;
        const uint8_t       maxFlushLvl;
        const unsigned int  maxToFlsLns;
        unsigned int        toFlsLns;
        const unsigned int  maxLogLns;
        unsigned int        logLns;                
                
        void            computeDate ();
        void            open ();
        unsigned int    countLogLines ();     
        bool            splitLogFile (); 
};

#endif //LOGWRITER_H
