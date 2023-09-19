#include "master.h" 
#include "slave.h"

int main(int argc, char* argv[])
{
    char ifaceName[] = "\\Device\\NPF_{A00D620E-09CB-4317-9A0F-4DAAD76B366E}"; // Your network interface name here

    Master ecMaster(ifaceName, 16000); 

    if (ecMaster.connected()){
        for (int i = 1; i <= ec_slavecount; i++)
        {
            ecMaster.reset(i);  
        }

        Slave drive(ecMaster, 1);

        drive.enable_powerstage(); 
        drive.referencing_task(); 

        drive.position_task(-10000000, 300000, true, true); 
        Sleep(1000);
        drive.position_task(10000000, 300000, true, true); 
        Sleep(1000);
        drive.position_task(-10000000, 300000, true, true);
        Sleep(1000);
        drive.position_task(10000000, 300000, true, true);

        drive.wait_for_target_position();
        return EXIT_SUCCESS; 
    }
    else{
        return EXIT_FAILURE;
    }
}