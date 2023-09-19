#include "master.h"

/**
 * @brief This class is used to call the functions of the EtherCAT Master from a slave object
 * 
 */
class Slave {
    public:
        Slave(Master& master, int slaveNr);

        int enable_powerstage();
        int disable_powerstage();
        int referencing_task(bool always = false);
        void jog_task(bool jog_positive, bool jog_negative, float duration);
        void stop_motion_task();
        int position_task(int32_t target, int32_t velocity, bool absolute = false, bool nonblocking = false);
        int position_task(int32_t target, uint32_t velocity, bool absolute = false, bool nonblocking = false);
        int position_task(int32_t target, uint32_t velocity, uint32_t acceleration, uint32_t deceleration, bool absolute = false, bool nonblocking = false);
        bool wait_for_target_position();
        void acknowledge_faults();

    private:
        Master& master;      // Reference to the EtherCAT master
        int slaveNr;     // The slave's number or identifier
};