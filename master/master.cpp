// master.cpp : Source file for your target.

#include "master.h"

/**
 * Constructor for the EtherCat Master
 * 
 * @param ifname Network interface name
 * @param cycletime Cycle time in microseconds
 * @param showNonErrors Show non errors
 */
Master::Master(char ifname[], const uint32_t cycletime, bool showNonErrors){
    /* init values */
    this->inOP = FALSE;
    this->ctime = cycletime;
    this->verbose = showNonErrors;
    memset(IOmap, 0, sizeof(IOmap)); // needed for unitialised values

    auto retry = 3;

    if (verbose)puts("Starting Ethercat Master");
    /* initialise SOEM, bind socket to ifname */
    if (ec_init(ifname)){
        if (verbose)printf("ec_init on %s succeeded.\n", ifname);
        while (startup() != EXIT_SUCCESS && retry--);

        if (inOP){
            cycle_thread = std::thread(&Master::cycle, this);
        }
        else{
            printf("Unable to start EtherCat Master");
        }
    }
    else{
        printf("No socket connection on %s\nExcecute as Administrator/root and verify your network adapter name\n", ifname);
    }
}

/**
 * Destructor for the EtherCat Master
 * 
 * @note This function is called when the program exits
 */
Master::~Master(){
    if (inOP){
        if (verbose){
            puts("Shutting down Ethercat");
            printf("\nRequest init state for all slaves\n");
        }
        ec_slave[0].state = EC_STATE_INIT; // 0 = master
        /* request INIT state for all slaves */
        ec_writestate(0); // 0 = master
        for (int timeout= this->ctime; timeout && ec_readstate() != EC_STATE_INIT;timeout--) waitCycle();
        inOP = FALSE;
        cycle_thread.join();
        if (verbose && ec_readstate() == EC_STATE_INIT)puts("Clean Exit");
        else puts("Could not exit cleanly");
        if (verbose) puts("Closing connection");
        ec_close();// stop SOEM, close socket
    }
}

/**
 * Checks if the slave is in the operational state
 * 
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
bool Master::readyState(int slaveNr){
    bool retVal = this->inOP; 
    if (retVal){
        retVal = getBit(slaveNr, status_operation_enabled);
    }
    else{
        printf("Slave %d not operational\n",slaveNr);
    }
    return retVal; 
}

/**
 * Set a bit
 * 
 * @param slaveNr Slave number
 * @param bit Bit to set
 * @param byte Byte to start writing
 * 
 * @return base Base value 
 */
uint8_t Master::setBit(int slaveNr, uint8_t bit, uint8_t byte){
    while (bit > 7){
        bit -= 8;
        byte += 1;
        if (byte >= ec_slave[slaveNr].Obytes) byte = 0; // prevent out of bounds
    }
    
    uint8_t base = *(ec_slave[slaveNr].outputs + byte);
    base |= (1 << bit);
    this->setByte(slaveNr, base, byte);
    return base;
}

/**
 * Unset a bit
 * 
 * @param slaveNr Slave number
 * @param bit Bit to unset
 * @param byte Byte to start writing
 * 
 * @return base Base value 
 */
uint8_t Master::unsetBit(int slaveNr, uint8_t bit, uint8_t byte){
    while (bit > 7){
        bit -= 8;
        byte += 1;
        if (byte >= ec_slave[slaveNr].Obytes) byte = 0; // prevent out of bounds
    }
    
    uint8_t base = *(ec_slave[slaveNr].outputs + byte);
    base &= ~(1 << bit);
    this->setByte(slaveNr, base, byte);
    return base;
}

/**
 * Unset all control bits
 * 
 * @param slaveNr Slave number
 * 
 * @return return value
 */
uint16_t Master::unsetControl(int slaveNr){
    uint8 byte0=0, byte1=0;

    //Unset all control bits from previous mode
    byte0 = unsetBit(slaveNr, control_4, Controlword);
    byte0 = unsetBit(slaveNr, control_5, Controlword);
    byte0 = unsetBit(slaveNr, control_6, Controlword);
    byte1 = unsetBit(slaveNr, control_9, Controlword);

    const uint16 retVal = (byte1 << 8) + byte0;

    return retVal;
}

/**
 * Get the value of a bit
 * 
 * @param slaveNr Slave number
 * @param bit Bit to get
 * @param byte Byte to start reading
 * 
 * @return true if bit is set
 */
