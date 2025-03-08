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

#include <catch2/catch_test_macros.hpp>

#include <unistd.h>
#include <fstream>

#include "ConfigReader.hpp"
#include "SerialPort.hpp"
#include "RelayDriver.hpp"
#include "CapacityReader.hpp"
#include "LogWriter.hpp"
#include "ChargeProfiles.hpp"
#include "StateFile.hpp"
#include "ProfileSchedules.hpp"
#include "stringtools.hpp"

TEST_CASE( "ConfigReader", "[configuration]" ) 
{
    ConfigReader cr ({
                        Configuration ({"!UNIQUE!", "single", "a param"}), 
                        Configuration ({"!UNIQUE!", "double", "p1", "p2"}), 
                        Configuration ({"!UNIQUE!", "numbers"}), 
                        Configuration ({"param"}), 
                        Configuration ({"!OPTIONAL!", "lists"}),
                        Configuration ({"!UNIQUE!", "default", "pippo"})
                    }, "../tests/configtest.conf");
    
    REQUIRE (cr.selectConfiguration ("single") == true);
    REQUIRE (cr.hasMoreValues () == true);
    REQUIRE (cr.getNextString () == "one parameter"); 
    REQUIRE (cr.hasMoreValues () == false);
    REQUIRE (cr.selectConfiguration ("single") == true);
    REQUIRE (cr.selectConfiguration ("single") == true);
    REQUIRE (cr.gotoNextConfiguration () == false);
    
    REQUIRE (cr.selectConfiguration ("double") == true);
    REQUIRE (cr.hasMoreValues () == true);
    REQUIRE (cr.getNextString () == "first"); 
    REQUIRE (cr.hasMoreValues () == true);
    REQUIRE (cr.getNextString () == "second"); 
    REQUIRE (cr.hasMoreValues () == false);
    REQUIRE (cr.fromValue (0).getNextString () == "first");
    REQUIRE (cr.getNextString () == "second");
    REQUIRE (cr.hasMoreValues () == false); 
    REQUIRE (cr.selectConfiguration ("double") == true);
    REQUIRE (cr.selectConfiguration ("double") == true);    
    REQUIRE (cr.gotoNextConfiguration () == false);
    
    REQUIRE (cr.selectConfiguration ("numbers") == true);
    REQUIRE (cr.hasMoreValues () == true);
    REQUIRE (cr.getNextInt () == 12); 
    REQUIRE (cr.hasMoreValues () == true);
    REQUIRE (cr.getNextInt () == 14); 
    REQUIRE (cr.hasMoreValues () == true);
    REQUIRE (cr.getNextInt () == 155); 
    REQUIRE (cr.hasMoreValues () == true);
    REQUIRE (cr.getNextInt () == -123); 
    REQUIRE (cr.hasMoreValues () == false);   
    REQUIRE (cr.fromValue (1).getNextInt () == 14);
    REQUIRE (cr.fromValue (3).getNextInt () == -123);
    REQUIRE (cr.selectConfiguration ("numbers") == true);    
    REQUIRE (cr.selectConfiguration ("numbers") == true);
    REQUIRE (cr.gotoNextConfiguration () == false);
    
    REQUIRE (cr.selectConfiguration ("param") == true);
    REQUIRE (cr.numberOfValues () == 3);
    REQUIRE (cr.hasMoreValues () == true);
    REQUIRE (cr.getNextString () == "a"); 
    REQUIRE (cr.numberOfValues () == 3);
    REQUIRE (cr.hasMoreValues () == true);
    REQUIRE (cr.getNextInt () == 1); 
    REQUIRE (cr.numberOfValues () == 3);
    REQUIRE (cr.hasMoreValues () == true);
    REQUIRE (cr.getNextString () == "b");          
    REQUIRE (cr.numberOfValues () == 3);
    REQUIRE (cr.hasMoreValues () == false);    

    REQUIRE (cr.selectConfiguration ("param") == true);
    REQUIRE (cr.numberOfValues () == 3);
    REQUIRE (cr.hasMoreValues () == true);
    REQUIRE (cr.getNextString () == "a"); 
    REQUIRE (cr.numberOfValues () == 3);
    REQUIRE (cr.hasMoreValues () == true);
    REQUIRE (cr.getNextInt () == 1); 
    REQUIRE (cr.numberOfValues () == 3);
    REQUIRE (cr.hasMoreValues () == true);
    REQUIRE (cr.getNextString () == "b");          
    REQUIRE (cr.numberOfValues () == 3);
    REQUIRE (cr.hasMoreValues () == false);    

    REQUIRE (cr.gotoNextConfiguration () == true);
    REQUIRE (cr.numberOfValues () == 3);
    REQUIRE (cr.hasMoreValues () == true);
    REQUIRE (cr.getNextString () == "c"); 
    REQUIRE (cr.numberOfValues () == 3);
    REQUIRE (cr.hasMoreValues () == true);
    REQUIRE (cr.getNextInt () == 2); 
    REQUIRE (cr.numberOfValues () == 3);
    REQUIRE (cr.hasMoreValues () == true);
    REQUIRE (cr.getNextString () == "d");          
    REQUIRE (cr.numberOfValues () == 3);
    REQUIRE (cr.hasMoreValues () == false); 
       
    REQUIRE (cr.selectConfiguration ("param") == true);
    REQUIRE (cr.selectConfiguration ("param") == true);
    REQUIRE (cr.selectConfiguration ("param") == true);    
    REQUIRE (cr.selectConfiguration ("param") == true);
    
    REQUIRE (cr.selectConfiguration ("single") == true);
    REQUIRE (cr.hasMoreValues () == true);
    REQUIRE (cr.getNextString () == "one parameter"); 
    REQUIRE (cr.hasMoreValues () == false);
    REQUIRE (cr.selectConfiguration ("single") == true);           
    
    REQUIRE (cr.selectConfiguration ("default") == true);
    REQUIRE (cr.numberOfValues () == 1);
    REQUIRE (cr.hasMoreValues () == true);
    REQUIRE (cr.getNextString () == "pippo");
    REQUIRE (cr.hasMoreValues () == false);    
    
    REQUIRE (cr.fromConfiguration ("numbers").getNextInt () == 12);
    REQUIRE (cr.getNextInt () == 14);
    REQUIRE (cr.getNextInt () == 155);
    REQUIRE (cr.getNextInt () == -123);
  
    REQUIRE (cr.selectConfiguration ("lists") == true);
    REQUIRE (cr.getNextListOfString () == std::vector <std::string> {"a", "b", "c"});
    REQUIRE (cr.getNextListOfInt () == std::vector <int> {1, 2, 3});
    REQUIRE (cr.getNextListOfBool () == std::vector <bool> {true, false, true, false, true, false});
    
    REQUIRE (cr.gotoNextConfiguration () == true);    
    REQUIRE (cr.getNextListOfBoolDescription (5,1) == std::vector <bool> {true,  true, false, true,  true});
    REQUIRE (cr.getNextListOfBoolDescription (5,1) == std::vector <bool> {false, true, true,  true,  false});
    REQUIRE (cr.getNextListOfBoolDescription (5,0) == std::vector <bool> {false, true, false, false, true});
    REQUIRE (cr.getNextListOfBoolDescription (5,0) == std::vector <bool> {true,  true, true,  true,  true});

    REQUIRE (cr.gotoNextConfiguration () == true);    
    REQUIRE (cr.getNextListOfBoolDescription (5,1) == std::vector <bool> {true,  true,  false, true,  true});
    REQUIRE (cr.getNextListOfBoolDescription (5,1) == std::vector <bool> {false, false, true,  false, true});    
    REQUIRE (cr.getNextListOfBoolDescription (5,1) == std::vector <bool> {true,  false, true,  true,  true});        
}

