#include "master.h" 
#include "slave.h"

char ifaceName[] = "\\Device\\NPF_{A00D620E-09CB-4317-9A0F-4DAAD76B366E}"; // Your network interface name here
    
Master ecMaster(ifaceName, 8000); 

int main(int argc, char* argv[]){
    if (ecMaster.connected()){
        Slave slave1(ecMaster, 1);
        Slave slave2(ecMaster, 2);

        slave1.acknowledge_faults();
        slave1.enable_powerstage();
        slave1.referencing_task();

        slave2.acknowledge_faults();
        slave2.enable_powerstage();
        slave2.referencing_task();

        slave2.velocity_task(100000, 5);

    
        return EXIT_SUCCESS; 
    }
    else{
        return EXIT_FAILURE;
    }
}