bool Master::getBit(int slaveNr, uint8_t bit, uint8_t byte){
    while (bit > 7){
        bit -= 8;
        byte += 1;
        if (byte >= ec_slave[slaveNr].Ibytes) byte = 0; // Prevent out of bounds
    }

    const auto retVal = *(ec_slave[slaveNr].inputs + byte) & (1 << bit);
    return retVal;
}

/**
 * Sets a byte
 * 
 * @param slaveNr Slave number
 * @param value Value to set
 * @param byte Byte to start writing
 */
void Master::setByte(int slaveNr, uint8_t value, uint8_t byte){
    m.lock();
    *(ec_slave[slaveNr].outputs + byte) = value;
    m.unlock();
}

/**
 * Gets errors on the slave 
 * 
 * @param slaveNr Slave number
 * 
 * @return Number of errors
 */
int Master::getError(int slaveNr){
    auto retval = 0;
    if(getBit(slaveNr, status_fault)) retval--;
    if (getBit(slaveNr, status_warning)) retval--;
    return retval;
}

/**
 * Startup function for the EtherCat Master
 * 
 * @param slaveNr Slave number
 * @param byte Byte to start reading
 * 
 * @return return value of @see ec_init
 */
int16_t Master::get16(int slaveNr, uint8_t byte){
    auto retVal = 0;
    
    retVal += (*(ec_slave[slaveNr].inputs + byte + 1) << 8);
    retVal += (*(ec_slave[slaveNr].inputs + byte + 0));

    return retVal;
}

/**
 * Get the position of the drive
 * 
 * @param slaveNr Slave number
 * 
 * @return Position
 */
int32_t Master::getPos(int slaveNr){
    auto retVal = 0;
    const int positionActualValueAddress = 3;
       
    retVal += (*(ec_slave[slaveNr].inputs + positionActualValueAddress + 3) << 24);
    retVal += (*(ec_slave[slaveNr].inputs + positionActualValueAddress + 2) << 16);
    retVal += (*(ec_slave[slaveNr].inputs + positionActualValueAddress + 1) << 8);
    retVal += (*(ec_slave[slaveNr].inputs + positionActualValueAddress + 0));

    return retVal;
}

/**
 * Get the record number that currently is being executed
 * 
 * @param slaveNr Slave number
 * 
 * @return record number
 */
int32_t Master::getRec(int slaveNr){
    int retVal = 0;
    int retvalSize = sizeof(retVal);
    int wc = ec_SDOread(slaveNr, 0x216F, 0x14, false, &retvalSize, &retVal, EC_TIMEOUTRXM);
    return retVal;
}

/**
 * Startup function for the EtherCat Master
 */
bool Master::connected(){
    return this->inOP;
}

/**
 * Set a value of 16 bits
 */
void Master::set16(int slaveNr, int16_t value, uint8_t byte){
    m.lock();
    *(ec_slave[slaveNr].outputs + byte + 1) = (value >> 8) & 0xFF;
    *(ec_slave[slaveNr].outputs + byte + 0) = value & 0xFF;
    m.unlock();
}

/**
 * Set the position
 * 
 * @param slaveNr Slave number
 * @param target Target position
 * @param byte Byte to start writing
 */
void Master::setPos(int slaveNr, int32_t target, uint8_t byte ){
    m.lock();
    *(ec_slave[slaveNr].outputs + byte + 3) = (target >> 24) & 0xFF;
    *(ec_slave[slaveNr].outputs + byte + 2) = (target >> 16) & 0xFF;
    *(ec_slave[slaveNr].outputs + byte + 1) = (target >>  8) & 0xFF;
    *(ec_slave[slaveNr].outputs + byte + 0) = target & 0xFF;
    m.unlock();
}

/**
 * @brief Set velocity target
 * 
 * @param slaveNr 
 * @param velocity 
 */
void Master::setTargetVelocity(int slaveNr, uint32_t velocity, uint8_t byte){
    m.lock();
    *(ec_slave[slaveNr].outputs + byte + 3) = (velocity >> 24) & 0xFF;
    *(ec_slave[slaveNr].outputs + byte + 2) = (velocity >> 16) & 0xFF;
    *(ec_slave[slaveNr].outputs + byte + 1) = (velocity >> 8) & 0xFF;
    *(ec_slave[slaveNr].outputs + byte + 0) = velocity & 0xFF;
    m.unlock();
}

/**
 * Set the profile velocity
 * 
 * @param slaveNr Slave number
 * @param velocity Velocity
 * @param byte Byte to start writing
 */
