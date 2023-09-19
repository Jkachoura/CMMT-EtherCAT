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
 * @note The jogging motion stops if jog_positive and jog_negative are equal
 */
void Slave::jog_task(bool jog_positive, bool jog_negative, float duration) {
    master.jog_task(this->slaveNr, jog_positive, jog_negative, duration);
}

/**
 * Stops any currently active motion task
 * 
 * @param slaveNr Slave number
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
 */
bool Slave::wait_for_target_position(){
    return master.wait_for_target_position(this->slaveNr);
}

void Slave::acknowledge_faults(){
    master.acknowledge_faults(this->slaveNr);
}