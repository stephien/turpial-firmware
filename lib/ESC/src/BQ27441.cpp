/**
 * @file BQ27441.cpp
 * @author Locha Mesh project developers (locha.io)
 * @brief Single Series Cell Li-Ion Battery Fuel Gauge BQ27441 Ctrl. library
 * @version 0.1
 * @date 2019-11-21
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#include <driver/gpio.h>
#include <driver/i2c.h>
#include <esp_log.h>

#include "BQ27441.h"
#include "BQ27441_constants.h"

namespace esc {

BQ27441::BQ27441()
{
    /* ~ */
}

BQ27441::~BQ27441()
{
    /* ~ */
}


} // namespace esc