void Master::setProfileVelocity(int slaveNr, uint32_t velocity, uint8_t byte){
    m.lock();
    *(ec_slave[slaveNr].outputs + byte + 3) = (velocity >> 24) & 0xFF;
    *(ec_slave[slaveNr].outputs + byte + 2) = (velocity >> 16) & 0xFF;
    *(ec_slave[slaveNr].outputs + byte + 1) = (velocity >> 8) & 0xFF;
    *(ec_slave[slaveNr].outputs + byte + 0) = velocity & 0xFF;
    m.unlock();
}

/**
 * @brief Set the Record number 
 * 
 * @param slaveNr Slave number
 * @param record The record you want to excute
 */
void Master::setRec(int slaveNr, int32_t record){
    m.lock();
    // Record number that is to be started is selecte via the Next record table index (0x216F.14)
    write_sdo(slaveNr, 0x216F, 0x14, &record, sizeof(record));
    m.unlock();
}

/**
 * Resets the slave
 * 
 * @param slaveNr Slave number
 * 
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int Master::reset(int slaveNr){
    unsigned int timeout = 1000;
    if(verbose)printf("Resetting slave nr : %d\n",slaveNr);
    if (this->inOP){
        m.lock();
        memset(ec_slave[slaveNr].outputs, 0, ec_slave[slaveNr].Obytes); // Start empty to prevent retriggering error
        m.unlock();
        if (verbose)printf("Wait for empty frame slave nr : %d\n", slaveNr);
        waitCycle(); // Wait for empty frame to be send

        while ((getError(slaveNr) != 0) && timeout--){
            setBit(slaveNr, control_fault_reset);
            if (verbose)printf("Waiting on fault slave nr : %d\n", slaveNr);
            waitCycle();
            unsetBit(slaveNr, control_fault_reset);
        }

        if (verbose){
            if (getError(slaveNr) == 0) printf("Resetting slave nr : %d done\n", slaveNr);
        }
        else if (getError(slaveNr) != 0){
            if (getBit(slaveNr, status_fault) && getBit(slaveNr, status_warning)) printf("Resetting slave nr : %d failed\n", slaveNr);
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    else{
        printf("Slave %d not in operational mode\n",slaveNr);
        return EXIT_FAILURE;
    }
   
}

/**
 * Wait for the cycle time
 */
void Master::waitCycle(){
    std::this_thread::sleep_for(std::chrono::microseconds(this->ctime));
}

/**
 * Write on a Service Data Object (SDO) to a slave device.
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
 */
