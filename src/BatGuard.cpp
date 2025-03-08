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
 
#include "BatGuard.hpp"
#include <unistd.h>

const std::string BatGuard::nameVersion {"batguard version 1.3.6 released on 2025/3/08"};

BatGuard::BatGuard (const std::string& cfn) :
    configReader    (   {
                        Configuration ({"!UNIQUE!", "serialpath",       "/dev/ttyRELAY0"}), 
                        Configuration ({"!UNIQUE!", "serialbaud",       "9600"}),
                        Configuration ({"!UNIQUE!", "serialtrials",     "5"}), 
                        Configuration ({"!UNIQUE!", "pollingtime",      "60"}),
                        Configuration ({"!UNIQUE!", "relaychannel",     "1"}),
                        Configuration ({"!UNIQUE!", "batterypath",      "/sys/class/power_supply/BAT0/capacity"}),
                        Configuration ({"!UNIQUE!", "logpath",          "/var/log/batguard.log"}),
                        Configuration ({"!UNIQUE!", "loglevel",         "2"}),
                        Configuration ({"!UNIQUE!", "logflush",         "1",        "10"}),
                        Configuration ({"!UNIQUE!", "logmaxlines",      "1000"}),
                        Configuration ({"!UNIQUE!", "feedback",         "on"}),                        
                        Configuration ({"!UNIQUE!", "commandfilepath",  "/etc/batguard/command"}),
                        Configuration ({"!UNIQUE!", "statefilepath",    "/etc/batguard/state"}),
                        Configuration ({"!UNIQUE!", "chargerno",        "true"}),
                        Configuration ({"!UNIQUE!", "chargerexitlast",  "false"}),
                        Configuration ({"!UNIQUE!", "chargerexitstate", "off"}), 
                        Configuration ({"!UNIQUE!", "keepstate",        "true"}),
                        Configuration ({"!OPTIONAL!",                   "schedule"}),                        
                        Configuration ({"profile"})
                        }, 
                        (cfn.size () ? cfn : "/etc/batguard/config")
                    ),
    serialPort          {configReader.fromConfiguration ("serialpath").getNextString (), configReader.fromConfiguration ("serialbaud").getNextUnsignedInt (), configReader.fromConfiguration ("serialtrials").getNextUnsignedInt8 ()},
    relayDriver         {serialPort, configReader.fromConfiguration ("relaychannel").getNextUnsignedInt8 ()},
    profiles            {},
    userCommand         {configReader.fromConfiguration ("commandfilepath").getNextString (), profiles},
    lastState           {configReader.fromConfiguration ("statefilepath").getNextString (), profiles},
    schedules           {},
    logWriter           {configReader.fromConfiguration ("logpath").getNextString (), configReader.fromConfiguration ("loglevel").getNextUnsignedInt8 (), configReader.fromConfiguration ("logflush").fromValue (0).getNextUnsignedInt8 (), configReader.fromConfiguration ("logflush").fromValue (1).getNextUnsignedInt (), configReader.fromConfiguration ("logmaxlines").getNextUnsignedInt ()},
    capacityReader      {configReader.fromConfiguration ("batterypath").getNextString ()},
    sleepTime           {configReader.fromConfiguration ("pollingtime").getNextUnsignedInt ()},
    checkFeedback       {configReader.fromConfiguration ("feedback").getNextBool ()}, 
    currentProfile      {nullptr},               
    running             {false},
    chargerState        {false},
    profileChanged      {false},
    chargerAtNO         {configReader.fromConfiguration ("chargerno").getNextBool ()},
    relayChannel        {configReader.fromConfiguration ("relaychannel").getNextUnsignedInt8 ()},
    chargerExtLast	    {configReader.fromConfiguration ("chargerexitlast").getNextBool ()},
    chargerExtState	    {configReader.fromConfiguration ("chargerexitstate").getNextBool ()},
    keepState           {configReader.fromConfiguration ("keepstate").getNextBool ()}
{
    loadProfiles ();
    
    loadSchedules ();

    readState ();
}

