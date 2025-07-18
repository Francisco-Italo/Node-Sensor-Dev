/*
 * fram.h
 *
 *  Created on: 7 de jan de 2025
 *      Author: italo
 */

#ifndef FRAM_FRAM_H_
#define FRAM_FRAM_H_

/**
 * Node sensor union
 */
union node_t
{
    struct
    {
        /**
         * DHT data type
         */
        struct
        {
            unsigned char hum_int,hum_decimals;
            unsigned char tmp_int,tmp_decimals;
            unsigned char checksum;
        }_dht_pck;
        /**
         * CCS811 data type
         */
        struct
        {
            unsigned int co2Lvl;
            unsigned int tvocLvl;
        }_ccs_pck;
        /**
         * MPU accelerometer data type
         */
        /*struct
        {
            unsigned int xAxis;
            unsigned int yAxis;
            unsigned int zAxis;
        }_mpu_pck;*/
        /**
         * HX711 data type
         */
        struct
        {
            unsigned long raw_weight;
            unsigned long tare;
        }_weight_pck;
    }sensor_data;

    unsigned char tx_block[18];
};
extern union node_t _pck;

#endif /* FRAM_FRAM_H_ */