void Master::write_sdo(uint16 slaveNr, uint16 index, uint8 subindex, void *value, int valueSize) {
    if(valueSize >= 0){
        int result = ec_SDOwrite(slaveNr, index, subindex, false, valueSize, value, EC_TIMEOUTRXM);
    
        if (result == 0) {
            printf("Error: %d\n", result);
        }
        else{
            printf("Write successful\n");
        }
    }
    else{
        printf("Error: valueSize must be greater than 0\n");
    }
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
 */
void Master::read_sdo(uint16 slaveNr, uint16 index, uint8 subindex, void *value, int *valueSize) {
    int result = ec_SDOread(slaveNr, index, subindex, false, valueSize, value, EC_TIMEOUTRXM);

    if (result == 0){
        printf("Error: %d\n", result);
    }
    else{
        printf("Read successful\n");
    }
}
/**
 * Enable the powerstage of the drive
 * 
 * @param slaveNr Slave number
 * 
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int Master::enable_powerstage(int slaveNr){
    unsigned int timeout = 1000000;
    if (verbose)printf("Start Enabling Drive %d\n", slaveNr);
    if (reset(slaveNr) == EXIT_SUCCESS){
        while ((!getBit(slaveNr, status_voltage_enabled) || !getBit(slaveNr, status_quick_stop)) && timeout--){
            setBit(slaveNr, control_quick_stop);
            setBit(slaveNr, control_enable_voltage);
            waitCycle();
        }

        while (!getBit(slaveNr, status_operation_enabled) && timeout--){
            setBit(slaveNr, control_enable_operation);
            setBit(slaveNr, control_switch_on);
            waitCycle();
        }

        if (getBit(slaveNr, status_voltage_enabled) && getBit(slaveNr, status_quick_stop) && getBit(slaveNr, status_operation_enabled)){
            if (verbose)printf("Enable Drive %d succesful\n", slaveNr);
            return EXIT_SUCCESS;
        }
        else{
            if (timeout == 0) printf("Timeout : ");
            printf("Enable Drive %d unsuccesful\n", slaveNr);
            return EXIT_FAILURE;
        }
    }
    else{
        printf("Enable Drive %d unsuccesful after unsuccesful reset\n", slaveNr);
        return EXIT_FAILURE;
    }
}

/**
 * Disable the powerstage of the drive
 * 
 * @param slaveNr Slave number
 * 
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int Master::disable_powerstage(int slaveNr){
    unsigned int timeout = 1000;
    unsetBit(slaveNr, control_enable_operation);
    unsetBit(slaveNr, control_switch_on);
    waitCycle();
    unsetBit(slaveNr, control_quick_stop);
    unsetBit(slaveNr, control_enable_voltage);
    
    while (getBit(slaveNr, status_operation_enabled) && timeout--){
        waitCycle();
    }
    if (timeout > 0){
        return EXIT_SUCCESS;
    }
    else{
        return EXIT_FAILURE;
    }
    
}

/**
 * Perform the referencing task. If successful, the drive is in the homed state.
 * 
 * @param slaveNr Slave number
 * @param always Always perform the referencing task
 * 
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int Master::referencing_task(int slaveNr, bool always){   
    if (readyState(slaveNr)){
        setMode(slaveNr, homing_mode);
        if (getBit(slaveNr, status_ref) && !always){
            printf("Slave %d already homed\n", slaveNr);
        }
        else{
            printf("Slave %d starting homing\n", slaveNr);
            unsetControl(slaveNr);
            setBit(slaveNr, control_4);
            while (!getBit(slaveNr, status_ref_reached)); // Check for rehoming
            unsetBit(slaveNr, control_4);
        }
        return 0;
    }
    else printf("Homing not possible slave %d not enabled\n ", slaveNr);

    return -1;
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
void Master::jog_task(int slaveNr, bool jog_positive, bool jog_negative, float duration) {
    if (jog_positive && jog_negative) {
        printf("Both positive and negative jog requested. Please choose one direction.\n");
        return;
    }

    if (readyState(slaveNr)) {
        uint8_t controlBit;
        if (jog_positive) {
            printf("Begin jog in positive direction\n");
            controlBit = control_4;
        } else if (jog_negative) {
            printf("Begin jog in negative direction\n");
            controlBit = control_5;
        } else {
            printf("No jog direction specified. Please specify either positive or negative jog.\n");
            return;
        }

        uint8_t controlbyte = setMode(slaveNr, jog_mode);
        unsetControl(slaveNr);
        while (!getBit(slaveNr, status_mc));
        controlbyte = setBit(slaveNr, controlBit);
        if (duration > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds((int) (duration * 1000)));
            stop_motion_task(slaveNr);
        }
    } else {
        printf("Jogging not possible, slave %d not enabled\n", slaveNr);
    }
}

/**
 * Stops any currently active motion task
 * 
 * @param slaveNr Slave number
 */
void Master::stop_motion_task(int slaveNr){
    if (verbose)printf("Stopping Movement\n");
    if (readyState(slaveNr)){
        unsetControl(slaveNr);
        while (!getBit(slaveNr, status_mc)) waitCycle();
    }
}

/**
 * Position task
 * 
 * @param slaveNr Slave number
 * @param target Target position
 * @param absolute Absolute or relative(false) movement
 * @param nonblocking Nonblocking or blocking movement
 * 
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int Master::position_task(int slaveNr, int32_t target, bool absolute, bool nonblocking){
    /**
     * Precondition for positioning mode
     * The following conditions must be fulfilled for positioning mode :
     * – Modes of operation display(0x6061) = 1
     * – Statusword(0x6041) = 1X0X X11X X011 0111b
     * Control and monitoring
     * Object 0x6040 : Controlword
     * The object controls the following functions of positioning mode :
     * – Bit 4 : start motion command(New set - point)
     * – Bit 5 : accept change immediately(Change set immediately)
     * Motion Control
     * Festo — CMMT - AS - SW — 2019 - 08c 303
     * – Bit 6: positioning type(absolute / relative)
     * – Bit 8 : stop motion command(Halt)
     */
    char modeVar[9] = "Relative";
    //Copy mode in to the global variable
    strncpy_s(mode, modeVar, 9);
    //Copy target in to the global variable
    this->target = target;
    if (absolute){
        strncpy_s(mode, "Absolute", 9);
    }
    if (verbose)printf("Starting %s movement to position %d of slave % d\n",mode , target, slaveNr);
    if (readyState(slaveNr)){
        setMode(slaveNr, profile_position_mode);
        unsetControl(slaveNr);
        if (!absolute)setBit(slaveNr, control_6);
        setPos(slaveNr, target);
        waitCycle();
        unsetBit(slaveNr, control_halt);
        setBit(slaveNr, control_4);
        if (nonblocking) {
            if (verbose) printf("Non-blocking mode: Movement initiated\n");
            return EXIT_SUCCESS;
        }
        while (!getBit(slaveNr, status_ack_start)); // Wait for ack to prevent response to previous mc 
        while (!getBit(slaveNr, status_mc)){
            if (verbose)printf("Move slave %d %s : %d %d\r", slaveNr, mode, target, getPos(slaveNr));
            unsetControl(slaveNr);
        }
        if (verbose)printf("\n");
        if (verbose)printf(" completed\n");
        return EXIT_SUCCESS;
    }
    else{
        printf("Drive %d not enabled, movement not possible\n", slaveNr);
    }
    return EXIT_FAILURE;
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
 * @return @see position_task
 */
int Master::position_task(int slaveNr, int32_t target, int32_t velocity, bool absolute, bool nonblocking){
    setProfileVelocity(slaveNr, velocity);
    if (velocity < 0) {
        printf("ERROR : Slave %d Velocity should be a positive number in positioning mode\n", slaveNr);
        return -1;
    }
    else return position_task(slaveNr, target, absolute, nonblocking);
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
 * @return @see position_task
 * 
 * @remark This function prevents an error
 */
int Master::position_task(int slaveNr, int32_t target, uint32_t velocity, bool absolute, bool nonblocking){
    setProfileVelocity(slaveNr, velocity);
    return position_task(slaveNr, target, absolute, nonblocking);
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
 * @return @see position_task
 */
int Master::position_task(int slaveNr, int32_t target, uint32_t velocity, uint32_t acceleration, uint32_t deceleration, bool absolute, bool nonblocking){
    auto retval = 0;
    // Writing acceleration
    retval += ec_SDOwrite(slaveNr, 0x6083, 0, false, sizeof(acceleration), &acceleration, EC_TIMEOUTRXM);
    
    // Writing deceleration
    retval += ec_SDOwrite(slaveNr, 0x6084, 0, false, sizeof(deceleration), &deceleration, EC_TIMEOUTRXM);

    if (retval == 2){
        retval = position_task(slaveNr, target, velocity, absolute, nonblocking);
        return retval;
    }
    else{
        printf("Writing of acceleration or deceleration failed on slave %d\n", slaveNr);
    }
    
    return EXIT_FAILURE;
}

/**
 * Wait for the target position to be reached
 * 
 * @param slaveNr Slave number
 * 
 * @return TRUE if target position is reached
 */
bool Master::wait_for_target_position(int slaveNr) {
    while (!getBit(slaveNr, status_ack_start)); // Wait for ack to prevent response to previous motion command
    while (!getBit(slaveNr, status_mc)) {
        if (verbose) {
            printf("Move slave %d %s : %d %d\r", slaveNr, mode, target, getPos(slaveNr));
        }
        unsetControl(slaveNr);
    }
    if (verbose)printf("\n");
    return TRUE;
}

/**
 * Cycle function for the EtherCat Master to receive and 
 * send data but wait for a cycle time. This function
 * orchestrates the communication cycle with EtherCAT slaves,
 * ensuring that data is sent and received within the specified
 * cycle time.
 */
void Master::cycle(){
    auto cycletime = std::chrono::microseconds(this->ctime);

    while (this->inOP) {
        auto start = std::chrono::high_resolution_clock::now();
        m.lock();
        ec_send_processdata();
        wkc = ec_receive_processdata(EC_TIMEOUTRET);
        m.unlock();
        auto end = std::chrono::high_resolution_clock::now();
        const auto delta = end - start;
        if (delta > cycletime){
            std::cout << "System too slow for cycle time " << cycletime.count() << "ms sending takes " << delta.count() << "ns" << std::endl;
        }
        else{
            std::this_thread::sleep_for(cycletime - delta);
        }
    }
}


/**
 * @brief Perform a preconfigured record task by providing the corresponding record number
 * 
 * @param slaveNr The slave number to perform the record task on
 * @param record The record number determining the record table entry that should be
 *               executed.
 *
 * @return int EXIT_SUCCESS or EXIT_FAILURE
 */
int Master::record_task(int slaveNr, int32_t record){
    if(verbose)printf("Starting Record task: %d on Slave: %d\n", record, slaveNr);

    if(readyState(slaveNr)){
        setMode(slaveNr, record_mode); // Set mode to record mode
        unsetControl(slaveNr);
        setRec(slaveNr, record); // Set record
        waitCycle();
        unsetBit(slaveNr, control_halt); // Bit 8 (Stop task) Must be 0 for record to be excuted
        
        // Bit 4 (Start task) To start the record
        setBit(slaveNr, control_4); 

        // Bit 12 (Acknowledge new command) Wait till new command is acknowledged
        while(!getBit(slaveNr, status_ack_start));
        // Bit 10 (Motion complete) Wait till motion is complete
        while(!getBit(slaveNr, status_mc)){ 
            //print current record number
            if(verbose)printf("Record task: %d is being excuted on Slave %d\r", getRec(slaveNr), slaveNr);
            unsetControl(slaveNr);
        }

        if(verbose)printf("\nRecord task %d completed\n", record);

        return EXIT_SUCCESS;
    }
    else{
        printf("Drive %d not enabled, movement not possible\n", slaveNr);
    }
    return EXIT_FAILURE;
}

/**
 * @brief Perform a velocity task with the given parameters using setup mode.
 * 
 * @param slaveNr Slave number
 * @param velocity Velocity setpoint in user units
 * @param duration Optional duration in seconds. 
 *                 A duration of 0 starts the task and returns immediately.
 * 
 * @return int EXIT_SUCCESS or EXIT_FAILURE
 */
int Master::velocity_task(int slaveNr, int32_t velocity, float duration){
    if(verbose)printf("Starting velocity task: %d on Slave: %d\n", velocity, slaveNr);
    if(readyState(slaveNr)){
        setMode(slaveNr, profile_velocity_mode);
        setTargetVelocity(slaveNr, velocity);
        waitCycle();
        unsetBit(slaveNr, control_halt);

        while(!getBit(slaveNr, status_mc)){
            if(getBit(slaveNr, status_rc)){
                printf("Slave %d: Error following velocity limit reached\n", slaveNr);
                return EXIT_FAILURE;
            }
            else if(getBit(slaveNr, status_mc)){
                printf("Slave %d: Velocity Reached\n", slaveNr);
                break;
            }
            if(verbose)printf("Velocity task: %d is being excuted on Slave %d\r", velocity, slaveNr);
        }
        if(verbose)printf("\n");
        unsetBit(slaveNr, control_halt);
        if (duration > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds((int) (duration * 1000)));
            stop_motion_task(slaveNr);
            if(verbose)printf("Velocity task %d completed\n", velocity);
        }
        return EXIT_SUCCESS;

    }
    else{
        printf("Drive %d not enabled, movement not possible\n", slaveNr);
        return EXIT_FAILURE;
    }
}

/**
 * Set the mode of the drive
 * 
 * @param slaveNr Slave number
 * @param mode Mode to set
 * 
 * @return mode Mode that is set
 */
int Master::setMode(int slaveNr, uint8_t mode){
    int timeout = 100;
    // Wait for mode to get active
    while (timeout-- && *(ec_slave[slaveNr].inputs + Mode_of_Operation_Display) != mode){
        unsetControl(slaveNr);
        //only change mode if not already in
        setByte(slaveNr, mode, Mode_of_Operation);
        waitCycle();
    }
    if (*(ec_slave[slaveNr].inputs + Mode_of_Operation_Display) == mode){
        unsetControl(slaveNr);
        if (verbose)printf("Arrived in mode %d\n", *(ec_slave[slaveNr].inputs + Mode_of_Operation_Display));
    }        
    else{
        printf("Failed to change into mode %d\n", *(ec_slave[slaveNr].inputs + Mode_of_Operation_Display));
    }
    return mode;
}

/**
 * Cia402 configuration for Festo CMMT-AS and CMMT-ST
 *
 * @param slaveNr Slave number
 * 
 * @return retval Number of successful writes
 */
int Master::mapCia402(uint16_t slaveNr){
    if (verbose)printf("Doing Cia402 configuration for Slave %d\n", slaveNr);
 
    int retval = 0;
    // Complete Access if true write multiple values in one go, false one value at a time
    bool ca = false; 

    // Floating value cycle time in seconds
    float32 ctimeInSeconds = (float32)this->ctime / 1000000;

    // PDO output
    uint8_t pdoOutputLength = 9; 
    uint32_t pdoOutput[9] = {0x60400010, 0x60600008, 0x607a0020,
                             0x60810020, 0x60ff0020, 0x60710010,
                             0x60b10020, 0x60b20010, 0x00000008};

    // PDO input
    uint8_t pdoInputLength = 9;
    uint32_t pdoInput[7] = {0x60410010, 0x60610008, 0x60640020,
                            0x606c0020, 0x60770010, 0x21940520,
                            0x00000008 };

    
    // Valus for confirming Jurgen Seymoutir explination
    uint16_t value16_1 = 0x1600; 
    uint16_t value16_2 = 0x1a00; 
    uint8_t value8 = 0x01;

    struct{
        uint16_t index;
        uint8_t subindex;
        int valueSize;
        void* value;
        bool ca;
    } configSteps[] = {
        {0x212E, 2, sizeof(ctimeInSeconds), &ctimeInSeconds, false}, // Everything cycle time related (should be checked)
        {0x1600, 0, sizeof(pdoOutputLength), &pdoOutputLength, false},  // Write amount of parameters for output
        {0x1600, 1, sizeof(pdoOutput), pdoOutput, true}, // Step set Output PDOs
        {0x1a00, 0, sizeof(pdoInputLength), &pdoInputLength, false}, // Write amount of parameters for input
        {0x1c12, 1, sizeof(value16_1), &value16_1, false}, // Step 1 confirming Jurgen Seymoutir explination
        {0x1c13, 1, sizeof(value16_2), &value16_2, false},// Step 2 
        {0x1c12, 0, sizeof(value8), &value8, false}, // Step 3
        {0x1c13, 0, sizeof(value8), &value8, false} // Step 4
    };

    int i = 0;
    // Loop through all steps and write them to the slave
    for (i = 0; i < sizeof(configSteps) / sizeof(configSteps[0]); i++){
        retval += ec_SDOwrite(slaveNr, configSteps[i].index, configSteps[i].subindex,
                                  configSteps[i].ca, configSteps[i].valueSize,
                                  configSteps[i].value, EC_TIMEOUTRXM);
    }

    if (verbose) puts("Done mapping drive");

    if (retval < i){
        printf("Check PDO mapping on slave %d\n", slaveNr);
    }

    return retval;
}

/**
 * Acknowledge faults on the slave
 * 
 * @param slaveNr Slave number
 */
void Master::acknowledge_faults(int slaveNr){
    if (verbose)printf("Acknowledge faults on slave %d\n", slaveNr);
    while (getError(slaveNr) != 0){
        setBit(slaveNr, control_fault_reset);
        waitCycle();
        unsetBit(slaveNr, control_fault_reset);
    }
}
/**
 * Configuring the slave before operational mode
 * 
 * @param slaveNr Slave number
 */
void Master::setPreOp(int slaveNr){
    printf("Configuring slave %d : %s id : 0x%x\n", slaveNr, ec_slave[slaveNr].name, ec_slave[slaveNr].eep_id);
    // Check if name is correct for all types of CMMT (not always reliable)
    if (strcmp(ec_slave[slaveNr].name, "CMMT-AS") == 0 || strcmp(ec_slave[slaveNr].name, "CMMT-ST") == 0 || strcmp(ec_slave[slaveNr].name, "FestoCMMT") == 0 ||
        ec_slave[slaveNr].eep_id == 0x7b5a25 || 0x7b1a95 == ec_slave[slaveNr].eep_id){ // Or based on ID
        mapCia402(slaveNr);
    }
    
}

/**
 * Startup function for the EtherCat Master
 * 
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int Master::startup(){
    if (verbose)printf("Starting Init\n");
    // Find and auto-config slaves
    if (ec_config_init(FALSE) > 0){
        for (int i = 1; i <= ec_slavecount; i++){
            setPreOp(i); // Mapping PDO data to drives
        }

        if(verbose)printf("%d slaves found and configured.\n", ec_slavecount);
        ec_config_map(&IOmap); // Make shadow coppy of online data

        ec_configdc();

        for (int i = 1; i <= ec_slavecount; i++){
            const auto state = ec_statecheck(i, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE);
            if ( verbose || state != EC_STATE_SAFE_OP) printf("Slave %d is trying to reach state 4:Safe-Op, current state = %d\n", i, state);
        }

        if (verbose)printf("Slaves mapped, state to SAFE_OP.\n");
        // Wait for all slaves to reach SAFE_OP state 
        ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE);
        printf("State %d = %d\n", EC_STATE_SAFE_OP,ec_readstate());
        if (verbose)printf("segments : %d : %d %d %d %d\n", ec_group[0].nsegments, ec_group[0].IOsegment[0], ec_group[0].IOsegment[1],
                                                            ec_group[0].IOsegment[2], ec_group[0].IOsegment[3]);
        if (verbose)printf("Request operational state for all slaves\n");
        int expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
        if (verbose)printf("Calculated workcounter %d\n", expectedWKC);
        ec_slave[0].state = EC_STATE_OPERATIONAL;
        // Request OP state for all slaves
        ec_writestate(0); // 0 == Master
        // Wait for all slaves to reach OP state 
        auto timeout = 5;

        do{
            // Send a least one valid process data to make outputs in slaves happy
            ec_send_processdata();
            ec_receive_processdata(EC_TIMEOUTRET);
            ec_statecheck(0, EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE); // Timeout was 50000
            if(ec_slave[0].state != EC_STATE_OPERATIONAL)printf("Tries left %d\n",timeout);
        } while (timeout-- && (ec_slave[0].state != EC_STATE_OPERATIONAL)); // Wait for operational or timeout
        
        if (ec_slave[0].state == EC_STATE_OPERATIONAL){
            if (verbose)printf("Operational state reached for all slaves.\n");
            inOP = TRUE;
            return EXIT_SUCCESS;
        }
        else{
            printf("Not all slaves reached operational state before timeout.\n");
            ec_readstate();
            for (int i = 1; i <= ec_slavecount; i++){
                if (ec_slave[i].state != EC_STATE_OPERATIONAL){
                    printf("Slave %d State=0x%2.2x StatusCode=0x%4.4x : %s\n",
                        i, ec_slave[i].state, ec_slave[i].ALstatuscode, ec_ALstatuscode2string(ec_slave[i].ALstatuscode));
                }
                else{
                    puts("This should not be possible, operational after timeout");
                }
            }
        }
    }
    else{
        printf("No slaves found!\n");
    }

    return EXIT_FAILURE;
}



// Torque mode not implemented
// =================================================================================================
// void Master::set8(int slaveNr, int8_t value, uint8_t byte) {
//     m.lock();
//     *(ec_slave[slaveNr].outputs + byte) = value;
//     m.unlock();
// }

// void Master::set32(int slaveNr, float32 value, uint8_t byte){
//     m.lock();
//     int floatAsInt = static_cast<int>(value);
//     *(ec_slave[slaveNr].outputs + byte + 3) = (floatAsInt >> 24) & 0xFF;
//     *(ec_slave[slaveNr].outputs + byte + 2) = (floatAsInt >> 16) & 0xFF;
//     *(ec_slave[slaveNr].outputs + byte + 1) = (floatAsInt >> 8) & 0xFF;
//     *(ec_slave[slaveNr].outputs + byte + 0) = floatAsInt & 0xFF;
//     m.unlock();
// }

// void Master::setTorSlo(int slaveNr, int8 torque, uint32 slope){
//     // Record number that is to be started is selecte via the Next record table index (0x216F.14)
//     write_sdo(slaveNr, 0x6071, 0x00, &torque, sizeof(torque));
//     waitCycle();
//     write_sdo(slaveNr, 0x6081, 0x00, &slope, sizeof(slope));
//     waitCycle();
// }

// float32 Master::getTor(int slaveNr){
//     float32 torque;
//     int size = sizeof(torque);
//     read_sdo(slaveNr, 0x6071, 0x00, &torque, &size);
//     return torque;
// }

// int Master::torque_task(int slaveNr, int8_t target, uint32 slope){
//     if (verbose)printf("Starting torque task with torque %d and slope %d on Slave: %d\n", target, slope, slaveNr);
//     if(readyState(slaveNr)){
//         setMode(slaveNr, profile_torque_mode);
//         unsetControl(slaveNr);
//         set8(slaveNr, target, Target_Torque);
//         waitCycle();
//         unsetBit(slaveNr, control_halt);
//         while(getBit(slaveNr, control_halt));
//         while(!getBit(slaveNr, status_mc)){
//             if(verbose)printf("Torque task: %d is being excuted on Slave %d\r", target, slaveNr);
//         }
//         printf("\n");
//         if(getBit(slaveNr, status_mc)){
//             setBit(slaveNr, control_halt);
//             printf("Slave %d: Torque Reached\n", slaveNr);
//             return EXIT_SUCCESS;
//         }
//         else{
//             printf("Slave %d: Error following torque limit reached\n", slaveNr);
//             return EXIT_FAILURE;
//         }
//     }
//     if(verbose)printf("Drive not enabled, movement not possible\n");
//     return EXIT_FAILURE;

// }