TEST_CASE("SerialPort", "[serial]") 
{
    // Start socat in the background
    REQUIRE (system("socat -d2 PTY,link=/tmp/ttyS10,raw,echo=0 PTY,link=/tmp/ttyS11,raw,echo=0 &") == 0);
    sleep (1); // Give socat time to start
    
    SerialPort wr ("/tmp/ttyS10", 9600);
    SerialPort rd ("/tmp/ttyS11", 9600);

    REQUIRE (rd.readFlush () == 0);
    
    REQUIRE (wr.writeByte (71) == true);
    REQUIRE (wr.writeByte (17) == true);
    REQUIRE (wr.writeByte (65) == true);
    REQUIRE (wr.writeByte (29) == true);                
    REQUIRE (wr.writeFlush () == 4);
    
    REQUIRE (rd.readFlush () == 4);    
    REQUIRE (rd.readByte () == 71);
    REQUIRE (rd.readByte () == 17);    
        
    REQUIRE (wr.writeByte (12) == true);  
    REQUIRE (wr.writeByte (34) == true);  
    REQUIRE (wr.writeByte (99) == true);
    REQUIRE (wr.writeFlush () == 3);
      
    REQUIRE (rd.readFlush () == 5);    
    REQUIRE (rd.readByte () == 65);
    REQUIRE (rd.readByte () == 29);
    REQUIRE (rd.bytesToRead () == 3);
    REQUIRE (rd.readByte () == 12);
    REQUIRE (rd.bytesToRead () == 2);
    REQUIRE (rd.readFlush () == 2);
    REQUIRE (rd.readByte () == 34);
    REQUIRE (rd.readByte () == 99);

    REQUIRE (wr.writeByte (38) == true);
    REQUIRE (wr.writeByte (39) == true);                
    REQUIRE (wr.writeFlush () == 2);
            
    REQUIRE (rd.bytesToRead () == 0);
    REQUIRE (rd.readFlush () == 2);
    REQUIRE (rd.readByte () == 38);
    REQUIRE (rd.bytesToRead () == 1);    
    REQUIRE (rd.readByte () == 39);
    REQUIRE (rd.bytesToRead () == 0);    
    REQUIRE (rd.readFlush () == 0);
    REQUIRE (rd.bytesToRead () == 0);
    
    //Kill the socat process
    REQUIRE(system("pkill socat") == 0);
}


