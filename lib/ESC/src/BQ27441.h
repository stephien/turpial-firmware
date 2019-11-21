/**
 * @file BQ27441.h
 * @author Locha Mesh project developers (locha.io)
 * @brief Single Series Cell Li-Ion Battery Fuel Gauge BQ27441 Ctrl. library
 * @version 0.1
 * @date 2019-11-21
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#ifndef BQ27441_H
#define BQ27441_H

namespace esc {

class BQ27441
{
private:
    /* data */
public:
    /**
     * @brief Construct a new BQ27441 object
     * 
     */
    BQ27441();
    esp_err_t begin();
    /**
     * @brief Destroy the BQ27441 object
     * 
     */
    ~BQ27441();
};

} // namespace esc

#endif // BQ27441_H