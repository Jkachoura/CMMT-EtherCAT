#include "master.h" 
#include "slave.h"

char ifaceName[] = "\\Device\\NPF_{A00D620E-09CB-4317-9A0F-4DAAD76B366E}"; // Your network interface name here
    
Master ecMaster(ifaceName, 8000); 

int main(int argc, char* argv[]){
    if (ecMaster.connected()){
        Slave slave1(ecMaster, 1);
        Slave slave2(ecMaster, 2);

        ecMaster.acknowledge_faults(1);
        ecMaster.enable_powerstage(1);
        ecMaster.referencing_task(1);

        ecMaster.velocity_task(1, 1000000);
    
        return EXIT_SUCCESS; 
    }
    else{
        return EXIT_FAILURE;
    }
}