TEST_CASE("RelayDriver", "[serial]") 
{
    // Start socat in the background
    REQUIRE (system("socat -d2 PTY,link=/tmp/ttyS10,raw,echo=0 PTY,link=/tmp/ttyS11,raw,echo=0 &") == 0);
    sleep (1); // Give socat time to start
    
    SerialPort spcmp ("/tmp/ttyS10", 9600);
    SerialPort sprel ("/tmp/ttyS11", 9600);
    
    RelayDriver urd (spcmp, 8, 1);
    sprel.readFlush ();
    while (sprel.bytesToRead ()) sprel.readByte (); //to clean the commands send by the driver to check the status of the relays
    
    //turn on relay 1
    REQUIRE (urd.sendCommand (1, RelayDriver::Command::ON) == RelayDriver::Command::NONE);
    REQUIRE (urd.lastError () == RelayDriver::Error::NO);

    REQUIRE (sprel.readFlush () == 4);
    REQUIRE (sprel.readByte () == 0xA0);
    REQUIRE (sprel.readByte () == 0x01);
    REQUIRE (sprel.readByte () == 0x01);
    REQUIRE (sprel.readByte () == 0xA2);

    //turn off relay 2
    REQUIRE (urd.sendCommand (2, RelayDriver::Command::OFF) == RelayDriver::Command::NONE);
    REQUIRE (urd.lastError () == RelayDriver::Error::NO);

    REQUIRE (sprel.readFlush () == 4);
    REQUIRE (sprel.readByte () == 0xA0);
    REQUIRE (sprel.readByte () == 0x02);
    REQUIRE (sprel.readByte () == 0x00);
    REQUIRE (sprel.readByte () == 0xA2);

    //turn on relay 3
    REQUIRE (urd.sendCommand (3, RelayDriver::Command::ON) == RelayDriver::Command::NONE);
    REQUIRE (urd.lastError () == RelayDriver::Error::NO);

    REQUIRE (sprel.readFlush () == 4);
    REQUIRE (sprel.readByte () == 0xA0);
    REQUIRE (sprel.readByte () == 0x03);
    REQUIRE (sprel.readByte () == 0x01);
    REQUIRE (sprel.readByte () == 0xA4);    

    //turn on relay 4
    REQUIRE (urd.sendCommand (4, RelayDriver::Command::ON) == RelayDriver::Command::NONE);
    REQUIRE (urd.lastError () == RelayDriver::Error::NO);

    REQUIRE (sprel.readFlush () == 4);
    REQUIRE (sprel.readByte () == 0xA0);
    REQUIRE (sprel.readByte () == 0x04);
    REQUIRE (sprel.readByte () == 0x01);
    REQUIRE (sprel.readByte () == 0xA5);    

    //turn on and check relay 1
    REQUIRE (sprel.writeByte (0xA0) == true);
    REQUIRE (sprel.writeByte (0x01) == true);
    REQUIRE (sprel.writeByte (0x01) == true);
    REQUIRE (sprel.writeByte (0xA2) == true);    
    REQUIRE (sprel.writeFlush () == 4);
        
    REQUIRE (urd.sendCommand (1, RelayDriver::Command::ON_CHECK) == RelayDriver::Command::ON);
    REQUIRE (urd.lastError () == RelayDriver::Error::NO);

    REQUIRE (sprel.readFlush () == 4);
    REQUIRE (sprel.readByte () == 0xA0);
    REQUIRE (sprel.readByte () == 0x01);
    REQUIRE (sprel.readByte () == 0x03);
    REQUIRE (sprel.readByte () == 0xA4);

    //turn off and check relay 3
    REQUIRE (sprel.writeByte (0xA0) == true);
    REQUIRE (sprel.writeByte (0x03) == true);
    REQUIRE (sprel.writeByte (0x00) == true);
    REQUIRE (sprel.writeByte (0xA3) == true);    
    REQUIRE (sprel.writeFlush () == 4);
    
    REQUIRE (urd.sendCommand (3, RelayDriver::Command::OFF_CHECK) == RelayDriver::Command::OFF);
    REQUIRE (urd.lastError () == RelayDriver::Error::NO);

    REQUIRE (sprel.readFlush () == 4);
    REQUIRE (sprel.readByte () == 0xA0);
    REQUIRE (sprel.readByte () == 0x03);
    REQUIRE (sprel.readByte () == 0x02);
    REQUIRE (sprel.readByte () == 0xA5);
  
    //turn on and check relay 2 with wrong relay state answer
    REQUIRE (sprel.writeByte (0xA0) == true);
    REQUIRE (sprel.writeByte (0x02) == true);
    REQUIRE (sprel.writeByte (0x00) == true);
    REQUIRE (sprel.writeByte (0xA2) == true);    
    REQUIRE (sprel.writeFlush () == 4);
    
    REQUIRE (urd.sendCommand (2, RelayDriver::Command::ON_CHECK) == RelayDriver::Command::ERROR);
    REQUIRE (urd.lastError () == RelayDriver::Error::WRNGSTA);

    REQUIRE (sprel.readFlush () == 4);
    REQUIRE (sprel.readByte () == 0xA0);
    REQUIRE (sprel.readByte () == 0x02);
    REQUIRE (sprel.readByte () == 0x03);
    REQUIRE (sprel.readByte () == 0xA5);
    
    //turn off and check relay 5 with wrong crc answer
    REQUIRE (sprel.writeByte (0xA0) == true);
    REQUIRE (sprel.writeByte (0x05) == true);
    REQUIRE (sprel.writeByte (0x00) == true);
    REQUIRE (sprel.writeByte (0xA6) == true);    
    REQUIRE (sprel.writeFlush () == 4);
    
    REQUIRE (urd.sendCommand (5, RelayDriver::Command::OFF_CHECK) == RelayDriver::Command::ERROR);
    REQUIRE (urd.lastError () == RelayDriver::Error::WRNGCRC);

    REQUIRE (sprel.readFlush () == 4);
    REQUIRE (sprel.readByte () == 0xA0);
    REQUIRE (sprel.readByte () == 0x05);
    REQUIRE (sprel.readByte () == 0x02);
    REQUIRE (sprel.readByte () == 0xA7);    
    
    //turn on and check relay 6 with wrong relay
    REQUIRE (sprel.writeByte (0xA0) == true);
    REQUIRE (sprel.writeByte (0x05) == true);
    REQUIRE (sprel.writeByte (0x01) == true);
    REQUIRE (sprel.writeByte (0xA6) == true);    
    REQUIRE (sprel.writeFlush () == 4);
    
    REQUIRE (urd.sendCommand (6, RelayDriver::Command::ON_CHECK) == RelayDriver::Command::ERROR);
    REQUIRE (urd.lastError () == RelayDriver::Error::WRNGREL);

    REQUIRE (sprel.readFlush () == 4);
    REQUIRE (sprel.readByte () == 0xA0);
    REQUIRE (sprel.readByte () == 0x06);
    REQUIRE (sprel.readByte () == 0x03);
    REQUIRE (sprel.readByte () == 0xA9);  
    
    //called with wrong command
    REQUIRE (urd.sendCommand (6, RelayDriver::Command::NONE) == RelayDriver::Command::ERROR);
    REQUIRE (urd.lastError () == RelayDriver::Error::WRNGCOM);

    REQUIRE (sprel.readFlush () == 0);        

    //called with wrong command
    REQUIRE (urd.sendCommand (6, RelayDriver::Command::ERROR) == RelayDriver::Command::ERROR);
    REQUIRE (urd.lastError () == RelayDriver::Error::WRNGCOM);

    REQUIRE (sprel.readFlush () == 0);        
    
    //turn on and check relay 9 with extra unuseful feedback data
    REQUIRE (sprel.writeByte (0xA0) == true);
    REQUIRE (sprel.writeByte (0xA0) == true);
    REQUIRE (sprel.writeByte (0xA0) == true);
    REQUIRE (sprel.writeByte (0xA0) == true);
    REQUIRE (sprel.writeByte (0x09) == true);
    REQUIRE (sprel.writeByte (0x01) == true);
    REQUIRE (sprel.writeByte (0xAA) == true);    
    REQUIRE (sprel.writeFlush () == 7);
    
    REQUIRE (urd.sendCommand (9, RelayDriver::Command::ON_CHECK) == RelayDriver::Command::ON);
    REQUIRE (urd.lastError () == RelayDriver::Error::NO);

    REQUIRE (sprel.readFlush () == 4);
    REQUIRE (sprel.readByte () == 0xA0);
    REQUIRE (sprel.readByte () == 0x09);
    REQUIRE (sprel.readByte () == 0x03);
    REQUIRE (sprel.readByte () == 0xAC);      
    
    //turn off and check relay 8 with fewer bytes than required 
    REQUIRE (sprel.writeByte (0xA0) == true);
    REQUIRE (sprel.writeByte (0x08) == true);
    REQUIRE (sprel.writeFlush () == 2);
    
    REQUIRE (urd.sendCommand (8, RelayDriver::Command::OFF_CHECK) == RelayDriver::Command::ERROR);
    REQUIRE (urd.lastError () == RelayDriver::Error::NORECV);

    REQUIRE (sprel.readFlush () == 4);
    REQUIRE (sprel.readByte () == 0xA0);
    REQUIRE (sprel.readByte () == 0x08);
    REQUIRE (sprel.readByte () == 0x02);
    REQUIRE (sprel.readByte () == 0xAA);    

    REQUIRE (sprel.writeByte (0x00) == true);
    REQUIRE (sprel.writeByte (0xA8) == true);    
    REQUIRE (sprel.writeFlush () == 2);      

    REQUIRE (urd.recvCommand () == RelayDriver::Command::OFF);
    REQUIRE (urd.lastError () == RelayDriver::Error::NO);

    //Kill the socat process
    REQUIRE(system("pkill socat") == 0);
}

