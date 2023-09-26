#include "master.h" 
#include "slave.h"

char ifaceName[] = "\\Device\\NPF_{A00D620E-09CB-4317-9A0F-4DAAD76B366E}"; // Your network interface name here
    
Master ecMaster(ifaceName, 8000); 

void thread_function(int slave_number){
    ecMaster.acknowledge_faults(slave_number);
    ecMaster.enable_powerstage(slave_number);
    ecMaster.referencing_task(slave_number);

    ecMaster.record_task(slave_number, 2);
}

int main(int argc, char* argv[]){
    if (ecMaster.connected()){
        std::thread t1(thread_function, 1);
        std::thread t2(thread_function, 2);

        t1.join();
        t2.join();
    
        return EXIT_SUCCESS; 
    }
    else{
        return EXIT_FAILURE;
    }
}