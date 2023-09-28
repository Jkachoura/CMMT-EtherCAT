// master.h : Header file for your target.

#define DEBUG

#pragma once
#include<iostream>
#ifndef INTMAX_MAX
    #define INTMAX_MAX MAXINT64
#endif // needed for thread on some platforms
#include"ethercat.h"
#include<thread>
#include<mutex>

constexpr int EC_TIMEOUTMON = 500;

/**
 * @brief  This class is used to control the EtherCAT Master
 * 
 */
class Master
{
    typedef enum {
        control_switch_on = 0,
        control_enable_voltage = 1,
        control_quick_stop = 2,
        control_enable_operation = 3,
        control_fault_reset = 7,
        control_4 = 4,
        control_5 = 5,
        control_6 = 6,
        control_halt = 8,
        control_9 = 9,
    }control_bit_t;
    typedef enum {
        status_ready_to_switch_on = 0,
        status_switched_on = 1,
        status_operation_enabled = 2,
        status_fault = 3,
        status_voltage_enabled = 4,
        status_quick_stop = 5,
        status_switch_on_disabled = 6,
        status_warning = 7,
        status_manfsp = 8,
        status_remote = 9,
        status_mc = 10, // motion complete / target reached // not jogging in jogging mode // record sequence complete
        status_ack_start = 12,
        status_ref_reached = 12,
        status_rc = 13, // individual record complete / velocity following error
        status_ref = 15, //drive homed
    }statusword_bit_t;
    typedef enum {
        Controlword = 0,
        Statusword = 0,
        Mode_of_Operation = 2,
        Mode_of_Operation_Display = 2,
        Target_Position = 3,
        Position_Actual_Value = 3,
        Profile_velocity = 7,
        Target_velocity = 11,
        Velocity_Actual_Value = 11,
        Target_Torque = 15,
        Torque_Actual_Value = 15,
        Velocity_Offset = 17,
        Torque_Offset = 21,
    }mapped_PDO_t;
    typedef enum {
        no_mode = 0,
        profile_position_mode = 1,
        velocity_mode = 2,
        profile_velocity_mode = 3,
        profile_torque_mode = 4,

        homing_mode = 6,
        interpolated_position_mode = 7,
        cyclic_sync_pos_mode = 8,
        cyclic_sync_vel_mode = 9,
        cyclic_sync_tor_mode = 10,

        record_mode = 236, //int8 = -20 || uint8 = 236

        jog_mode = 253, // uint8 = 253 || int8 = -3
    }Mode_of_Operation_t;
    public:
        // Constructor / Destructor
        Master(char ifname[] = "eth0", const uint32_t cycletime = 2000, bool showNonErrors = true);
        ~Master();

        // status
        int getError(int slaveNr);
        int16_t get16(int slaveNr, uint8_t byte);
        int32_t getPos(int slaveNr);
        int32_t getRec(int slaveNr);
        bool connected(); // Check if drives are ready to use (in operation mode)

        
        // control
        int enable_powerstage(int slaveNr);
        int disable_powerstage(int slaveNr);
        int referencing_task(int slaveNr,bool always = true);
        void jog_task(int slaveNr, bool jog_positive, bool jog_negative, float duration);
        void stop_motion_task(int slaveNr);
        int record_task(int slaveNr, int32_t record);
        int position_task(int slaveNr, int32_t target, bool absolute = false, bool nonblocking = false);
        int position_task(int slaveNr, int32_t target, int32_t velocity, bool absolute = false, bool nonblocking = false);
        int position_task(int slaveNr, int32_t target, uint32_t velocity, bool absolute = false, bool nonblocking = false);
        int position_task(int slaveNr, int32_t target, uint32_t velocity, uint32_t acceleration, uint32_t deceleration, bool absolute = false, bool nonblocking = false);
        int velocity_task(int slaveNr, int32_t velocity, float duration);
        bool wait_for_target_position(int slaveNr);
        int reset(int slaveNr);
        void waitCycle(); // Wait for the cycle time
        void acknowledge_faults(int slaveNr);
        void write_sdo(uint16 slaveNr, uint16 index, uint8 subindex, void *value, int valueSize);
        void read_sdo(uint16 slaveNr, uint16 index, uint8 subindex, void *value, int *valueSize);

    private:
        uint32_t ctime; // Store the cycle time in microseconds
        std::mutex m; // prevent acces to EC data at the same time
        
        char IOmap[4096];
        volatile int wkc;

        bool inOP;
        bool verbose; // Output to screen
        uint8_t currentgroup = 0;

        char mode[9]; 
        int32_t target;

        bool readyState(int slaveNr);

        // Data handling
        uint8_t setBit(int slaveNr, uint8_t bit, uint8_t byte = Controlword);
        uint8_t unsetBit(int slaveNr, uint8_t bit, uint8_t byte = Controlword);
        uint16_t unsetControl(int slaveNr);
        bool getBit(int slaveNr, uint8_t bit, uint8_t byte = Statusword);
        void setByte(int slaveNr, uint8_t value, uint8_t byte = Controlword);
        void set16(int slaveNr, int16_t value, uint8_t byte);
        void setPos(int slaveNr, int32_t target, uint8_t byte = Target_Position);
        void setProfileVelocity(int slaveNr, uint32_t velocity, uint8_t byte = Profile_velocity);
        void setTargetVelocity(int slaveNr, uint32_t velocity, uint8_t byte = Target_velocity);
        void setRec(int slaveNr, int32_t record);
        int  startup();
        void cycle(); // send and recieve data, wait cycletime 
        int  setMode(int slaveNr, uint8_t mode);

        // create PDO's
        int mapCia402(uint16_t slaveNr);
        
        // Ethercat state
        void setPreOp(int slaveNr);

        //Thread
        std::thread cycle_thread;
 };