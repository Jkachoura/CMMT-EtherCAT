#include "master.h" // To include the EtherCat Master library

int main(int argc, char* argv[])
{
    char ifaceName[] = "\\Device\\NPF_{A00D620E-09CB-4317-9A0F-4DAAD76B366E}"; // Your network interface name here

    Master ecMaster(ifaceName, 8000); // 8000 = 8 ms cycle time

    if (ecMaster.connected()){ // Check if EtherCat Master reached the Operational State
        for (int i = 1; i <= ec_slavecount; i++)
        {
            ecMaster.reset(i);  // reset all drives
        }

        Slave drive(ecMaster, 1); // Create a slave object with ID 1
        drive.enable_powerstage(); // Enable the powerstage of the drive
        drive.referencing_task(); // Start the referencing task
        drive.jog_task(true, false, 15); // Jog the drive positive for 2 seconds
        drive.jog_task(false, true, 15); // Jog the drive negative for 2 seconds
        drive.jog_task(true, false, 0); // Jog the drive positive for 2 seconds
        Sleep(5000); // Wait 2 seconds
        drive.stop_motion_task(); // Stop the motion of the drive
        Sleep(2000); // Wait 2 seconds
        drive.disable_powerstage(); // Disable the powerstage of the drive
        Sleep(1000);
        drive.enable_powerstage(); // Enable the powerstage of the drive
        Sleep(1000);
        drive.position_task(1000000, 100000, true, false); // Move the drive to position 1000000 with a velocity of 100000
        drive.position_task(0, 100000, false, true); // Move the drive to position 0 with a velocity of 100000
        drive.wait_for_target_position(); // Wait until the drive reached the target position
        return EXIT_SUCCESS; // Exit the program
    }
    else{
        return EXIT_FAILURE; // Exit the program
    }
}