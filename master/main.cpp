#include "master.h" // To include the EtherCat Master library

int main(int argc, char* argv[])
{
    char ifaceName[] = "\\Device\\NPF_{A00D620E-09CB-4317-9A0F-4DAAD76B366E}"; // Your network interface name here

    Master ecMaster(ifaceName, 8000); // 8000 = 8 ms cycle time

    if (ecMaster.connected()) // Check if EtherCat Master reached the Operational State
    {
        for (int i = 1; i <= ec_slavecount; i++)
        {
            ecMaster.reset(i);  // reset all drives
        }

        int slaveNr = 1; // Slave to move in this example 0 = Master, slaves numbered in connected order
        ecMaster.enable_powerstage(slaveNr);
        ecMaster.referencing_task(slaveNr, true); // Home the drive, even when already homed (true)
        ecMaster.jog_task(slaveNr, true, false, 0);
        Sleep(10000);
        ecMaster.jog_task(slaveNr, false, true, 5.5);
        Sleep(5000); // delay next instruction for 5 seconds
        ecMaster.disable_powerstage(slaveNr); // Power off the drive
        Sleep(5000); // delay next instruction for 5 seconds
        ecMaster.enable_powerstage(slaveNr); // Power the drive

        ecMaster.position_task(slaveNr, 20000000, 1000000); // absolute movement position and velocity
        ecMaster.position_task(slaveNr, -5000000, 100000, true); // relative movement (true)

        ecMaster.position_task(slaveNr, 20000000, 1000000, 100000, 100000); // absolute movement position and velocity

        return EXIT_SUCCESS; // Exit the program
    }
    else
    {
        return EXIT_FAILURE; // Exit the program
    }
}