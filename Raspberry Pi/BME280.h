#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>

#include<wiringPi.h>
#include<wiringPiI2C.h>


class BME280 {
private:
        int t_fine;
        u_int16_t digT0 = 0;
        int16_t   digT[3] = {};

        u_int16_t digH0 = 0, digH2 = 0;
        int16_t   digH[6] = {};

        u_int16_t digP0 = 0;
        int16_t   digP[9] = {};

        int devfd;
public:

        BME280(){
                devfd = 0;
                init_bme280(&devfd);
                get_calib_param();

        }

        void init_bme280(int *devfd);
        bool begin();

        float   measure_Temperature();
        float   measure_humidity();
        float   measure_pressure();
        void get_calib_param();

};