TEST_CASE("CapacityReader", "[file]") 
{
    
    std::ofstream cw ("./capacity");
    
    cw.seekp(0, std::ios::beg);
    cw << "0\n";
    cw.flush ();
    
    CapacityReader cr ("./capacity");
    
    REQUIRE (cr.readCapacity () == 0);
    REQUIRE (cr.readCapacity () == 0);
           
    cw.seekp(0, std::ios::beg);       
    cw << "32\n";
    cw.flush ();
    
    REQUIRE (cr.readCapacity () == 32);
    REQUIRE (cr.deltaCapacity () == 32);
    
    REQUIRE (cr.readCapacity () == 32);
    REQUIRE (cr.deltaCapacity () == 0);
    
    cw.seekp(0, std::ios::beg);
    cw << "47\n";    
    cw.flush ();
    
    REQUIRE (cr.readCapacity () == 47);
    REQUIRE (cr.deltaCapacity () == 15);
    
    REQUIRE (cr.readCapacity () == 47);
    REQUIRE (cr.deltaCapacity () == 0);
    
    cw.seekp(0, std::ios::beg);
    cw << "89\n";    
    cw.flush ();
    
    REQUIRE (cr.readCapacity () == 89);
    REQUIRE (cr.deltaCapacity () == 42);
    
    REQUIRE (cr.readCapacity () == 89); 
    REQUIRE (cr.deltaCapacity () == 0);
    
    cw.seekp(0, std::ios::beg);
    cw << "100\n";    
    cw.flush ();
    
    REQUIRE (cr.readCapacity () == 100);
    REQUIRE (cr.deltaCapacity () == 11);
    
    REQUIRE (cr.readCapacity () == 100);   
    REQUIRE (cr.deltaCapacity () == 0);
    
    cw.close ();
}     

TEST_CASE("LogWriter", "[file]") 
{
    int index = 0;
    while (true) //set true to delete previous log file
    {
        std::string name ("./log.txt");
        if (index) name += '.' + std::to_string (index);
        std::ifstream ifs (name);
        if (not ifs.is_open ()) break;
        ifs.close ();
        if (remove (name.c_str ())) FAIL ("It was not possible to delete the old log file called: " + name);
        ++ index;
    }
     
    LogWriter lo ("./log.txt", 2, 0, 1, 3);
    
    REQUIRE (lo.getNumLines () == 0);
    
    REQUIRE (lo.writeMessage (LogWriter::Level::BASIC, "Just turned on 1") == true);
    REQUIRE (lo.getNumLines () == 1);
    
    REQUIRE (lo.writeMessage (LogWriter::Level::ERROR, "a new message 2") == true);
    REQUIRE (lo.getNumLines () == 2);
    
    REQUIRE (lo.writeMessage (LogWriter::Level::FULL, "last log") == false);
    
    REQUIRE (lo.writeMessage (LogWriter::Level::FULL, "this will not be written") == false);
    REQUIRE (lo.writeMessage (LogWriter::Level::FULL, "neither this will go out") == false);

    std::ifstream testSplit;
    testSplit.open ("./log.txt.1");
    REQUIRE (testSplit.is_open () == false);
    
    REQUIRE (lo.writeMessage (LogWriter::Level::ERROR, "a new message 3") == true);
    REQUIRE (lo.getNumLines () == 0);

    testSplit.open ("./log.txt.1");
    REQUIRE (testSplit.is_open () == true);
    testSplit.close ();
    
    REQUIRE (lo.writeMessage (LogWriter::Level::ERROR, "a new message 4") == true);
    REQUIRE (lo.getNumLines () == 1);    
    
    REQUIRE (lo.writeMessage (LogWriter::Level::ERROR, "a new message 5") == true);
    REQUIRE (lo.getNumLines () == 2);        

    testSplit.open ("./log.txt.2");
    REQUIRE (testSplit.is_open () == false);
    
    REQUIRE (lo.writeMessage (LogWriter::Level::ERROR, "a new message 6") == true);
    REQUIRE (lo.getNumLines () == 0);            
    
    testSplit.open ("./log.txt.2");
    REQUIRE (testSplit.is_open () == true);
    testSplit.close ();    
    
    REQUIRE (lo.writeMessage (LogWriter::Level::ERROR, "a new message 7") == true);
    REQUIRE (lo.getNumLines () == 1);       
}

