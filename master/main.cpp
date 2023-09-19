#include "master.h" 
#include "slave.h"

void thread_function(Slave& drive){

    drive.enable_powerstage();
    drive.acknowledge_faults();
    drive.referencing_task();

    drive.position_task(20000000, 100000, true);
    drive.position_task(-5000000, 10000, false);
}

int main(int argc, char* argv[])
{
    char ifaceName[] = "\\Device\\NPF_{A00D620E-09CB-4317-9A0F-4DAAD76B366E}"; // Your network interface name here

    Master ecMaster(ifaceName, 8000); 

    if (ecMaster.connected()){
        Slave drive(ecMaster, 1);
        Slave drive2(ecMaster, 2);

        std::thread thread1(thread_function, std::ref(drive));
        std::thread thread2(thread_function, std::ref(drive2));

        thread1.join();
        thread2.join();

        return EXIT_SUCCESS; 
    }
    else{
        return EXIT_FAILURE;
    }
}