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
 
#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <string>
#include <array>
#include <cstdint>

class SerialPort
{
    public:
        //open a serial port at the given path
        //with the given baudrate, only standard ones are allowed
        //with the maximum connection attempts after which throws an error, it waits a second before each trial
        //with the given number of bits
        //with the parity bit
        //with the single stop
        //with the flow control 
        //the serial port hardware/low-level driver has a write and a read buffers where data is written just before sending or after reception
        //this call has also its read and write buffers which are read or wrote byte per byte with calls
        //those read/write buffers are filled in/out with their flush calls
                        SerialPort (const std::string& path, const unsigned int baudrate, const uint8_t maxConnAttemp = 10, const uint8_t bits = 8, const bool parity=false, const bool singlestop=true, const bool flowctl=false);
        
        //the serial port is closed when the object is destroyed
                        ~SerialPort ();
        
        //write a byte in the sending buffer but does not send it
        //return true if there is space in the buffer for its addition to the queue
        //gets the value of the byte to send
        bool            writeByte (uint8_t);
        
        //try to send all the bytes in the output buffer through the serial port, may be hardware/low-level-driver limitations
        //returns the number of byte actually sent
        //consider, there may be pending bytes in the pipe from the previous flush
        unsigned int    writeFlush ();
        
        //returns the number of bytes to be written by writeByte in the write buffer
        unsigned int    bytesToWrite () const;        
        
        //read a byte from the reception buffer
        //if the buffer is empty throw an exception
        //returns the value of the read byte
        uint8_t         readByte ();
        
        //fill in the read buffer with all the bytes arrived the the serial port
        //returns the number of bytes available on the read buffer
        //consider, there may be not-read bytes from the last flush
        unsigned int    readFlush ();
        
        //returns the number of bytes to be read by readByte in the read buffer        
        unsigned int    bytesToRead () const;
        
    private:
        const int                               serialDesc;
        static constexpr unsigned int           bufferSize = 128;
        std::array <uint8_t, bufferSize>        readBuffer;
        std::array <uint8_t, bufferSize>        writeBuffer;
        unsigned int                            readBeg;
        unsigned int                            readEnd;
        unsigned int                            writeBeg; 
        unsigned int                            writeEnd;        
        
        int      tryToOpen (const std::string&, const uint8_t);
};

#endif //SERIALPORT_H
