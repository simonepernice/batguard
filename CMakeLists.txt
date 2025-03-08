#
# This file is part of batguard.
#
# batguard is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# batguard is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

cmake_minimum_required(VERSION 3.10)

project(batguard)

option (BUILD_TESTS "Build unit tests" OFF)

if (BUILD_TESTS)

    find_package (Catch2 3 REQUIRED)

    add_executable (tests 
                    tests/tests.cpp 
                    src/ConfigReader.cpp        include/ConfigReader.hpp 
                    src/SerialPort.cpp          include/SerialPort.hpp 
                    src/RelayDriver.cpp         include/RelayDriver.hpp
                    src/CapacityReader.cpp      include/CapacityReader.hpp
                    src/LogWriter.cpp           include/LogWriter.hpp
                    src/ChargeProfiles.cpp      include/ChargeProfiles.hpp
                    src/stringtools.cpp         include/stringtools.hpp
                    src/StateFile.cpp      include/StateFile.hpp 
                    src/ProfileSchedules.cpp    include/ProfileSchedules.hpp )

    target_link_libraries       (tests PRIVATE Catch2::Catch2WithMain)
    target_include_directories  (tests PRIVATE include)
    target_compile_options      (tests PRIVATE "-Wall" "-Wextra" "-Werror" "-Wconversion" "-Wpedantic" "-g")
    
endif ()
                
add_executable (batguard 
                src/main.cpp 
                src/ConfigReader.cpp        include/ConfigReader.hpp 
                src/SerialPort.cpp          include/SerialPort.hpp 
                src/RelayDriver.cpp         include/RelayDriver.hpp
                src/CapacityReader.cpp      include/CapacityReader.hpp
                src/LogWriter.cpp           include/LogWriter.hpp
                src/ChargeProfiles.cpp      include/ChargeProfiles.hpp
                src/stringtools.cpp         include/stringtools.hpp
                src/StateFile.cpp      include/StateFile.hpp
                src/BatGuard.cpp            include/BatGuard.hpp 
                src/ProfileSchedules.cpp    include/ProfileSchedules.hpp )                

target_include_directories (batguard PRIVATE include)
target_compile_options     (batguard PRIVATE "-Wall" "-Wextra" "-Werror" "-Wconversion" "-Wpedantic" "-O3")

include (GNUInstallDirs)
set (INSTALL_BIN_DIR ${CMAKE_INSTALL_BINDIR})

message (STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
message (STATUS "CMAKE_INSTALL_BINDIR: ${CMAKE_INSTALL_BINDIR}")

install (TARGETS batguard RUNTIME DESTINATION ${INSTALL_BIN_DIR})

install (FILES install/command              DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
install (FILES install/config               DESTINATION ${CMAKE_CURRENT_BINARY_DIR}) 
install (FILES install/makeconfigfile.py    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}) 
install (CODE 
        "
            execute_process (COMMAND /usr/bin/env python3 ${CMAKE_CURRENT_BINARY_DIR}/makeconfigfile.py RESULT_VARIABLE result)
            if (NOT result EQUAL 0) 
                message (FATAL_ERROR \"Post-install configuration validation script failed with code: ${result}\")
            endif ()
        ")
install (DIRECTORY DESTINATION "/etc/batguard")
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/config   DESTINATION /etc/batguard PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ) 
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/command  DESTINATION /etc/batguard PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ WORLD_WRITE)        

install (FILES install/makeudevrule.py              DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
install (FILES install/enablerelayudevrules.sh      DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
install (CODE 
        "
            execute_process (COMMAND /usr/bin/env python3 ${CMAKE_CURRENT_BINARY_DIR}/makeudevrule.sh RESULT_VARIABLE result)
            if (NOT result EQUAL 0) 
                message (FATAL_ERROR \"Post-install udev rule creation script failed with code: ${result}\")
            endif ()
        ")
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/90-batguard-relay.rules DESTINATION /etc/udev/rules.d PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ) 
install (CODE 
        "
            execute_process (COMMAND /usr/bin/sh ${CMAKE_CURRENT_BINARY_DIR}/enablerelayudevrules.sh RESULT_VARIABLE result)
            if (NOT result EQUAL 0) 
                message (FATAL_ERROR \"Post-install udev rule abilitation script failed with code: ${result}\")
            endif ()
        ")
        
install (FILES install/batguard.service             DESTINATION /etc/systemd/system PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
install (FILES install/batguardsleep.service        DESTINATION /etc/systemd/system PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
install (FILES install/enablebatguardservices.sh    DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
install (CODE 
        "
            execute_process (COMMAND /usr/bin/sh ${CMAKE_CURRENT_BINARY_DIR}/enablebatguardservices.sh RESULT_VARIABLE result)
            if (NOT result EQUAL 0) 
                message (FATAL_ERROR \"Post-install service initialization script failed with code: ${result}\")
            endif ()
        ")

install (CODE "message (STATUS \"Installation completed successfully\") ")
