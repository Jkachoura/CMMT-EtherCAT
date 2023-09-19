#include "master.h" 
#include "slave.h"

int main(int argc, char* argv[])
{
    char ifaceName[] = "\\Device\\NPF_{A00D620E-09CB-4317-9A0F-4DAAD76B366E}"; // Your network interface name here

    Master ecMaster(ifaceName, 16000); 

    if (ecMaster.connected()){
        Slave drive(ecMaster, 1);
        Sleep(5000);
        drive.acknowledge_faults();
        Sleep(5000);
        drive.enable_powerstage(); 

        
        
        Sleep(1000);
        return EXIT_SUCCESS; 
    }
    else{
        return EXIT_FAILURE;
    }
}