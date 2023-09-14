// master.cpp : Source file for your target.

#include "master.h"

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

uint16_t Master::unsetControl(int slaveNr){
    uint8 byte0=0, byte1=0;

    //unset all control bits from previous mode
    byte0 = unsetBit(slaveNr, control_4, Controlword);
    byte0 = unsetBit(slaveNr, control_5, Controlword);
    byte0 = unsetBit(slaveNr, control_6, Controlword);
    byte1 = unsetBit(slaveNr, control_9, Controlword);

    const uint16 retVal = (byte1 << 8) + byte0;

    return retVal;
}

bool Master::getBit(int slaveNr, uint8_t bit, uint8_t byte){
    while (bit > 7){
        bit -= 8;
        byte += 1;
        if (byte >= ec_slave[slaveNr].Ibytes) byte = 0; // prevent out of bounds
    }

    const auto retVal = *(ec_slave[slaveNr].inputs + byte) & (1 << bit);
    return retVal;
}

void Master::setByte(int slaveNr, uint8_t value, uint8_t byte){
    m.lock();
    *(ec_slave[slaveNr].outputs + byte) = value;
    m.unlock();
}

int Master::getError(int slaveNr){
    auto retval = 0;
    if(getBit(slaveNr, status_fault)) retval--;
    if (getBit(slaveNr, status_warning)) retval--;
    return retval;
}

int16_t Master::get16(int slaveNr, uint8_t byte){
    auto retVal = 0;
    
    retVal += (*(ec_slave[slaveNr].inputs + byte + 1) << 8);
    retVal += (*(ec_slave[slaveNr].inputs + byte + 0));

    return retVal;
}

int32_t Master::getPos(int slaveNr){
    auto retVal = 0;
    const int positionActualValueAddress = 3;
       
    retVal += (*(ec_slave[slaveNr].inputs + positionActualValueAddress + 3) << 24);
    retVal += (*(ec_slave[slaveNr].inputs + positionActualValueAddress + 2) << 16);
    retVal += (*(ec_slave[slaveNr].inputs + positionActualValueAddress + 1) << 8);
    retVal += (*(ec_slave[slaveNr].inputs + positionActualValueAddress + 0));

    return retVal;
}

bool Master::connected(){
    return this->inOP;
}

void Master::set16(int slaveNr, int16_t value, uint8_t byte){
    m.lock();
    *(ec_slave[slaveNr].outputs + byte + 1) = (value >> 8) & 0xFF;
    *(ec_slave[slaveNr].outputs + byte + 0) = value & 0xFF;
    m.unlock();
}

void Master::setPos(int slaveNr, int32_t target, uint8_t byte ){
    m.lock();
    *(ec_slave[slaveNr].outputs + byte + 3) = (target >> 24) & 0xFF;
    *(ec_slave[slaveNr].outputs + byte + 2) = (target >> 16) & 0xFF;
    *(ec_slave[slaveNr].outputs + byte + 1) = (target >>  8) & 0xFF;
    *(ec_slave[slaveNr].outputs + byte + 0) = target & 0xFF;
    m.unlock();
}

void Master::setProfileVelocity(int slaveNr, uint32_t velocity, uint8_t byte){
    m.lock();
    *(ec_slave[slaveNr].outputs + byte + 3) = (velocity >> 24) & 0xFF;
    *(ec_slave[slaveNr].outputs + byte + 2) = (velocity >> 16) & 0xFF;
    *(ec_slave[slaveNr].outputs + byte + 1) = (velocity >> 8) & 0xFF;
    *(ec_slave[slaveNr].outputs + byte + 0) = velocity & 0xFF;
    m.unlock();
}

