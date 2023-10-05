<div style = "float: right">
    <img src="https://upload.wikimedia.org/wikipedia/commons/thumb/8/86/Festo_logo.svg/1200px-Festo_logo.svg.png" alt="Festo Logo" width="150">
</div>

# CMMT EtherCAT controlled by C++

Simple implementation to control CMMT servodrives from Festo over EtherCAT by C++. Uses the [Simple EtherCAT Master Librabry](https://github.com/OpenEtherCATsociety/SOEM).

## Installation
See the application note for detailed description of start up.
### Software used
The test was conducted on the CMMT-AS-C2-3A-MP-S1 servodrive 

Make sure you have the following software versions installed:

- [Visual Studio 2019](https://visualstudio.microsoft.com/2019/): Required for development. (Make sure to install C++ Desktop development)

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

With the following functions we can control the slave:
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

