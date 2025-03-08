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
 
#ifndef BATGUARD_H
#define BATGUARD_H

#include "RelayDriver.hpp"
#include "ConfigReader.hpp"
#include "StateFile.hpp"
#include "LogWriter.hpp"
#include "CapacityReader.hpp"
#include "ProfileSchedules.hpp"
#include <string>

class BatGuard
{
    public:
        //Create a batguard object, read all configuration parameters to set up serial port
        explicit            BatGuard (const std::string& cnf);
        
        //Start the end-less loop to check the battery charge and set the relay accordingly
        void                start ();
        
        //At the next polling time the run () method will close and the log file is immediately closed
        void                stop ();
        
        //returns true if the run () is in execution
        bool                isRunning () const;
        
        //Ad a message into the log file, if log was enabled
        //It is logged with error priority
        //return true if the message was added
        bool                logMessage (const std::string&);
        
        //Send the give command to the relay
        //If the command provides a feedback, it is reporte as string
        std::string         sendCommandRelay (const std::string &);
        
        //Returns a string containing the battery capacity
        std::string         getBatteryCapacity () const;
        
        //Returns a string with all the available charge profiles
        std::string         getChargeProfiles () const;
        
        //Returns a string with the command file content
        std::string         getUserCommand ();
        
        //Returns a string with the state file content
        std::string         getLastState ();     
        
        //Returns a string with the schedules
        std::string         getProfileSchedules () const;
        
        //Return batguard name and version
        static const std::string nameVersion ;
        
    private:
        const ConfigReader      configReader;
        SerialPort              serialPort;
        RelayDriver             relayDriver;
        ChargeProfiles          profiles;
        StateFile               userCommand;
        StateFile               lastState;
        ProfileSchedules        schedules;
        LogWriter               logWriter;
        CapacityReader          capacityReader;
        
        const unsigned int      sleepTime;
        const bool              checkFeedback;
        
        const ChargeProfile*    currentProfile;
        bool                    running;
        bool                    chargerState;
        bool                    profileChanged;
        const bool              chargerAtNO;        
        const uint8_t           relayChannel;
        const bool				chargerExtLast;
        const bool 				chargerExtState;
        const bool              keepState;
        
        void 		sendRelayCommand ();
        void        computeChargerState ();
        void        selectCurrentProfile ();
        void        loadProfiles ();
        void        loadSchedules ();
        void        readState ();
        void        writeState ();
};

#endif //BATGUARD_H
