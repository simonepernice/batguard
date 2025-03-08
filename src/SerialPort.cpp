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
 
#include "SerialPort.hpp"
#include <string.h>
#include <stdio.h>
#include <fcntl.h>  
#include <errno.h>  
#include <termios.h>   
#include <unistd.h> 
#include <stdexcept>
#include <iostream>

bool SerialPort::writeByte (uint8_t c)
{
    if (writeEnd >= bufferSize) return false;
    writeBuffer [writeEnd] = c;
    ++ writeEnd;
    return true;
}
    
unsigned int SerialPort::writeFlush ()
{
    unsigned int writtenbytes = (unsigned int) write (serialDesc, writeBuffer.data () + writeBeg, writeEnd - writeBeg);
    writeBeg += writtenbytes;
    if (writeBeg == writeEnd) writeBeg = writeEnd = 0;
    return writtenbytes;
}

unsigned int SerialPort::bytesToWrite () const
{
    return writeEnd - writeBeg;
}
                
unsigned int SerialPort::bytesToRead () const
{
    return readEnd - readBeg;
} 

uint8_t SerialPort::readByte ()
{
    if (readBeg == readEnd) throw std::runtime_error ("Internal lastError in SerialPort::readByte called with empty buffer");
    unsigned int bytetoread = readBeg;
    ++ readBeg ;
    if (readBeg == readEnd) readBeg = readEnd = 0;
    return readBuffer [bytetoread];
}

unsigned int SerialPort::readFlush ()
{
    readEnd += (unsigned int) read (serialDesc, readBuffer.data () + readEnd, bufferSize - readEnd);
    return readEnd - readBeg;
}
        
SerialPort::~SerialPort ()
{
    close (serialDesc);
}

int SerialPort::tryToOpen (const std::string& path, const uint8_t maxConnAttemp)
{    
    int sd = open (path.c_str (), O_RDWR);
    
    uint8_t conTrial = 1;
    while (sd < 0 and conTrial < maxConnAttemp)
    {
        std::cout << "Unable to link to " << path << '\n';
        std::cout << "Connection trial " << static_cast<int> (conTrial) << " of " << static_cast<int> (maxConnAttemp) << '\n'; 
        sleep (2);
        
        sd = open (path.c_str (), O_RDWR);
        
        ++ conTrial;
    }
    
    if (sd < 0) throw std::invalid_argument ("It was not possible to open for read and write the device: " + path + ", lastError: " + std::string (strerror (errno)));
    
    return sd;    
}

SerialPort::SerialPort (const std::string& path, const unsigned int baudrate, const uint8_t maxConnAttemp, const uint8_t bits, const bool parity, const bool singlestop, const bool flowctl) :
    serialDesc  {tryToOpen (path.c_str (), maxConnAttemp)},
    readBuffer  {},
    writeBuffer {},
    readBeg     {0},
    readEnd     {0},
    writeBeg    {0},
    writeEnd    {0}
{
    struct termios serialconfig;
    if(tcgetattr (serialDesc, &serialconfig)) throw std::invalid_argument ("It was not possible to retrieve the configuration of: " + path + " lastError: " + std::string (strerror (errno)));
    
    if (parity) serialconfig.c_cflag |=  PARENB;
    else        serialconfig.c_cflag &= ~PARENB;
    
    if (singlestop) serialconfig.c_cflag &= ~CSTOPB;
    else            serialconfig.c_cflag |=  CSTOPB;
    
    serialconfig.c_cflag &= ~CSIZE;
    if      (bits == 5) serialconfig.c_cflag |= CS5;
    else if (bits == 6) serialconfig.c_cflag |= CS6;
    else if (bits == 7) serialconfig.c_cflag |= CS7;
    else if (bits == 8) serialconfig.c_cflag |= CS8;
    else throw std::invalid_argument ("The given number of bits is not supported: " + std::to_string (bits));
    
    if (flowctl) serialconfig.c_cflag |=  CRTSCTS;
    else         serialconfig.c_cflag &= ~CRTSCTS;
    
    //Disables modem signal like carrier detect
    serialconfig.c_cflag |= CLOCAL;
    
    //Allow to read data
    serialconfig.c_cflag |= CREAD;
    
    //Disable canonical mode to receive every single character instead of wait for the new line
    serialconfig.c_lflag &= ~ICANON;
    
    //Disable echo
    serialconfig.c_lflag &= ~ECHO;
    
    //Disable erasure
    serialconfig.c_lflag &= ~ECHOE; 

    //Disable new-line echo
    serialconfig.c_lflag &= ~ECHONL; 
    
    //Disable interpretation of INTR, QUIT and SUSP
    serialconfig.c_lflag &= ~ISIG; 
    
    //Disable software flow ctrl
    serialconfig.c_iflag &= ~(IXON | IXOFF | IXANY); 
    
    //Disable any special handling of received bytes
    serialconfig.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); 
    
    //Disable special interpretation of output bytes (e.g. newline chars)
    serialconfig.c_oflag &= ~OPOST; 
    
    //Disable conversion of newline to carriage return/line feed
    serialconfig.c_oflag &= ~ONLCR; 

    //read call waits up to 1s 
    serialconfig.c_cc[VTIME] = 10;  
    
    //read call does not wait for any number of chars received
    serialconfig.c_cc[VMIN] = 0;
    
    int spd = B9600;
    if      (baudrate == 50)     spd = B50;
    else if (baudrate == 75)     spd = B75;  
    else if (baudrate == 110)    spd = B110;
    else if (baudrate == 134)    spd = B134;
    else if (baudrate == 150)    spd = B150;
    else if (baudrate == 200)    spd = B200;
    else if (baudrate == 300)    spd = B300;
    else if (baudrate == 600)    spd = B600;
    else if (baudrate == 1200)   spd = B1200;
    else if (baudrate == 1800)   spd = B1800;
    else if (baudrate == 2400)   spd = B2400;
    else if (baudrate == 4800)   spd = B4800;    
    else if (baudrate == 9600)   spd = B9600;
    else if (baudrate == 19200)  spd = B19200;
    else if (baudrate == 38400)  spd = B38400;
    else if (baudrate == 57600)  spd = B57600;
    else if (baudrate == 115200) spd = B115200;
    else if (baudrate == 230400) spd = B230400;
    else if (baudrate == 460800) spd = B460800;
    else throw std::invalid_argument ("The given baud rate is not supported: " + baudrate);
    cfsetspeed(&serialconfig, spd);
    
    if (tcsetattr (serialDesc, TCSANOW, &serialconfig) != 0) throw std::invalid_argument ("Error configuring the serial port parameters: " + std::string (strerror (errno)));
}
