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
#include <iostream>
#include <stdexcept>
#include <signal.h>
#include <iostream>

BatGuard* batGuardPtr = nullptr;

void signalHandler (int signal) 
{
    switch (signal)
    {
        case SIGTERM:
        case SIGINT:
            if (batGuardPtr and batGuardPtr -> isRunning ()) batGuardPtr -> stop ();
            else exit (0);
            break;
        case SIGHUP:
            std::cout << "SIGHUP signal is not supported by batguard, please restart batguard to reload the configuration: sudo systemctl restart batguard";
            break;
    }
}
  
int main (int argc, char** argv)
{
    signal (SIGINT,     signalHandler); 
    signal (SIGTERM,    signalHandler); 
    signal (SIGHUP,     signalHandler); 
    
    std::string configFile {""};
    std::string relayCommand;
    std::string logMessage;
    bool        printBattery = false;
    bool        printProfiles = false;
    bool        printSchedules = false;
    bool        printUserCommand = false;
    bool        printLastState = false;
    bool        quit = false;
    
    int opt;
    while ((opt = getopt (argc, argv, "c:r:l:bpqusvht")) != -1)
    {
        switch (opt)
        {
            case 'c':
                configFile = std::string (optarg);
                break;
            case 'r':
                relayCommand = std::string (optarg);
                break;
            case 'l':
                logMessage = std::string (optarg);
                break;
            case 'b':
                printBattery = true;
                break;
            case 'p':
                printProfiles = true;
                break;
            case 'q':
                quit = true;
                break;
            case 'u':
                printUserCommand = true;
                break;
            case 't':
                printLastState = true;
                break;                
            case 's':
                printSchedules = true;
                break;
            case 'v':
                std::cout << BatGuard::nameVersion << '\n';
                std::cout << "Copyright (C) 2025 Simone Pernice pernice@libero.it\n";
                std::cout << "Released under GPL 3.0 licence\n";
                return 0;
                break;
            case 'h':
                std::cout << "Available options: \n";
                std::cout << "-c config_file_path   (read the configuration file from the given path instead of the default /etc/batguard/config)\n";
                std::cout << "-r relay_command      (send one of the following commands to the relay: off, on, offc, onc, notc, check where the ending 'c' stands for check feedback)\n";
                std::cout << "-l log_message        (write an ERROR-level message into the log file as far as the log is enabled)\n";
                std::cout << "-b                    (print the battery capacity)\n";
                std::cout << "-p                    (print the list of capacity profiles loaded from the configuration file)\n";
                std::cout << "-s                    (print the list of profile schedules loaded from the configuration file)\n";
                std::cout << "-v                    (print the batguard version)\n";
                std::cout << "-u                    (print the command file content)\n";
                std::cout << "-t                    (print the state file content)\n";
                std::cout << "-q                    (read the configuration file then quit without entering the main loop)\n";
                std::cout << "-h                    (print this help)\n";                
                std::cout << '\n';
                return 0;
            case '?':
                std::cout << "Command not understood. Run with -h option to print the available options.\n";
                return 1;
            default :
                return 1;
        }
    }
    
    try 
    {
        BatGuard batGuard (configFile);
        batGuardPtr = & batGuard;
        
        if (relayCommand.size ())   std::cout << "Relay feedback: "         << batGuard.sendCommandRelay (relayCommand) << '\n';
        
        if (logMessage.size ())     std::cout << "Log message result: "     << batGuard.logMessage (logMessage) << '\n';
        
        if (printBattery)           std::cout << "Battery capacity: "       << batGuard.getBatteryCapacity () << '\n';
        
        if (printProfiles)          std::cout << "Charge profiles:\n"       << batGuard.getChargeProfiles () << '\n';
        
        if (printSchedules)         std::cout << "Profile schedules:\n"     << batGuard.getProfileSchedules () << '\n';
        
        if (printUserCommand)       std::cout << "Command file content: "   << batGuard.getUserCommand () << '\n';    
        
        if (printLastState)         std::cout << "State file content: "     << batGuard.getLastState () << '\n';   
        
        
        if (relayCommand.size () or logMessage.size () or printBattery or printProfiles or printSchedules or printUserCommand or printLastState or quit) return 0;
        
        batGuard.start ();        
    }
    catch (const std::invalid_argument & e)
    {
        std::cout << "Argument error: " << e.what () << " .\n";
        return 1;
    }
    catch (const std::runtime_error & e)
    {
        std::cout << "Internal error, send the following message to the author: " << e.what () << " .\n";
        return 2;
    }
    catch (...)
    {
        std::cout << "Unknow error.\n";
        return 3;
    }
        
    return 0;
}