void BatGuard::readState ()
{
    if (keepState)
    {
        const StateFile::Error err = lastState.read ();
        if (err == StateFile::Error::NO)
        {
            if (lastState.profileInit () != nullptr) currentProfile = lastState.profileInit (); 
            if (lastState.chargerInit () != StateFile::State::LAST) chargerState = StateFile::stateToBool (lastState.chargerInit ());
            if (lastState.schedulerInit () != StateFile::State::LAST) schedules.setEnable (StateFile::stateToBool (lastState.schedulerInit ()));        
        }
        else
        {
            logWriter.writeMessage (LogWriter::Level::ERROR, "It was not possible to read the state file due to the following error: " + StateFile::errorToString (err)); 
        }
    }        
}

void BatGuard::writeState ()
{
    if  (keepState)
    {
        const StateFile::Error err = lastState.read ();        
        
        if (err != StateFile::Error::NO or lastState.isChangedRespectTo (currentProfile, StateFile::boolToState (chargerState), StateFile::boolToState (schedules.isEnabled ()))) 
        {
            const StateFile::Error erw = lastState.write (currentProfile, StateFile::boolToState (chargerState), StateFile::boolToState (schedules.isEnabled ()));
            if (erw != StateFile::Error::NO) logWriter.writeMessage (LogWriter::Level::ERROR, "It was not possible to write the state file due to the following error: " + StateFile::errorToString (erw));
        }        
    }
}

void BatGuard::loadProfiles ()
{
    configReader.selectConfiguration ("profile"); 
        
    //profile is mandatory at least one must be present    
    do
    {
        const std::string   pnm = configReader.getNextString ();
        const uint8_t       cmi = configReader.getNextUnsignedInt8 ();
        const uint8_t       cma = configReader.getNextUnsignedInt8 ();
        const bool          cin = configReader.getNextBool ();
        
        if (configReader.hasMoreValues ()) throw std::invalid_argument ("Profile definition requires 4 arguments instead were found " + std::to_string (configReader.numberOfValues ()) + " at " + configReader.currentConfigurationToString ());
        
        try
        {
            profiles.addProfile (ChargeProfile (pnm, cmi, cma, cin));
        }
        catch (const std::invalid_argument& exc)
        {
            throw std::invalid_argument (exc.what () + std::string (" during the addition of the profile: ") + configReader.currentConfigurationToString ());                                    
        }                        
    }
    while (configReader.gotoNextConfiguration ());   
    
    currentProfile = profiles.getProfileWithIndex (0);         
}

void BatGuard::loadSchedules ()
{
    //schedules are optional, they may miss at all
    if (configReader.selectConfiguration ("schedule")) 
    {
        do
        {
            const bool                ena = configReader.getNextBool ();
            const std::vector <bool>  moy = configReader.getNextListOfBoolDescription (12);
            const std::vector <bool>  dom = configReader.getNextListOfBoolDescription (31);
            const std::vector <bool>  dow = configReader.getNextListOfBoolDescription (7);
            const std::vector <int>   fhm = configReader.getNextListOfInt (2);
            const std::vector <int>   thm = configReader.getNextListOfInt (2);
            const std::string         pnm = configReader.getNextString ();
            
            if (configReader.hasMoreValues ()) throw std::invalid_argument ("Schedule definition requires 7 arguments instead were found " + std::to_string (configReader.numberOfValues ()) + " at " + configReader.currentConfigurationToString ());
            
            try
            {
                schedules.addSchedule (ProfileSchedule (ena, moy, dom, dow, HourMin (fhm), HourMin (thm), profiles.getProfileWithName (pnm)));
            }
            catch (const std::invalid_argument& exc)
            {
                throw std::invalid_argument (exc.what () + std::string (" during the addition of the schedule: ") + configReader.currentConfigurationToString ());
            }        
        }
        while (configReader.gotoNextConfiguration ());
        
        schedules.setEnable (true);
    }    
}

void BatGuard::stop ()
{
    running = false;
}

void BatGuard::start ()
{
    running = true;
    
    logWriter.writeMessage (LogWriter::Level::BASIC, nameVersion + " is going to start");
    
    while (running)
    {        
        selectCurrentProfile ();

        computeChargerState ();
        
        sendRelayCommand ();
        
        if (userCommand.loggerInit ()) logWriter.flushMessages ();
        
        writeState ();
        
        sleep (sleepTime);
    }
    
    if (not chargerExtLast)
    {
		chargerState = chargerExtState;
		
		sendRelayCommand ();
	}
    
    logWriter.writeMessage (LogWriter::Level::BASIC, nameVersion + " is going to stop");
}

