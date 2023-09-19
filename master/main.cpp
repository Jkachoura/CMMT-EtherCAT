#include "master.h" 
#include "slave.h"

int main(int argc, char* argv[])
{
    char ifaceName[] = "\\Device\\NPF_{A00D620E-09CB-4317-9A0F-4DAAD76B366E}"; // Your network interface name here

    Master ecMaster(ifaceName, 8000); 

    if (ecMaster.connected()){
        Slave drive(ecMaster, 1);
        drive.enable_powerstage();
        drive.acknowledge_faults();
        
        float32 floatValue = 0.123;
        ecMaster.write_pdo(1, 0x2168, 4, &floatValue, sizeof(floatValue));

        float32 value;
        int valueSize = sizeof(value);
        ecMaster.read_pdo(1, 0x2168, 2, &value, &valueSize);
        printf("Value: %f\n", value);
        
        return EXIT_SUCCESS; 
    }
    else{
        return EXIT_FAILURE;
    }
}