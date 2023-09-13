<div style = "float: right">
    <img src="https://upload.wikimedia.org/wikipedia/commons/thumb/8/86/Festo_logo.svg/1200px-Festo_logo.svg.png" alt="Festo Logo" width="150">
</div>

# CMMT EtherCAT controlled by C++

Simple implementation to control CMMT servodrives from Festo over EtherCAT by C++. Uses the [Simple EtherCAT Master Librabry](https://github.com/OpenEtherCATsociety/SOEM).

## Installation
See the application note for detailed description of start up.
### Software used
The test was conducted on the CMMT-AS-C2-3A-MP-S1 servodrive but should work on all CMMT drives where EtherCAT is available.

Make sure you have the following software versions installed:

- [Visual Studio 2019](https://visualstudio.microsoft.com/2019/): Required for development.

- [Pcap 1.76](https://npcap.com/#download): Install Pcap for network packet capture.

- [Festo Automation Suite](https://www.festo.com/net/en-gb_gb/SupportPortal/Details/686274/Download/0/): Required for configuration of the Fieldbus

### Guide

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
4. Copy the indentifier and replace it with the in the main.cpp file which can be found in the master folder. **Note** the added backslashes
```C++
char ifaceName[] = "\\Device\\NPF_{1975556f-9B2A-4879-B28-88F43D0F1FD9}"
```

## Usage
TODO