TEST_CASE("ChargeProfiles", "[charge]") 
{
    ChargeProfiles cp;
    
    cp.addProfile (ChargeProfile ("home", 40, 60, false));
    
    cp.addProfile (ChargeProfile ("trip", 80, 100, true));
    cp.addProfile (ChargeProfile ("maytrip", 60, 80, true));
    
    REQUIRE_THROWS (cp.addProfile (ChargeProfile ("home", 40, 60, false)));
    REQUIRE_THROWS (cp.addProfile (ChargeProfile ("test", 30, 10, false))); 
    REQUIRE_THROWS (cp.addProfile (ChargeProfile ("test", 50, 110, false))); 
    REQUIRE_THROWS (cp.addProfile (ChargeProfile ("test", -5, 90, false))); 
    REQUIRE_THROWS (cp.addProfile (ChargeProfile ("new ", 60, 80, false))); 
    
    INFO ("Charge profiles :\n" << cp.toString ());  
    
    REQUIRE (cp.numberOfProfiles () == 3);
    
    REQUIRE (cp.getProfileWithName ("test") == nullptr);
    
    const ChargeProfile* c;
    
    c = cp.getProfileWithIndex (0); 
    REQUIRE (c->name == "home");
    REQUIRE (c->minCharge == 40);
    REQUIRE (c->maxCharge == 60);
    REQUIRE (c->startState == false);  
    
    REQUIRE (cp.getProfileWithIndex (10) == nullptr); 
    
    c = cp.getProfileWithName ("home");
    REQUIRE (c->name == "home");
    REQUIRE (c->minCharge == 40);
    REQUIRE (c->maxCharge == 60);
    REQUIRE (c->startState == false);
    
    c = cp.getProfileWithName ("maytrip"); 
    REQUIRE (c->name == "maytrip");
    REQUIRE (c->minCharge == 60);
    REQUIRE (c->maxCharge == 80);
    REQUIRE (c->startState == true);    
    
    
    c = cp.getProfileWithIndex (2); 
    REQUIRE (c->name == "maytrip");
    REQUIRE (c->minCharge == 60);
    REQUIRE (c->maxCharge == 80);
    REQUIRE (c->startState == true);      
    
    REQUIRE (cp.getProfileWithName ("hao") == nullptr);
    
    c = cp.getProfileWithName ("trip");
    REQUIRE (c->name == "trip");
    REQUIRE (c->minCharge == 80);
    REQUIRE (c->maxCharge == 100);
    REQUIRE (c->startState == true); 
    
    c = cp.getProfileWithIndex (1); 
    REQUIRE (c->name == "trip");
    REQUIRE (c->minCharge == 80);
    REQUIRE (c->maxCharge == 100);
    REQUIRE (c->startState == true); 
        
    c = cp.getProfileWithName ("home");
    REQUIRE (c->name == "home");
    REQUIRE (c->minCharge == 40);
    REQUIRE (c->maxCharge == 60);
    REQUIRE (c->startState == false);
}

