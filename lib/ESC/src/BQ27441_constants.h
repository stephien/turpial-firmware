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

/**
 * @brief According to: http://www.ti.com/lit/ds/symlink/bq27441-g1.pdf 
 * the fuel gauge uses a series of 2-byte **Standard Commands**
 * to enable system reading and writing of battery information.
 * 
 */
enum command {
    CONTROL = 0x00,        // Control                       NA      RW
    TEMP = 0x02,           // Temperature                   0.1°K   RW
    VOLTAGE = 0x04,        // Voltage                       mV      R
    FLAGS = 0x06,          // Flags                         NA      R
    NOM_CAPACITY = 0x08,   // NominalAvailableCapacity      -       R
    AVAIL_CAPACITY = 0x0A, // FullAvailableCapacity         -       R
    REM_CAPACITY = 0x0C,   // RemainingCapacity             -       R
    FULL_CAPACITY = 0x0E,  // FullChargeCapacity            -       R
    AVG_CURRENT = 0x10,    // AverageCurrent                -       R
    STDBY_CURRENT = 0x12,  // StandbyCurrent                -       R
    MAX_CURRENT = 0x14,    // MaxLoadCurrent                -       R
    AVG_POWER = 0x18,      // AveragePower                  -       R
    SOC = 0x1C,            // StateOfCharge                 -       R
    INT_TEMP = 0x1E,       // InternalTemperature           -       R
    SOH = 0x20,            // StateOfHealth                 -       R
    REM_CAP_UNFL = 0x28,   // RemainingCapacityUnfiltered   -       R
    REM_CAP_FIL = 0x2A,    // RemainingCapacityFiltered     mAh     R
    FULL_CAP_UNFL = 0x2C,  // FullChargeCapacityUnfiltered  mAh     R
    FULL_CAP_FIL = 0x2E,   // FullChargeCapacityFiltered    mAh     R
    SOC_UNFL = 0x30,       // StateOfChargeUnfiltered       %       R
    TRUE_REM_CAP = 0x6A    // TrueRemainingCapacity         mAh     R
};

} // namespace esc

#endif // BQ27441_CONSTANTS_H