void BatGuard::selectCurrentProfile ()
{
    const StateFile::Error usrCmdRd = userCommand.read ();
    
    if (usrCmdRd != StateFile::Error::NO) logWriter.writeMessage (LogWriter::Level::ERROR, "The command file was not correctly formed and will be ignored: " + StateFile::errorToString (usrCmdRd));
    
    //Once the commands are read, the file is emptied
    if (usrCmdRd != StateFile::Error::NO or userCommand.isChangedRespectTo ()) userCommand.write ();

    //this must be done soon to allow the scheduler disable and profile change with a single command 
    if (userCommand.schedulerInit () != StateFile::State::LAST) 
    {                
        schedules.setEnable (StateFile::stateToBool (userCommand.schedulerInit ()));
        logWriter.writeMessage (LogWriter::Level::BASIC, std::string ("The command file required to change the scheduler state to: ") + (userCommand.schedulerInit () == StateFile::State::ON ? "enabled" : "disabled"));                
    }
    
    profileChanged = false;
    const ProfileSchedule* profileSchedTrig = schedules.getScheduleTriggered ();
    const ChargeProfile* profileUserComnd = userCommand.profileInit ();
    if (profileSchedTrig != nullptr)
    {
        if (profileSchedTrig->profile != currentProfile) 
        {
            profileChanged = true;
            currentProfile = profileSchedTrig->profile;
            logWriter.writeMessage (LogWriter::Level::BASIC, "The following schedule triggered: " + profileSchedTrig->toString ());                            
        }
        if (profileUserComnd != nullptr and profileUserComnd != currentProfile) logWriter.writeMessage (LogWriter::Level::ERROR, "Since a schedule is triggering, it was ignored the command file to switch to the profile: " + profileUserComnd->toString () + ", disable the scheduler to force a profile");
    }
    else 
    {            
        if (profileUserComnd != nullptr and profileUserComnd != currentProfile) 
        {
            profileChanged = true;
            currentProfile = profileUserComnd;
            logWriter.writeMessage (LogWriter::Level::BASIC, "The command file required to change the current profile to: " + currentProfile->toString ());                    
        }
    }        
}    

void BatGuard::computeChargerState ()
{        
    const uint8_t charge = capacityReader.readCapacity ();
    
    if (chargerState)
    {
        if (capacityReader.deltaCapacity () < 0) logWriter.writeMessage (LogWriter::Level::ERROR, "The battery capacity is decreasing although the charger is turned on, last capacity variation measured is: " + std::to_string (capacityReader.deltaCapacity ()));
    }
    else
    {
        if (capacityReader.deltaCapacity () > 0) logWriter.writeMessage (LogWriter::Level::ERROR, "The battery capacity is increasing although the charger is turned off, last capacity variation measured is: " + std::to_string (capacityReader.deltaCapacity ()));
    }
        
    if      (charge < currentProfile->minCharge) 
    {            
        if (chargerState == false)  logWriter.writeMessage (LogWriter::Level::BASIC, "Capacity: " + std::to_string (charge) + " went below the min threshold, the charger is turned on");
        else                        logWriter.writeMessage (LogWriter::Level::FULL, "Capacity: " + std::to_string (charge) + " is still below the min threshold, the charger stays on"); 

        if (userCommand.chargerInit () == StateFile::State::OFF) logWriter.writeMessage (LogWriter::Level::ERROR, "Charger-off user-command was ignored because the battery charge is too low, change to a wider charge profile to force the charger state");                                
        
        chargerState = true;
    }
    else if (charge > currentProfile->maxCharge) 
    {            
        if (chargerState == true)   logWriter.writeMessage (LogWriter::Level::BASIC, "Capacity: " + std::to_string (charge) + " went above the max threshold, the charger is turned off");
        else                        logWriter.writeMessage (LogWriter::Level::FULL, "Capacity: " + std::to_string (charge) + " is still above the max threshold, the charger stays off"); 
        
        if (userCommand.chargerInit () == StateFile::State::ON) logWriter.writeMessage (LogWriter::Level::ERROR, "Charger-on user-command was ignored because the battery charge is too high, change to a wider charge profile profile to force the charger state");
        
        chargerState = false;
    }
    //this order of else if is to allows to change profile and set the charger in a single editing of the command file
    else if (userCommand.chargerInit () != StateFile::State::LAST) 
    {
        chargerState = StateFile::stateToBool (userCommand.chargerInit ());
        
        logWriter.writeMessage (LogWriter::Level::BASIC, std::string ("The command file forced the charger to: ") + (chargerState ? "enabled" : "disabled"));
    }
    else if (profileChanged)
    {
        chargerState = currentProfile->startState;
        
        logWriter.writeMessage (LogWriter::Level::BASIC, std::string ("A profile change forced the charger to its initial state: ") + (chargerState ? "enabled" : "disabled"));            
    }
    else
    {
        logWriter.writeMessage (LogWriter::Level::FULL, std::string ("Capacity: " + std::to_string (charge) + " is still between min and max thresholds, the charger stays: " + (chargerState ? "enabled" : "disabled"))); 
    }        
}