TEST_CASE("StateFile", "[file]") 
{
    ChargeProfiles cp;
    cp.addProfile (ChargeProfile ("home", 40, 60, false));
    cp.addProfile (ChargeProfile ("trip", 80, 100, true));
    cp.addProfile (ChargeProfile ("maytrip", 60, 80, true));
    
    remove ("./runfile");   
    StateFile rfm ("./runfile", cp);  
    
    REQUIRE (rfm.profileInit () == nullptr);
    REQUIRE (rfm.chargerInit () == StateFile::State::LAST);
    REQUIRE (rfm.schedulerInit () == StateFile::State::LAST);
    REQUIRE (rfm.loggerInit () == false);
    REQUIRE (rfm.isChangedRespectTo (cp.getProfileWithName ("home")) == true);
    
    REQUIRE (rfm.write (cp.getProfileWithName ("home")) == StateFile::Error::NO);
        
    std::string line;
    std::ifstream rfr ("./runfile");    
    getline (rfr, line);
    rfr.close ();
    trim (line);
    REQUIRE (line == "home");
    
    REQUIRE (rfm.read () == StateFile::Error::NO);    
    REQUIRE (rfm.chargerInit () == StateFile::State::LAST);
    REQUIRE (rfm.schedulerInit () == StateFile::State::LAST);
    REQUIRE (rfm.profileInit () == cp.getProfileWithName ("home"));
    REQUIRE (rfm.loggerInit () == false);
    REQUIRE (rfm.isChangedRespectTo (cp.getProfileWithName ("home")) == false);
    
    REQUIRE (rfm.write (cp.getProfileWithName ("home")) == StateFile::Error::NO);
    
    rfr.open ("./runfile");    
    getline (rfr, line);
    rfr.close ();
    trim (line);
    REQUIRE (line == "home");

    std::ofstream rfw ("./runfile");
    rfw << "trip #loggerflush";
    rfw.close ();
    
    rfr.open ("./runfile");    
    getline (rfr, line);
    rfr.close ();
    trim (line);
    REQUIRE (line == "trip #loggerflush");    
    
    REQUIRE (rfm.read () == StateFile::Error::NO);
    REQUIRE (rfm.chargerInit () == StateFile::State::LAST);
    REQUIRE (rfm.schedulerInit () == StateFile::State::LAST);    
    REQUIRE (rfm.profileInit () == cp.getProfileWithName ("trip"));
    REQUIRE (rfm.loggerInit () == true);
    REQUIRE (rfm.isChangedRespectTo (cp.getProfileWithName ("trip")) == true);
    
    REQUIRE (rfm.write (cp.getProfileWithName ("trip")) == StateFile::Error::NO);
    
    rfr.open ("./runfile");    
    getline (rfr, line);
    rfr.close ();
    trim (line);
    REQUIRE (line == "trip");
    
    rfw.open ("./runfile");
    rfw << "test #loggerflush";
    rfw.close ();
    
    REQUIRE (rfm.read () == StateFile::Error::PROFILE);
    REQUIRE (rfm.chargerInit () == StateFile::State::LAST);
    REQUIRE (rfm.schedulerInit () == StateFile::State::LAST);    
    REQUIRE (rfm.profileInit () == nullptr);
    REQUIRE (rfm.loggerInit () == false);
    REQUIRE (rfm.isChangedRespectTo (cp.getProfileWithName ("trip")) == true);
    
    REQUIRE (rfm.write (cp.getProfileWithName ("trip")) == StateFile::Error::NO);
    
    rfr.open ("./runfile");
    getline (rfr, line);
    rfr.close ();
    trim (line);
    REQUIRE (line == "trip");
    
    remove ("./runfile");

    REQUIRE (rfm.read () == StateFile::Error::NOTFUND);
    REQUIRE (rfm.chargerInit () == StateFile::State::LAST);
    REQUIRE (rfm.schedulerInit () == StateFile::State::LAST);    
    REQUIRE (rfm.profileInit () == nullptr);
    REQUIRE (rfm.loggerInit () == false);
    REQUIRE (rfm.isChangedRespectTo (cp.getProfileWithName ("trip")) == true);
    
    REQUIRE (rfm.write (cp.getProfileWithName ("trip")) == StateFile::Error::NO);
    
    rfr.open ("./runfile");
    getline (rfr, line);
    rfr.close ();
    trim (line);
    REQUIRE (line == "trip");
    
    rfw.open ("./runfile");
    rfw << "#loggerflush maytrip";
    rfw.close ();
    
    REQUIRE (rfm.read () == StateFile::Error::NO);
    REQUIRE (rfm.chargerInit () == StateFile::State::LAST);
    REQUIRE (rfm.schedulerInit () == StateFile::State::LAST);    
    REQUIRE (rfm.profileInit () == cp.getProfileWithName ("maytrip"));
    REQUIRE (rfm.loggerInit () == true);
    REQUIRE (rfm.isChangedRespectTo (cp.getProfileWithName ("maytrip")) == true);
    
    REQUIRE (rfm.write (cp.getProfileWithName ("maytrip")) == StateFile::Error::NO);
    
    rfr.open ("./runfile");
    getline (rfr, line);
    rfr.close ();
    trim (line);
    REQUIRE (line == "maytrip");
    
    rfw.open ("./runfile");
    rfw << "#loggerflush maytrip #chargeron";
    rfw.close ();    
    
    REQUIRE (rfm.read () == StateFile::Error::NO);   
    REQUIRE (rfm.chargerInit () == StateFile::State::ON);
    REQUIRE (rfm.schedulerInit () == StateFile::State::LAST);      
    REQUIRE (rfm.profileInit () == cp.getProfileWithName ("maytrip")); 
    REQUIRE (rfm.loggerInit () == true);
    REQUIRE (rfm.isChangedRespectTo (cp.getProfileWithName ("maytrip")) == true);
    
    REQUIRE (rfm.write (cp.getProfileWithName ("maytrip")) == StateFile::Error::NO);
    
    rfr.open ("./runfile");
    getline (rfr, line);
    rfr.close ();
    trim (line);
    REQUIRE (line == "maytrip");
    
    rfw.open ("./runfile");
    rfw << "maytrip #chargeroff";
    rfw.close ();    
    
    REQUIRE (rfm.read () == StateFile::Error::NO); 
    REQUIRE (rfm.chargerInit () == StateFile::State::OFF);  
    REQUIRE (rfm.schedulerInit () == StateFile::State::LAST);    
    REQUIRE (rfm.profileInit () == cp.getProfileWithName ("maytrip"));     
    REQUIRE (rfm.loggerInit () == false);
    REQUIRE (rfm.isChangedRespectTo (cp.getProfileWithName ("maytrip")) == true);
    
    REQUIRE (rfm.write (cp.getProfileWithName ("maytrip")) == StateFile::Error::NO);
        
    rfr.open ("./runfile");
    getline (rfr, line);
    rfr.close ();
    trim (line);
    REQUIRE (line == "maytrip");
    
    rfw.open ("./runfile");
    rfw << "home onn";
    rfw.close ();    
    
    REQUIRE (rfm.read () == StateFile::Error::MUPRSET);
    REQUIRE (rfm.chargerInit () == StateFile::State::LAST);  
    REQUIRE (rfm.schedulerInit () == StateFile::State::LAST);    
    REQUIRE (rfm.profileInit () == nullptr);      
    REQUIRE (rfm.isChangedRespectTo (cp.getProfileWithName ("home")) == true);
    REQUIRE (rfm.loggerInit () == false);

    rfw.open ("./runfile");
    rfw << "home #chargeron #schedulern";
    rfw.close ();    
    
    REQUIRE (rfm.read () == StateFile::Error::UNKNSTA);
    REQUIRE (rfm.chargerInit () == StateFile::State::LAST);  
    REQUIRE (rfm.schedulerInit () == StateFile::State::LAST);    
    REQUIRE (rfm.profileInit () == nullptr);    
    REQUIRE (rfm.loggerInit () == false);
    REQUIRE (rfm.isChangedRespectTo (cp.getProfileWithName ("home")) == true);    
    REQUIRE (rfm.isChangedRespectTo (nullptr) == false);   
    
    rfw.open ("./runfile");
    rfw << "#chargeron #scheduleroff homes";
    rfw.close ();    
    
    REQUIRE (rfm.read () == StateFile::Error::PROFILE);    
    REQUIRE (rfm.chargerInit () == StateFile::State::LAST);  
    REQUIRE (rfm.schedulerInit () == StateFile::State::LAST);    
    REQUIRE (rfm.profileInit () == nullptr);  
    REQUIRE (rfm.loggerInit () == false);
    REQUIRE (rfm.isChangedRespectTo (cp.getProfileWithName ("home")) == true);
    REQUIRE (rfm.isChangedRespectTo (nullptr) == false);   
    
    rfw.open ("./runfile");
    rfw << "#chargeron home #schedulerofff";
    rfw.close ();    
    
    REQUIRE (rfm.read () == StateFile::Error::UNKNSTA);   
    REQUIRE (rfm.chargerInit () == StateFile::State::LAST);  
    REQUIRE (rfm.schedulerInit () == StateFile::State::LAST);    
    REQUIRE (rfm.profileInit () == nullptr);  
    REQUIRE (rfm.loggerInit () == false);
    REQUIRE (rfm.isChangedRespectTo (cp.getProfileWithName ("home")) == true);
    REQUIRE (rfm.isChangedRespectTo (nullptr) == false);   
                
    REQUIRE (rfm.write (cp.getProfileWithName ("home")) == StateFile::Error::NO);
        
    rfr.open ("./runfile");
    getline (rfr, line);
    rfr.close ();
    trim (line);
    REQUIRE (line == "home");    
    
    rfw.open ("./runfile");
    rfw << "home #scheduleroff";
    rfw.close ();    
    
    REQUIRE (rfm.read () == StateFile::Error::NO);
    REQUIRE (rfm.chargerInit () == StateFile::State::LAST);  
    REQUIRE (rfm.schedulerInit () == StateFile::State::OFF);    
    REQUIRE (rfm.profileInit () == cp.getProfileWithName ("home"));   
    REQUIRE (rfm.loggerInit () == false);
    REQUIRE (rfm.isChangedRespectTo (cp.getProfileWithName ("home")) == true);
    REQUIRE (rfm.isChangedRespectTo (cp.getProfileWithName ("home"), StateFile::State::LAST, StateFile::State::OFF) == false);   
    
    REQUIRE (rfm.write (cp.getProfileWithName ("home")) == StateFile::Error::NO);
        
    rfr.open ("./runfile");
    getline (rfr, line);
    rfr.close ();
    trim (line);
    REQUIRE (line == "home");    
    
    rfw.open ("./runfile");
    rfw << "home #scheduleroff #chargeroff";
    rfw.close ();    
    
    REQUIRE (rfm.read () == StateFile::Error::NO);
    REQUIRE (rfm.chargerInit () == StateFile::State::OFF);  
    REQUIRE (rfm.schedulerInit () == StateFile::State::OFF);    
    REQUIRE (rfm.profileInit () == cp.getProfileWithName ("home"));      
    REQUIRE (rfm.loggerInit () == false);
    REQUIRE (rfm.isChangedRespectTo (cp.getProfileWithName ("home")) == true);    
    REQUIRE (rfm.isChangedRespectTo (cp.getProfileWithName ("home"), StateFile::State::OFF, StateFile::State::OFF) == false);   
    
    REQUIRE (rfm.write (cp.getProfileWithName ("home")) == StateFile::Error::NO); 
        
    rfr.open ("./runfile");
    getline (rfr, line);
    rfr.close ();
    trim (line);
    REQUIRE (line == "home");      
        
    rfw.open ("./runfile");
    rfw << "trip #scheduleron #chargeron #loggerflush";
    rfw.close ();    
    
    REQUIRE (rfm.read () == StateFile::Error::NO);
    REQUIRE (rfm.chargerInit () == StateFile::State::ON);  
    REQUIRE (rfm.schedulerInit () == StateFile::State::ON);    
    REQUIRE (rfm.profileInit () == cp.getProfileWithName ("trip"));       
    REQUIRE (rfm.loggerInit () == true);
    REQUIRE (rfm.isChangedRespectTo (cp.getProfileWithName ("trip")) == true);    
    REQUIRE (rfm.isChangedRespectTo (cp.getProfileWithName ("trip"), StateFile::State::ON, StateFile::State::ON) == true);   
    
    REQUIRE (rfm.write (cp.getProfileWithName ("trip")) == StateFile::Error::NO);
        
    rfr.open ("./runfile");
    getline (rfr, line);
    rfr.close ();
    trim (line);
    REQUIRE (line == "trip");    
    
    REQUIRE (rfm.read () == StateFile::Error::NO);
    REQUIRE (rfm.chargerInit () == StateFile::State::LAST);  
    REQUIRE (rfm.schedulerInit () == StateFile::State::LAST);    
    REQUIRE (rfm.profileInit () == cp.getProfileWithName ("trip")); 
    REQUIRE (rfm.loggerInit () == false);
    REQUIRE (rfm.isChangedRespectTo (cp.getProfileWithName ("trip")) == false);    
    
    REQUIRE (rfm.write (cp.getProfileWithName ("trip")) == StateFile::Error::NO);  
    
    rfr.open ("./runfile");
    getline (rfr, line);
    rfr.close ();
    trim (line);
    REQUIRE (line == "trip");    
    
    REQUIRE (rfm.read () == StateFile::Error::NO); 
    REQUIRE (rfm.chargerInit () == StateFile::State::LAST);  
    REQUIRE (rfm.schedulerInit () == StateFile::State::LAST);    
    REQUIRE (rfm.profileInit () == cp.getProfileWithName ("trip"));   
    REQUIRE (rfm.loggerInit () == false);
    REQUIRE (rfm.isChangedRespectTo (cp.getProfileWithName ("trip")) == false);
    
    REQUIRE (rfm.isChangedRespectTo (cp.getProfileWithName ("trip"), StateFile::State::ON) == true);
    REQUIRE (rfm.isChangedRespectTo (cp.getProfileWithName ("trip"), StateFile::State::LAST, StateFile::State::ON) == true);
    REQUIRE (rfm.isChangedRespectTo (cp.getProfileWithName ("trip"), StateFile::State::LAST, StateFile::State::LAST) == false);
    
    REQUIRE (rfm.write (cp.getProfileWithName ("home")) == StateFile::Error::NO);  
    
    rfr.open ("./runfile");
    getline (rfr, line);
    rfr.close ();
    trim (line);
    REQUIRE (line == "home");        
    
    REQUIRE (rfm.write (nullptr, StateFile::State::ON, StateFile::State::OFF) == StateFile::Error::NO);  
    rfr.open ("./runfile");
    getline (rfr, line);
    rfr.close ();
    trim (line);
    REQUIRE (line == "#chargeron #scheduleroff");                       
    
    REQUIRE (rfm.write (nullptr, StateFile::State::OFF, StateFile::State::ON) == StateFile::Error::NO);  
    rfr.open ("./runfile");
    getline (rfr, line);
    rfr.close ();
    trim (line);
    REQUIRE (line == "#chargeroff #scheduleron");                           
    
    REQUIRE (rfm.write (cp.getProfileWithName ("home"), StateFile::State::LAST, StateFile::State::ON) == StateFile::Error::NO);  
    rfr.open ("./runfile");
    getline (rfr, line);
    rfr.close ();
    trim (line);
    REQUIRE (line == "home #scheduleron");                               
    
    REQUIRE (rfm.write (cp.getProfileWithName ("trip"), StateFile::State::OFF) == StateFile::Error::NO);  
    rfr.open ("./runfile");
    getline (rfr, line);
    rfr.close ();
    trim (line);
    REQUIRE (line == "trip #chargeroff");    
    
    REQUIRE (rfm.write () == StateFile::Error::NO);  
    rfr.open ("./runfile");
    getline (rfr, line);
    rfr.close ();
    trim (line);
    REQUIRE (line == "");        
}


