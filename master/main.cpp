#include "master.h" 
#include "slave.h"

char ifaceName[] = "\\Device\\NPF_{A00D620E-09CB-4317-9A0F-4DAAD76B366E}"; // Your network interface name here
    
Master ecMaster(ifaceName, 8000); 

void thread_function(Slave& slave){
    slave.acknowledge_faults();
    slave.enable_powerstage();
    slave.referencing_task();
    slave.record_task(2);
    slave.jog_task(true, false, 5.0);
}

int main(int argc, char* argv[]){
    if (ecMaster.connected()){
        Slave slave1(ecMaster, 1);
        Slave slave2(ecMaster, 2);
        std::thread t1(thread_function, slave1);
        std::thread t2(thread_function, slave2);

        t1.join();
        t2.join();
    
        return EXIT_SUCCESS; 
    }
    else{
        return EXIT_FAILURE;
    }
}