void BatGuard::sendRelayCommand ()
{
	const bool relayState = (chargerAtNO == chargerState);
    
	RelayDriver::Command feedback = relayDriver.sendCommand (relayChannel, RelayDriver::relayFeedbackToCommand (relayState, checkFeedback));

	if (feedback == RelayDriver::Command::ERROR)
	{
		logWriter.writeMessage (LogWriter::Level::ERROR, std::string ("There was an error setting the charger to: ") + (chargerExtState ? "enabled" : "disabled") + ", the command sent to the relay returned the error: " + RelayDriver::errorToString (relayDriver.lastError ()));
		
        //retry to send the last command without feedback because it is ignored
		relayDriver.sendCommand (relayChannel, RelayDriver::relayFeedbackToCommand (relayState, false));
	}
	else if (checkFeedback and RelayDriver::commandToBool (feedback) != relayState) 
	{
		logWriter.writeMessage (LogWriter::Level::ERROR, std::string ("There was an error setting the charger to: ") + (chargerExtState ? "enabled" : "disabled") + ", the relay feedback was: " + RelayDriver::commandToString (feedback) + " does not match its required state: " + (relayState ? "enabled" : "disabled"));
		
        //retry to send the last command without feedback because it is ignored
		relayDriver.sendCommand (relayChannel, RelayDriver::relayFeedbackToCommand (relayState, false));
	}
}	

bool BatGuard::isRunning () const
{
    return running;
}

bool BatGuard::logMessage (const std::string& mes)
{
    return logWriter.writeMessage (LogWriter::Level::ERROR, mes);
}

std::string BatGuard::sendCommandRelay (const std::string & cmd)
{
    RelayDriver::Command c = relayDriver.sendCommand (relayChannel, RelayDriver::stringToCommand (cmd));
    
    if (c != RelayDriver::Command::ERROR) return RelayDriver::commandToString (c);
    
    return RelayDriver::errorToString (relayDriver.lastError ());
}

std::string BatGuard::getBatteryCapacity () const
{
    return capacityReader.toString ();
}

std::string BatGuard::getChargeProfiles () const
{
    return profiles.toString ();
}

std::string BatGuard::getUserCommand ()
{
    StateFile::Error err = userCommand.read ();
    if (err == StateFile::Error::NO) return userCommand.toString ();
    return "Error reading the command file: " + StateFile::errorToString (err);
}

std::string BatGuard::getLastState ()
{
    StateFile::Error err = lastState.read ();
    if (err == StateFile::Error::NO) return lastState.toString ();
    return "Error reading the state file: " + StateFile::errorToString (err);
}

std::string BatGuard::getProfileSchedules () const
{
    std::string output;
    if (schedules.getNumOfSchedules ()) output += schedules.toString ();
    else output += "No schedule is defined\n";
    
    const ProfileSchedule* st = schedules.getScheduleTriggered ();
    if (st != nullptr) output += "There is a schedule triggering: " + st->toString ();
    else output += "No schedule is triggering.";
    return output;
}

