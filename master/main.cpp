#include "master.h"
#include "slave.h"

int main(int argc, char* argv[]) {
    // Your network interface name here
    char ifaceName[] = "\\Device\\NPF_{A00D620E-09CB-4317-9A0F-4DAAD76B366E}";
    Master ecMaster(ifaceName, 8000);

    if (ecMaster.connected()) {
        Slave ecSlave(ecMaster, 1);

        ecSlave.acknowledge_faults();
        ecSlave.enable_powerstage();
        ecSlave.referencing_task();

        // Relative
        ecSlave.position_task(100000, 600000);
        ecSlave.position_task(-50000, 50000);
        // Absolute
        ecSlave.position_task(300000, 600000, true);

        return EXIT_SUCCESS;
    }
    else {
        return EXIT_FAILURE;
    }
}