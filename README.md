<div style = "float: right">
    <img src="https://upload.wikimedia.org/wikipedia/commons/thumb/8/86/Festo_logo.svg/1200px-Festo_logo.svg.png" alt="Festo Logo" width="150">
</div>

# CMMT EtherCAT controlled by C++

The project at hand is a straightforward implementation designed to facilitate the control of CMMT servodrives from Festo through EtherCAT using the C++ programming language. It leverages the [Simple EtherCAT Master Librabry](https://github.com/OpenEtherCATsociety/SOEM) for seamless communication with these servodrives.

This application serves as a bridge between a computer system and CMMT servodrives from Festo, allowingcontrol of servomotors. It empowers users to operate various automation systems, such as gantry systems and linear robots, directly from a PC instead of relying on a traditional Programmable Logic Controller (PLC).

## Table of Contents
  - [Installation](#installation)
    - [Software used](#software-used)
    - [Guide](#guide)
  - [Usage](#usage)
    - [Functions](#functions)
    - [Examples](#examples)
      - [Position](#position)
      - [Position (nonblock)](#position-nonblock)
      - [Multiposition (nonblocking)](#multiposition-nonblocking)
      - [Velocity](#velocity)
      - [Velocity (nonblock)](#velocity-nonblock)
      - [Jog](#jog)
      - [Jog (nonblock)](#jog-nonblock)
      - [SDO Access](#sdo-access)

## Installation
See the [Application Note](ApplicationNoteCMMTwithCcpp%23100337-V1.10.pdf) for detailed description of start up.
### Software used
The test was conducted on the CMMT-AS-C2-3A-MP-S1 servodrive 

Make sure you have the following software versions installed:

- [Visual Studio 2019](https://visualstudio.microsoft.com/2019/): Required for development. (Make sure to install Desktop Developement for C++)

- [Pcap 1.76](https://npcap.com/#download): Install Pcap for network packet capture.

- [Festo Automation Suite](https://www.festo.com/net/en-gb_gb/SupportPortal/Details/686274/Download/0/): Required for configuration of the Fieldbus

### Guide

**Note** Ensure that when using an MP servodrive, the fieldbus configuration is set to EtherCAT. This can be achieved by establishing a connection between your servodrive and the Festo Automation Suite. For detailed instructions, please consult the specific servodrive's user guide.

1. Clone the repository
```bash
git clone <git-url> <destination>
```
2. Open the cloned repositry with Visual Studio

3. Run the slaveinfo.exe to acquire the unique identifier of your network adapter
```
Example
win:   \Device\NPF_{1975556f-9B2A-4879-B28-88F43D0F1FD9}
linux: eth0
```
4. Copy the indentifier and replace it with the in the main.cpp file which can be found in the master folder. 
**Note** the added backslashes
```C++
char ifaceName[] = "\\Device\\NPF_{1975556f-9B2A-4879-B28-88F43D0F1FD9}"
```

## Usage
Before we can control a servodrive we have to create a master. Then assign a slave to the master.

```C++
Master ecMaster(ifaceName, 8000);
Slave slave1(ecMaster, 1);
Slave slave2(ecMaster, 2);
Slave slave3(ecMaster, 3);
...
```

### Functions
| Function| Description   |
| :---   | :--- |
| enable_powerstage() | Enable the powerstage of the drive | 
| disable_powerstage() | Disable the powerstage of the drive | 
| referencing_task() | Perform the referencing task. If successful, the drive is in the homed state. | 
| acknowledge_faults() | Acknowledge faults of the drive | 
| jog_task(bool jog_positive, bool jog_negative, float duration) | Perform a jogging task with given duration | 
| stop_motion_task() | Stops any currently active motion task | 
| int position_task(int32_t target, uint32_t velocity, bool absolute, bool nonblocking) | Position task with velocity | 
| position_task(int32_t target, uint32_t velocity, uint32_t acceleration, uint32_t deceleration, bool absolute, bool nonblocking) | Position task with acceleration and deceleration | 
| wait_for_target_position() |  Wait for the target position to be reached |
| record_task(int32_t record) | Perform a preconfigured record task by providing the corresponding record number |
| velocity_task(int32_t velocity, float duration) | Perform a velocit |y task with given velocity and duration |
| write_sdo(uint16 index, uint8 subindex, void *value, int valueSize) | Write on a Service Data Object (SDO) from a slave device. |
| read_sdo(uint16 index, uint8 subindex, void *value, int *valueSize) | Read a Service Data Object (SDO) from a slave device. |

### Examples

#### Position
```C++
#include "master.h"
#include "slave.h"

int main(int argc, char* argv[]){
    // Your network interface name here
    char ifaceName[] = "\\Device\\NPF_{A00D620E-09CB-4317-9A0F-4DAAD76B366E}"; 
    Master ecMaster(ifaceName, 8000);

    if (ecMaster.connected()){
        Slave ecSlave(ecMaster, 1);

        ecSlave.acknowledge_faults();
        ecSlave.enable_powerstage();
        ecSlave.referencing_task();

        // Relative
        ecSlave.position_task(100000, 600000);
        ecSlave.position_task(-50000, 50000);
        // Absolute
        ecSlave.position_task(300000, 600000, true);
        
        return EXIT_SUCCESS;
    }
    else{
        return EXIT_FAILURE;
    }
}
```

#### Position (nonblock)

``````C++
#include "master.h" 
#include "slave.h"

int main(int argc, char* argv[]){
    // Your network interface name here
    char ifaceName[] = "\\Device\\NPF_{A00D620E-09CB-4317-9A0F-4DAAD76B366E}"; 
    Master ecMaster(ifaceName, 8000);

    if (ecMaster.connected()){
        Slave ecSlave(ecMaster, 1);

        ecSlave.acknowledge_faults();
        ecSlave.enable_powerstage();
        ecSlave.referencing_task();

        // No continuous update
        // Absolute and nonblocking
        ecSlave.position_task(10000000, 600000, true, true);
        Sleep(1000);
        ecSlave.position_task(-10000000, 600000, true, true);
        Sleep(1000);
        ecSlave.position_task(10000000, 600000, true, true);
        Sleep(1000);
        ecSlave.position_task(10000000, 600000, true, true);
        
        ecSlave.wait_for_target_position();

        return EXIT_SUCCESS;
    }
    else{
        return EXIT_FAILURE;
    }
}
``````

#### Multiposition (nonblocking)
```` C++
#include "master.h" 
#include "slave.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <algorithm> // Include the algorithm header for std::all_of


int main(int argc, char* argv[]){
    char ifaceName[] = "\\Device\\NPF_{A00D620E-09CB-4317-9A0F-4DAAD76B366E}";
    Master ecMaster(ifaceName, 8000);
    if (ecMaster.connected()){
        std::vector<Slave> slaves = {
            Slave(ecMaster, 1),
            Slave(ecMaster, 2)
        };

        for (auto& slave : slaves) {
            slave.acknowledge_faults();
            slave.enable_powerstage();
            slave.referencing_task();
        }

        for (auto& slave : slaves) {
            slave.position_task(10000000, 300000, true, true);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        while (true) {
            std::vector<bool> target_positions_reached;
            for (auto& slave : slaves) {
                target_positions_reached.push_back(slave.wait_for_target_position());
            }
            std::cout << "Target positions reached: ";
            for (bool reached : target_positions_reached) {
                std::cout << (reached ? "true " : "false ");
            }
            std::cout << std::endl;
            if (std::all_of(target_positions_reached.begin(), target_positions_reached.end(), [](bool val) { return val; })) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        return EXIT_SUCCESS;
    }
    else{
        return EXIT_FAILURE;
    }
}
````
#### Velocity

````C++
#include "master.h" 
#include "slave.h"

int main(int argc, char* argv[]){
    char ifaceName[] = "\\Device\\NPF_{A00D620E-09CB-4317-9A0F-4DAAD76B366E}";
    Master ecMaster(ifaceName, 8000);
    if (ecMaster.connected()){
        Slave ecSlave(ecMaster, 2);

        ecSlave.acknowledge_faults();
        ecSlave.enable_powerstage();
        ecSlave.referencing_task();

        ecSlave.velocity_task(50000, 3.0);
        Sleep(1000);
        ecSlave.velocity_task(-30000, 3.0);


        return EXIT_SUCCESS;
    }
    else{
        return EXIT_FAILURE;
    }
}
````
#### Velocity (nonblock)
````C++
#include "master.h" 
#include "slave.h"

int main(int argc, char* argv[]){
    char ifaceName[] = "\\Device\\NPF_{A00D620E-09CB-4317-9A0F-4DAAD76B366E}";
    Master ecMaster(ifaceName, 8000);
    if (ecMaster.connected()){
        Slave ecSlave(ecMaster, 2);

        ecSlave.acknowledge_faults();
        ecSlave.enable_powerstage();
        ecSlave.referencing_task();


        // No continuous update
        ecSlave.velocity_task(50000, 0);
        Sleep(1000);
        ecSlave.velocity_task(-10000, 0);
        Sleep(1000);
        ecSlave.velocity_task(10001, 0);

        Sleep(3000);
        ecSlave.stop_motion_task();

        return EXIT_SUCCESS;
    }
    else{
        return EXIT_FAILURE;
    }
}
````
#### Jog
````C++
#include "master.h" 
#include "slave.h"

int main(int argc, char* argv[]){
    char ifaceName[] = "\\Device\\NPF_{A00D620E-09CB-4317-9A0F-4DAAD76B366E}";
    Master ecMaster(ifaceName, 8000);
    if (ecMaster.connected()){
        Slave ecSlave(ecMaster, 2);

        ecSlave.acknowledge_faults();
        ecSlave.enable_powerstage();

        ecSlave.jog_task(true, false, 3.0);

        return EXIT_SUCCESS;
    }
    else{
        return EXIT_FAILURE;
    }
}
````
#### Jog (nonblock)
````C++
#include "master.h" 
#include "slave.h"

int main(int argc, char* argv[]){
    char ifaceName[] = "\\Device\\NPF_{A00D620E-09CB-4317-9A0F-4DAAD76B366E}";
    Master ecMaster(ifaceName, 8000);
    if (ecMaster.connected()){
        Slave ecSlave(ecMaster, 2);

        ecSlave.acknowledge_faults();
        ecSlave.enable_powerstage();

        ecSlave.jog_task(true, false, 0.0);
        Sleep(4000);
        ecSlave.jog_task(false, true, 0.0);
        Sleep(4000);

        ecSlave.stop_motion_task();

        return EXIT_SUCCESS;
    }
    else{
        return EXIT_FAILURE;
    }
}
````
#### SDO Access
````C++
#include "master.h" 
#include "slave.h"

int main(int argc, char* argv[]){
    char ifaceName[] = "\\Device\\NPF_{A00D620E-09CB-4317-9A0F-4DAAD76B366E}";
    Master ecMaster(ifaceName, 8000);
    if (ecMaster.connected()){
        Slave ecSlave(ecMaster, 2);

        // Read Torque upper limit value
        // Define buffer to store the value and its size
        float32 original_value;
        int size = sizeof(original_value);
        ecSlave.read_sdo(0x2168, 0x04, &original_value, &size);

        // Print in terminal the original value
        std::cout << "Original Torque upper limit value: " << original_value << std::endl;

        // Configure a different torque upper limit value
        float32 new_value = 0.6720;
        ecSlave.write_sdo(0x2168, 0x04, &new_value, sizeof(new_value));

        // Read the new torque upper limit value
        float32 read_value;
        int sizer = sizeof(read_value);
        ecSlave.read_sdo(0x2168, 0x04, &read_value, &sizer);

        // Print in terminal the new value
        std::cout << "New Torque upper limit value: " << read_value << std::endl;

        // Restore the original value
        ecSlave.write_sdo(0x2168, 0x04, &original_value, sizeof(original_value));

        // Print in terminal the original value again
        std::cout << "Original Torque upper limit value: " << original_value << std::endl;
        

        return EXIT_SUCCESS;
    }
    else{
        return EXIT_FAILURE;
    }
}
````