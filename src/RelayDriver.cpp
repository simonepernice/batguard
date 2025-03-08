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
 
#include "RelayDriver.hpp"
#include <unistd.h>
#include <stdexcept>
#include <algorithm>

bool RelayDriver::sendMessage ()
{
    if (not std::all_of (message.begin (), message.end (), [this](uint8_t c) {return serialPort.writeByte (c);})) return false ;
    return serialPort.writeFlush () >= messageLength;
}

bool RelayDriver::recvMessage ()
{
    if (serialPort.readFlush () < messageLength) return false;
    
    //if there are extra inputs, may be previous ones were missed, it realign to the last 4 bytes received
    while (serialPort.bytesToRead () > messageLength) serialPort.readByte (); 
    
    for (unsigned int i = 0; i < messageLength; ++ i) message [i] = serialPort.readByte ();
    return true;
}

uint8_t RelayDriver::computeCRC ()
{
    unsigned int crc = 0;
    for (unsigned int i = 0; i < messageLength - 1; ++ i) crc += message [i];
    return static_cast<uint8_t> (crc % 0x100);
}

void RelayDriver::addCRC ()
{
    message [messageLength - 1] = computeCRC ();
}

bool RelayDriver::checkCRC ()
{
    return message [0] == 0xA0 and message [messageLength - 1] == computeCRC ();
}

RelayDriver::RelayDriver (SerialPort& s, uint8_t channels, unsigned int awms) :
    maxRelayChannels    {++ channels}, //one more because [0] is not used!!!!
    relayStates         (maxRelayChannels, false),
    serialPort          {s},    
    answerWaitMs        {awms * 1000},
    error               {NO}
{
    //called sendCommand to initialize relayState variable properly
    for (uint8_t c = 1; c < maxRelayChannels; ++ c) sendCommand (c, CHECK);
}

RelayDriver::Error RelayDriver::lastError () const
{
    return error;
}

RelayDriver::Command RelayDriver::recvCommand ()
{
    if (not recvMessage ()) 
    {
        error = NORECV;
        return ERROR;
    }

    if (not checkCRC ()) 
    {
        error = WRNGCRC;
        return ERROR;
    }
   
    if (message [1] != relayChannel)
    {
        error = WRNGREL;
        return ERROR;
    }    
 
    if ((bool) message [2] != relayStates [relayChannel])
    {
        relayStates [relayChannel] = (bool) message [2]; //align internal relay state with real status
        error = WRNGSTA;
        return ERROR;
    }

    error = NO;
    return (Command) message [2];    
}

RelayDriver::Command RelayDriver::sendCommand (uint8_t channel, Command c)
{
    if (channel < 1 or channel > maxRelayChannels) throw std::invalid_argument ("Relay driver called with a relay channel out of range");
    
    if (c == NONE or c == ERROR) 
    {
        error = WRNGCOM;
        return ERROR;
    }
    
    relayChannel = channel;
    
    switch (c) 
    {
        case ON:
        case ON_CHECK:
            relayStates [relayChannel] = true;
            break;
        case OFF:
        case OFF_CHECK:
            relayStates [relayChannel] = false;
            break;
        case NEGATE_CHECK:
            relayStates [relayChannel] = not relayStates [relayChannel]; 
            break;
        case CHECK:
            //keep previous value
            break;
        case NONE:
        case ERROR:
            throw std::runtime_error ("The send command reached a wrong state");
            break;
    }
    
    message [0] = 0xA0;
    message [1] = relayChannel;
    message [2] = c;
    addCRC ();
    
    if (not sendMessage ()) 
    {
        error = NOSEND;
        return ERROR;
    }
    
    if (c == OFF or c == ON) 
    {
        error = NO;
        return NONE;
    }
    
    if (answerWaitMs)
    {
        usleep (answerWaitMs); 
        return recvCommand ();
    }

    error = NORECV;
    return ERROR;
}

std::string RelayDriver::errorToString (RelayDriver::Error e)
{
    switch (e)
    {
        case NO:        return "there is not any error";
        case NORECV:    return "the feedback was not received";
        case NOSEND:    return "it was not sent anything";
        case WRNGCRC:   return "the relay feedback message has wrong CRC";
        case WRNGREL:   return "the relay feedback message is related to a different relay than the one to which the command was issued to";
        case WRNGCOM:   return "the sendCommand method called with a wrong command (NONE or ERROR)";
        case WRNGSTA:   return "the relay feedback provides a different state than expected";         
    }
    throw std::runtime_error ("Internal error in the function RelayDriver:::errortostring");
}

RelayDriver::Command RelayDriver::stringToCommand (const std::string& c)
{
        if      (c == "off")        return OFF;
        else if (c == "on")         return ON;
        else if (c == "offc")       return OFF_CHECK;
        else if (c == "onc")        return ON_CHECK;
        else if (c == "notc")       return NEGATE_CHECK;
        else if (c == "check")      return CHECK;
        else throw std::invalid_argument ("The given command does not exists: " + c);
}

std::string RelayDriver::commandToString (RelayDriver::Command c)
{
    switch (c)
    {
        case OFF:           return "off";
        case ON:            return "on";
        case OFF_CHECK:     return "offc";
        case ON_CHECK:      return "onc";
        case NEGATE_CHECK:  return "notc";
        case CHECK:         return "check";
        case NONE:          return "none";
        case ERROR:         return "error";
    }
    throw std::runtime_error ("Internal error in the function RelayDriver:::string to command");    
}

RelayDriver::Command RelayDriver::relayFeedbackToCommand (bool relay, bool feedback)
{
    if (    relay   and     feedback) return ON_CHECK;
    if (not relay   and     feedback) return OFF_CHECK;
    if (    relay   and not feedback) return ON;
    if (not relay   and not feedback) return OFF;
    return NONE; //this statement is not reachable
}

bool RelayDriver::commandToBool (RelayDriver::Command c)
{
    if (c == ON)    return true;
    if (c == OFF)   return false;
    throw std::runtime_error ("Unexpected command at RelayDriver::CommandToBool method");
}  
