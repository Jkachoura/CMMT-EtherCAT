#include "master.h" // To include the EtherCat Master library

int main(int argc, char* argv[])
{
    char ifaceName[] = "\\Device\\NPF_{DEA85026-34BA-4C8B-9840-A3CE7793A348}"; // Your network interface name here

    Master ecMaster(ifaceName, 8000); // 8000 = 8 ms cycle time

    if (ecMaster.connected()) // Check if EtherCat Master reached the Operational State
    {
        for (int i = 1; i <= ec_slavecount; i++)
        {
            ecMaster.reset(i);  // reset all drives
        }

        int slaveNr = 1; // Slave to move in this example 0 = Master, slaves numbered in connected order
        ecMaster.enable(slaveNr); // Power the drive

        ecMaster.home(slaveNr, true); // Home the drive, even when already homed (true)
        ecMaster.jogPos(slaveNr);
        Sleep(5000); // delay next instruction for 5 seconds
        ecMaster.jogNeg(slaveNr);
        Sleep(5000); // delay next instruction for 5 seconds
        ecMaster.jogStop(slaveNr);
        Sleep(5000); // delay next instruction for 5 seconds
        ecMaster.disable(slaveNr); // Power off the drive
        Sleep(5000); // delay next instruction for 5 seconds
        ecMaster.enable(slaveNr); // Power the drive

        ecMaster.movePosition(slaveNr, 20000000, 1000000); // absolute movement position and velocity
        ecMaster.movePosition(slaveNr, -5000000, 100000, true); // relative movement (true)

        ecMaster.movePosition(slaveNr, 20000000, 1000000, 100000, 100000); // absolute movement position and velocity

        return EXIT_SUCCESS; // Exit the program
    }
    else
    {
        return EXIT_FAILURE; // Exit the program
    }
}