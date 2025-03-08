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
 
#ifndef RELAYDRIVER_H
#define RELAYDRIVER_H

#include "SerialPort.hpp"
#include <array>
#include <vector>
#include <cstdint>

class RelayDriver
{
    public:        
        //Commands recognized by relay
        enum Command 
        { 
            OFF             = 0x00,         //turn off
            ON              = 0x01,         //turn on
            OFF_CHECK       = 0x02,         //turn off then return the current state
            ON_CHECK        = 0x03,         //turn on  then return the current state
            NEGATE_CHECK    = 0x04,         //negate current state then return the current state
            CHECK           = 0x05,         //return the current state
            NONE,                           //does nothing, not a relay command
            ERROR                           //lastError happened, not a relay command
        };
        
        //Errors related to relay management
        enum Error
        {
            NO,             //there is no lastError
            NORECV,         //was not received anything
            NOSEND,         //was not sent anything
            WRNGCRC,        //the relay feedback message has wrong CRC
            WRNGREL,        //the relay feedback message is related to a different relay than the command issued
            WRNGCOM,        //the sendCommand method was called with a wrong command (NONE or ERROR)
            WRNGSTA,        //the relay feedback provides a different state than required 
        };

        //Create an RelayDriver to manage a relay device with the given number of channels linked to the given serial port
        //It looks like if more relay than available are configured there is not any error obviously the ones not available are not managed by the commands
        //For the requests requiring an answer, it is possible specify the delay (in ms) to wait before check
        //If the delay is 0, the relay feedback is not checked
        explicit            RelayDriver (SerialPort&, uint8_t channels=8, unsigned int answaitms = 15);
        
        //Send the Command at the given relayChannel
        //If the command requires a feedback, the relay status ON/OFF is reported, otherwise NONE  
        Command             sendCommand (uint8_t channel, Command);
        
        //Check if there is a relay feedback: ON/OFF
        Command             recvCommand ();
        
        //Returns the last lastError happened
        Error               lastError () const;
        
        //Convert an lastError to a string
        static std::string  errorToString (Error);
        
        //Convert a string to a command
        static Command      stringToCommand (const std::string&);

        //Convert a command to string
        static std::string  commandToString (Command);  
        
        //Convert a command to boolean only ON and OFF are accepted otherwhise an exception is rised
        static bool         commandToBool (Command);  
        
        //Convert relay status true/false and feedback true/false to command
        static Command      relayFeedbackToCommand (bool relay, bool feedback);              
        
    private:
        static constexpr unsigned int               messageLength = 4;
        
        const uint8_t                               maxRelayChannels; //one more than the real number because [0] is not used!!!!
        std::vector <bool>                          relayStates;
        SerialPort&                                 serialPort;        
        std::array <uint8_t, messageLength>         message;        
        const unsigned int                          answerWaitMs;
        Error                                       error;
        uint8_t                                     relayChannel;
        
        bool            sendMessage ();
        bool            recvMessage ();        
        void            addCRC ();
        uint8_t         computeCRC ();
        bool            checkCRC ();
};

#endif //RELAYDRIVER_H
