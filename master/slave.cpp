#include "slave.h"

/** 
 * Slave class functions uses the master class functions,
 * this is to "call" the functions from the slave. 
 * A preferable method in Festo.
 */
Slave::Slave(Master& master, int slaveNr) : master(master), slaveNr(slaveNr) {}

/**
 * Enable the powerstage of the drive
 * 
 * @param slaveNr Slave number
 * 
 * @return EXIT_SUCCESS or EXIT_FAILURE
 * @see enable_powerstage from master
 */
int Slave::enable_powerstage(){
    return master.enable_powerstage(this->slaveNr);
}

/**
 * Disable the powerstage of the drive
 * 
 * @param slaveNr Slave number
 * 
 * @return EXIT_SUCCESS or EXIT_FAILURE
 * @see disable_powerstage from master
 */
int Slave::disable_powerstage(){
    return master.disable_powerstage(this->slaveNr);
}

/**
 * Perform the referencing task. If successful, the drive is in the homed state.
 * 
 * @param slaveNr Slave number
 * @param always Always perform the referencing task
 * 
 * @return EXIT_SUCCESS or EXIT_FAILURE
 * @see referencing_task from master
 */
int Slave::referencing_task(bool always){
    return master.referencing_task(this->slaveNr);
}

/**
 * Perform a jogging task with given duration
 * 
 * @param slaveNr Slave number
 * @param jog_positive Jog positive
 * @param jog_negative Jog negative
 * @param duration Duration in seconds (0 = nonblocking)
 * 
 * @see jog_task from master
 * @note The jogging motion stops if jog_positive and jog_negative are equal
 */
void Slave::jog_task(bool jog_positive, bool jog_negative, float duration) {
    master.jog_task(this->slaveNr, jog_positive, jog_negative, duration);
}

/**
 * Stops any currently active motion task
 * 
 * @param slaveNr Slave number
 * 
 * @see stop_motion_task from master
 */
void Slave::stop_motion_task(){
    master.stop_motion_task(this->slaveNr);
}

/**
 * Position task with velocity
 * 
 * @param slaveNr Slave number
 * @param target Target position
 * @param velocity Velocity
 * @param absolute Absolute or relative(false) movement
 * @param nonblocking Nonblocking or blocking movement
 * 
 * @return EXIT_SUCCESS or EXIT_FAILURE
 * @see position_task from master
 */
int Slave::position_task(int32_t target, int32_t velocity, bool absolute, bool nonblocking){
    return master.position_task(this->slaveNr, target, velocity, absolute, nonblocking);
}

/**
 * Position task with velocity
 * 
 * @param slaveNr Slave number
 * @param target Target position
 * @param velocity Velocity
 * @param absolute Absolute or relative(false) movement
 * @param nonblocking Nonblocking or blocking movement
 * 
 * @return EXIT_SUCCESS or EXIT_FAILURE
 * @see position_task from master
 */
int Slave::position_task(int32_t target, uint32_t velocity, bool absolute, bool nonblocking){
    return master.position_task(this->slaveNr, target, velocity, absolute, nonblocking);
}

/**
 * Position task with acceleration and deceleration
 * 
 * @param slaveNr Slave number
 * @param target Target position
 * @param velocity Velocity
 * @param acceleration Acceleration
 * @param deceleration Deceleration
 * @param absolute Absolute or relative(false) movement
 * @param nonblocking Nonblocking or blocking movement
 * 
 * @return EXIT_SUCCESS or EXIT_FAILURE
 * @see position_task from master
 */
int Slave::position_task(int32_t target, uint32_t velocity, uint32_t acceleration, uint32_t deceleration, bool absolute, bool nonblocking){
    return master.position_task(this->slaveNr, target, velocity, acceleration, deceleration, absolute, nonblocking);
}

/**
 * @brief Wait for the target position to be reached
 * 
 * @return true 
 * @return false 
 * @see wait_for_target_position from master
 */
bool Slave::wait_for_target_position(){
    return master.wait_for_target_position(this->slaveNr);
}

/**
 * Acknowledge faults
 * 
 * @param slaveNr Slave number
 * 
 * @see acknowledge_faults from master
 */
void Slave::acknowledge_faults(){
    master.acknowledge_faults(this->slaveNr);
}

/**
 * @brief Perform a preconfigured record task by providing the corresponding record number
 * 
 * @param slaveNr The slave number to perform the record task on
 * @param record The record number determining the record table entry that should be
 *               executed.
 *
 * @return int EXIT_SUCCESS or EXIT_FAILURE
 * @see record_task from master
 */
int Slave::record_task(int32_t record){
    return master.record_task(this->slaveNr, record);
}

/**
 * @brief Perform a velocity task with given velocity and duration
 * 
 * @param slaveNr The slave number to perform the velocity task on
 * @param velocity The velocity to be performed
 * @param duration The duration of the velocity task
 *                 A duration of 0 starts the task and returns immediately.
 * 
 * @return int EXIT_SUCCESS or EXIT_FAILURE
 * @see velocity_task from master
 */
int Slave::velocity_task(int32_t velocity, float duration){
    return master.velocity_task(this->slaveNr, velocity, duration);
}

/**
 * Write on a Service Data Object (SDO) from a slave device.
 *
 * @param slaveNr    The slave number to write to.
 * @param index      The index of the SDO (e.g., 0x2168).
 * @param subindex   The subindex of the SDO.
 * @param value      A pointer to the value to be written to the SDO.
 * @param valueSize  The size (in bytes) of the value.
 * 
 * @note
 *   To use this function, you must define the value beforehand and pass a pointer to it as the 'value' parameter.
 *   For example:
 *   - For a float32 value: float32 floatValue = 0.123; write_sdo(slaveNr, index, subindex, &floatValue, sizeof(floatValue));
 *   - For an int32 value: int32_t intValue = 42; write_sdo(slaveNr, index, subindex, &intValue, sizeof(intValue));
 *   This function sends the specified data to the slave device using CoE SDO write.
 * 
 * @see write_sdo from master
 */
void Slave::write_sdo(uint16 index, uint8 subindex, void *value, int valueSize){
    master.write_sdo(this->slaveNr, index, subindex, value, valueSize);
}

/**
 * Read a Service Data Object (SDO) from a slave device.
 *
 * @param slaveNr     The slave number to read from.
 * @param index       The index of the SDO (e.g., 0x2168).
 * @param subindex    The subindex of the SDO.
 * @param value       A pointer to store the read value from the SDO.
 * @param valueSize   A pointer to an integer to store the size (in bytes) of the read value.
 * 
 * @note
 *   To use this function, you must provide a pointer to a buffer where the read value will be stored ('value') 
 *   and a pointer to an integer ('valueSize') to store the size of the read value.
 *   For example:
 *   - float32 floatValue; int size; read_sdo(slaveNr, index, subindex, &floatValue, &size);
 *   - int32_t intValue; int size; read_sdo(slaveNr, index, subindex, &intValue, &size);
 *   This function reads the specified SDO from the slave device using CoE SDO read.
 * 
 * @see read_sdo from master
 */
void Slave::read_sdo(uint16 index, uint8 subindex, void *value, int *valueSize){
    master.read_sdo(this->slaveNr, index, subindex, value, valueSize);
}