int Master::reset(int slaveNr){
    unsigned int timeout = 1000;
    if(verbose)printf("Resetting slave nr : %d\n",slaveNr);
    if (this->inOP){
        m.lock();
        memset(ec_slave[slaveNr].outputs, 0, ec_slave[slaveNr].Obytes); // start empty to prevent retriggering error
        m.unlock();
        if (verbose)printf("Wait for empty frame slave nr : %d\n", slaveNr);
        waitCycle(); // wait for empty frame to be send

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

void Master::waitCycle(){
    std::this_thread::sleep_for(std::chrono::microseconds(this->ctime));
}

int Master::enable(int slaveNr){
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

int Master::disable(int slaveNr){
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

int Master::home(int slaveNr, bool always){   
    if (readyState(slaveNr)){
        setMode(slaveNr, homing_mode);
        if (getBit(slaveNr, status_ref) && !always){
            printf("Slave %d already homed\n", slaveNr);
        }
        else{
            printf("Slave %d starting homing\n", slaveNr);
            unsetControl(slaveNr);
            setBit(slaveNr, control_4);
            while (!getBit(slaveNr, status_ref_reached)); // check for rehoming
            unsetBit(slaveNr, control_4);
        }
        return 0;
    }
    else printf("Homing not possible slave %d not enabled\n ", slaveNr);

    return -1;
}

void Master::jogPos(int slaveNr){
    if (verbose)puts("Begin jog in positive direction");
    if (readyState(slaveNr)){
        uint8_t controlbyte;
        controlbyte = setMode(slaveNr, jog_mode);
        unsetControl(slaveNr);
        while (!getBit(slaveNr, status_mc));
        controlbyte = setBit(slaveNr, control_4);
    }
    else printf("Jogging not possible slave %d not enabled\n ", slaveNr);
}

void Master::jogNeg(int slaveNr){
    puts("Begin jog in negative direction");
    if (readyState(slaveNr)){
        uint8_t controlbyte;
        controlbyte = setMode(slaveNr, jog_mode);
        unsetControl(slaveNr);
        while (!getBit(slaveNr, status_mc));
        controlbyte = setBit(slaveNr, control_5);
    }
    else printf("Jogging not possible slave %d not enabled\n ", slaveNr);
}

void Master::jogStop(int slaveNr){
    if (verbose)printf("Stopping Jog movement\n");
    if (readyState(slaveNr)){
        unsetControl(slaveNr);
        while (!getBit(slaveNr, status_mc)) waitCycle();
    }
}

int Master::movePosition(int slaveNr, int32_t target, bool relative){
    /*Precondition for positioning mode
        The following conditions must be fulfilled for positioning mode :
    – Modes of operation display(0x6061) = 1
    – Statusword(0x6041) = 1X0X X11X X011 0111b
        Control and monitoring
        Object 0x6040 : Controlword
        The object controls the following functions of positioning mode :
    – Bit 4 : start motion command(New set - point)
    – Bit 5 : accept change immediately(Change set immediately)
        Motion Control
        Festo — CMMT - AS - SW — 2019 - 08c 303
    – Bit 6: positioning type(absolute / relative)
    – Bit 8 : stop motion command(Halt)*/
    char mode[9] = "Absolute";
    if (relative){
        strncpy_s(mode, "Relative", 9);
    }
    if (verbose)printf("Starting %s movement to position %d of slave % d\n",mode , target, slaveNr);
    if (readyState(slaveNr)){
        setMode(slaveNr, profile_position_mode);
        unsetControl(slaveNr);
        if (relative)setBit(slaveNr, control_6);
        setPos(slaveNr, target);
        waitCycle();
        unsetBit(slaveNr, control_halt);
        setBit(slaveNr, control_4);
        while (!getBit(slaveNr, status_ack_start)); // wait for ack to prevent response to previous mc 
        while (!getBit(slaveNr, status_mc)){
            if (verbose)printf("Move slave %d %s : %d %d\r", slaveNr, mode, target, getPos(slaveNr));
            unsetControl(slaveNr);
        }
        if (verbose)printf(" completed\n");
        return EXIT_SUCCESS;
    }
    else{
        printf("Drive %d not enabled, movement not possible\n", slaveNr);
    }
    return EXIT_FAILURE;
}

int Master::movePosition(int slaveNr, int32_t target, int32_t velocity, bool relative){
    setProfileVelocity(slaveNr, velocity);
    if (velocity < 0) {
        printf("ERROR : Slave %d Velocity should be a positive number in positioning mode\n", slaveNr);
        return -1;
    }
    else return movePosition(slaveNr, target, relative);
}

int Master::movePosition(int slaveNr, int32_t target, uint32_t velocity, bool relative){
    setProfileVelocity(slaveNr, velocity);
    return movePosition(slaveNr, target, relative);
}

int Master::movePosition(int slaveNr, int32_t target, uint32_t velocity, uint32_t acceleration, uint32_t deceleration, bool relative){
    auto retval = 0;
    // writing acceleration
    retval += ec_SDOwrite(slaveNr, 0x6083, 0, false, sizeof(acceleration), &acceleration, EC_TIMEOUTRXM);
    
    // writing deceleration
    retval += ec_SDOwrite(slaveNr, 0x6084, 0, false, sizeof(deceleration), &deceleration, EC_TIMEOUTRXM);

    if (retval == 2){
        retval = movePosition(slaveNr, target, velocity, relative);
        return retval;
    }
    else{
        printf("Writing of acceleration or deceleration failed on slave %d\n", slaveNr);
    }
    
    return EXIT_FAILURE;
}

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
 * @return retval Number of successful writes
 */
int Master::mapCia402(uint16_t slaveNr){
    if (verbose)printf("Doing Cia402 configuration for Slave %d\n", slaveNr);
 
    int retval = 0;
    // Complete Access if true write multiple values in one go, false one value at a time
    bool ca = false; 

    // floating value cycle time in seconds
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
        {0x1600, 1, sizeof(pdoOutput), pdoOutput, true}, // Step set Output PDOs
        {0x1600, 0, sizeof(pdoOutputLength), &pdoOutputLength, false},  // Write amount of parameters for output
        {0x1a00, 1, sizeof(pdoInput), pdoInput, true}, // Step set Input PDOs 
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

void Master::setPreOp(int slaveNr){
    printf("Configuring slave %d : %s id : 0x%x\n", slaveNr, ec_slave[slaveNr].name, ec_slave[slaveNr].eep_id);
    // check if name is correct for all types of CMMT (not always reliable)
    if (strcmp(ec_slave[slaveNr].name, "CMMT-AS") == 0 || strcmp(ec_slave[slaveNr].name, "CMMT-ST") == 0 || strcmp(ec_slave[slaveNr].name, "FestoCMMT") == 0 ||
        ec_slave[slaveNr].eep_id == 0x7b5a25 || 0x7b1a95 == ec_slave[slaveNr].eep_id){ // Or based on ID{
        mapCia402(slaveNr);
    }
    
}

int Master::startup(){
    if (verbose)printf("Starting Init\n");
    /* find and auto-config slaves */
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
        /* wait for all slaves to reach SAFE_OP state */
        ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE);
        printf("State %d = %d\n", EC_STATE_SAFE_OP,ec_readstate());
        if (verbose)printf("segments : %d : %d %d %d %d\n", ec_group[0].nsegments, ec_group[0].IOsegment[0], ec_group[0].IOsegment[1], ec_group[0].IOsegment[2], ec_group[0].IOsegment[3]);

        if (verbose)printf("Request operational state for all slaves\n");
        int expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
        if (verbose)printf("Calculated workcounter %d\n", expectedWKC);
        ec_slave[0].state = EC_STATE_OPERATIONAL;
        /* request OP state for all slaves */
        ec_writestate(0); // 0 == Master
        /* wait for all slaves to reach OP state */
        auto timeout = 5;
        do{
            /* send a least one valid process data to make outputs in slaves happy*/
            ec_send_processdata();
            ec_receive_processdata(EC_TIMEOUTRET);
            ec_statecheck(0, EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE); // timeout was 50000
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