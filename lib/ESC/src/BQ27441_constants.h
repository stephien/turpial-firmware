/**
 * @file BQ27441_constants.h
 * @author Locha Mesh project developers (locha.io)
 * @brief Fuel Gauge BQ27441 Ctrl. library constants, registers, and bit positions.
 * @version 0.1
 * @date 2019-11-21
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#ifndef BQ27441_CONSTANTS_H
#define BQ27441_CONSTANTS_H

#include <cstdint>

namespace esc {

// Default I2C address for the BQ27441
const std::uint8_t I2C_ADDRESS = 0x55;

// Secret code to unseal the BQ27441
const std::uint16_t UNSEAL_KEY = 0x8000;

// Device type id
const std::uint16_t DEVICE_TYPE_ID = 0x0421;

// Standard Commands
// The fuel gauge uses a series of 2-byte standard commands to enable system reading and writing of battery information. 

enum command {
    CONTROL,
    TEMP,
    VOLTAGE,
    FLAGS,
    NOM_CAPACITY,
    AVAIL_CAPACITY,
    REM_CAPACITY,
    FULL_CAPACITY,
    AVG_CURRENT,
    STDBY_CURRENT,
    MAX_CURRENT,
    AVG_POWER,
    //
    // WORK IN PROGRESS

};

} // namespace

#endif // BQ27441_CONSTANTS_H