TEST_CASE("ProfileSchedules", "[schedule]") 
{
    ChargeProfiles cp;
    cp.addProfile (ChargeProfile ("home", 40, 60, false));
    cp.addProfile (ChargeProfile ("trip", 80, 100, true));
    cp.addProfile (ChargeProfile ("maytrip", 60, 80, true));
    
    ProfileSchedules sc;
    REQUIRE (sc.isEnabled () == false);
    
    sc.addSchedule (ProfileSchedule (true, std::vector<bool> (12, true), std::vector<bool> (31, true), {true, true, true, true, true, true, true}, HourMin ({12,00}), HourMin ({18,00}), cp.getProfileWithName ("home"))); 
    REQUIRE_THROWS (sc.addSchedule (ProfileSchedule (true, std::vector<bool> (12, true), std::vector<bool> (31, true), {true, false, false, false, false, false, false}, HourMin ({18,00}), HourMin ({20,00}), cp.getProfileWithName ("trip")))); 
    REQUIRE_THROWS (sc.addSchedule (ProfileSchedule (true, std::vector<bool> (12, true), std::vector<bool> (31, true), {false, false, false, false, false, false, true}, HourMin ({10,01}), HourMin ({12,00}), cp.getProfileWithName ("trip"))));     
    sc.addSchedule (ProfileSchedule (true, std::vector<bool> (12, true), std::vector<bool> (31, true), {true, true, true, true, true, true, true}, HourMin ({18,01}), HourMin ({20,00}), cp.getProfileWithName ("trip"))); 
    REQUIRE_THROWS (sc.addSchedule (ProfileSchedule (true, std::vector<bool> (12, true), std::vector<bool> (31, true), {true, true, true, true, true, true, true}, HourMin ({22,01}), HourMin ({24,00}), cp.getProfileWithName ("trip"))));     
    REQUIRE_THROWS (sc.addSchedule (ProfileSchedule (true, std::vector<bool> (12, true), std::vector<bool> (31, true), {true, true, true, true, true, true, true}, HourMin ({23,01}), HourMin ({22,20}), cp.getProfileWithName ("trip"))));         
    sc.addSchedule (ProfileSchedule (true, std::vector<bool> (12, true), std::vector<bool> (31, true), {true, true, true, true, true, true, true}, HourMin ({20,01}), HourMin ({21,00}), cp.getProfileWithName ("maytrip")));
    sc.addSchedule (ProfileSchedule (true, std::vector<bool> (12, true), std::vector<bool> (31, true), {true, true, true, true, true, true, true}, HourMin ({20,51}), HourMin ({21,30}), cp.getProfileWithName ("maytrip")));
    sc.addSchedule (ProfileSchedule (false, std::vector<bool> (12, true), std::vector<bool> (31, true), {true, true, true, true, true, true, true}, HourMin ({20,01}), HourMin ({21,00}), cp.getProfileWithName ("trip")));
    sc.addSchedule (ProfileSchedule (false, std::vector<bool> (12, true), std::vector<bool> (31, true), {true, true, true, true, true, true, true}, HourMin ({20,51}), HourMin ({21,30}), cp.getProfileWithName ("trip")));
    REQUIRE_THROWS (sc.addSchedule (ProfileSchedule (true, std::vector<bool> (12, true), std::vector<bool> (31, true), {true, true, true, true, true, true, true}, HourMin ({20,01}), HourMin ({21,00}), cp.getProfileWithName ("trip"))));
    REQUIRE_THROWS (sc.addSchedule (ProfileSchedule (true, std::vector<bool> (12, true), std::vector<bool> (31, true), {true, true, true, true, true, true, true}, HourMin ({20,51}), HourMin ({21,30}), cp.getProfileWithName ("trip"))));
    INFO ("ProfileSchedules :\n" << sc.toString ());
    
    REQUIRE (ProfileSchedule::vectorBoolToString (std::vector<bool> {true, false, true, true, false, false, true, true, true, true, false, false, false, true}, 1) == "1.3-4.7-10.14");
    REQUIRE (ProfileSchedule::vectorBoolToString (std::vector<bool> {false, false, false, true, true, true, true, true, false, false, false}, 1) == "4-8");
    REQUIRE (ProfileSchedule::vectorBoolToString (std::vector<bool> {false, true, false, true, false, true, false, true, false, true, false}, 1) == "2.4.6.8.10");
    REQUIRE (ProfileSchedule::vectorBoolToString (std::vector<bool> {true, true, true, true, false, true, false, true, true, true, true}, 1) == "-4.6.8-");
    REQUIRE (ProfileSchedule::vectorBoolToString (std::vector<bool> {true, true, true, true, false, false, false, true, true, true, true}, 1) == "-4.8-");
    REQUIRE (ProfileSchedule::vectorBoolToString (std::vector<bool> {true, true, true, true, true, true, true, true, true, true, true}, 1) == "-");
    
    struct tm now;
    now.tm_year = 2024 - 1900; // Year since 1900
    now.tm_mon = 10;           // Month (0-11)
    now.tm_mday = 20;           // Day of the month
    now.tm_hour = 18;           // Hour (0-23)
    now.tm_min = 00;            // Minute (0-59)
    now.tm_sec = 0;             // Second (0-61)
    now.tm_isdst = -1;          // Determine daylight saving time automatically
    
    REQUIRE (sc.getScheduleTriggered (mktime (& now)) == nullptr);
    
    sc.setEnable (true);
    REQUIRE (sc.isEnabled () == true);
    
    now.tm_hour = 18;
    now.tm_min = 0;            
    REQUIRE (sc.getScheduleTriggered (mktime (& now))->profile == cp.getProfileWithName ("home"));
    
    now.tm_hour = 18;
    now.tm_min = 10;            
    REQUIRE (sc.getScheduleTriggered (mktime (& now))->profile == cp.getProfileWithName ("trip"));

    now.tm_hour = 20;
    now.tm_min = 00;            
    REQUIRE (sc.getScheduleTriggered (mktime (& now))->profile == cp.getProfileWithName ("trip"));

    now.tm_hour = 20;
    now.tm_min = 01;            
    REQUIRE (sc.getScheduleTriggered (mktime (& now))->profile == cp.getProfileWithName ("maytrip"));

    now.tm_hour = 21;
    now.tm_min = 31;            
    REQUIRE (sc.getScheduleTriggered (mktime (& now)) == nullptr);
        
}    
