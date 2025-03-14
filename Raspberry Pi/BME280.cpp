#include "BME280.h"
#define DEVID (0x76)

void BME280::init_bme280(int *devfd)
{
    if((*devfd=wiringPiI2CSetup(DEVID)) < 0) {
        printf("wiringPiI2CSetup error(%d):%s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
//   printf("  devfd = %d\n", *devfd);

    /* Humidity oversampling x1(osrs_h=0x1) */
    if(wiringPiI2CWriteReg8(*devfd, 0xf2, 0x01) < 0) {
        printf("write error register 0xf2(%d):%s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* Temparature oversampling x1(osrs_t=0x1),
     * Pressure oversampling x1   (osrs_p=0x1),
     * Normal mode                (mode=0x3)
     */
    if(wiringPiI2CWriteReg8(*devfd, 0xf4, 0x27) < 0) {
        printf("write error register 0xf4(%d):%s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* Standby 1000ms(t_sb=0x10), Filter off(filter=0x4), spi3w_en=0x0 */
    if(wiringPiI2CWriteReg8(*devfd, 0xf5, 0xa0) < 0) {
        printf("write error register 0xf4(%d):%s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void BME280::get_calib_param()
{
    digT0 = wiringPiI2CReadReg16(devfd, 0x88);
    digT[1] = wiringPiI2CReadReg16(devfd, 0x8a);
    digT[2] = wiringPiI2CReadReg16(devfd, 0x8c);
//   printf("digT0=0x%04x, digT1=0x%04x, digT2=0x%04x\n", digT[0], digT[1], digT[2]);

    digP0 = wiringPiI2CReadReg16(devfd, 0x8e);
    digP[1] = wiringPiI2CReadReg16(devfd, 0x90);
    digP[2] = wiringPiI2CReadReg16(devfd, 0x92);
    digP[3] = wiringPiI2CReadReg16(devfd, 0x94);
    digP[4] = wiringPiI2CReadReg16(devfd, 0x96);
    digP[5] = wiringPiI2CReadReg16(devfd, 0x98);
    digP[6] = wiringPiI2CReadReg16(devfd, 0x9a);
    digP[7] = wiringPiI2CReadReg16(devfd, 0x9c);
    digP[8] = wiringPiI2CReadReg16(devfd, 0x9e);
  //  printf("digP0=0x%04x, digP1=0x%04x, digP2=0x%04x digP3=0x%04x\n", digP0, digP[1], digP[2], digP[3]);
  // printf("digP4=0x%04x, digP5=0x%04x, digP6=0x%04x digP7=0x%04x\n", digP[4], digP[5], digP[6], digP[7]);
  //  printf("digP8=0x%04x\n", digP[8]);

    digH0 = wiringPiI2CReadReg8(devfd, 0xa1);
    digH[1] = wiringPiI2CReadReg16(devfd, 0xe1);
    digH2 = wiringPiI2CReadReg8(devfd, 0xe3);
    digH[3] = (wiringPiI2CReadReg8(devfd, 0xe4) << 4) | (wiringPiI2CReadReg8(devfd, 0xe5) & 0x0f);
    digH[4] = (wiringPiI2CReadReg8(devfd, 0xe6) << 4) | ((wiringPiI2CReadReg8(devfd, 0xe5) >> 4) & 0x0f);
    digH[5] = wiringPiI2CReadReg8(devfd, 0xe7);
//    printf("digH0=0x%04x, digH1=0x%04x, digH2=0x%04x, digH3=0x%04x, digH4=0x%04x, digH5=0x%04x\n",
//        digH[0], digH[1], digH[2], digH[3], digH[4], digH[5]);
}

float BME280::measure_Temperature()
{
    unsigned int raw_temp = 0;
    unsigned char raw_data[3] = {};

    raw_data[0] = wiringPiI2CReadReg8(devfd, 0xfa);
    raw_data[1] = wiringPiI2CReadReg8(devfd, 0xfb);
    raw_data[2] = wiringPiI2CReadReg8(devfd, 0xfc);

    raw_temp = (raw_data[0] << 12) | (raw_data[1] << 4) | (raw_data[2] >> 4);
//    printf("raw temperature = %d(0x%x)\n", raw_temp, raw_temp);

    int temp_data = 0;
    temp_data =
        (((((raw_temp >> 3) - (digT0 << 1))) * digT[1]) >> 11) +
        ((((((raw_temp >> 4) - digT0) * ((raw_temp >> 4) - digT0)) >> 12) * digT[2]) >> 14);
//    printf("temp = %d(0x%x)\n", temp_data, temp_data);
    t_fine = temp_data;

    return( (float)((temp_data * 5 + 128) >> 8) / 100.0f );
}

float BME280::measure_humidity()
{
    unsigned int raw_humid = 0;
    int v_x1 = 0;

    raw_humid = (wiringPiI2CReadReg8(devfd, 0xfd) << 8) | wiringPiI2CReadReg8(devfd, 0xfe);

    v_x1 = t_fine - 76800;
    v_x1 =  (((((raw_humid << 14) - (((int32_t)digH[3]) << 20) - (((int32_t)digH[4]) * v_x1)) +
               ((int32_t)16384)) >> 15) * (((((((v_x1 * (int32_t)digH[5]) >> 10) *
                                            (((v_x1 * ((int32_t)digH2)) >> 11) + 32768)) >> 10) + 2097152) *
                                            (int32_t)digH[1] + 8192) >> 14));
    v_x1 = (v_x1 - (((((v_x1 >> 15) * (v_x1 >> 15)) >> 7) * (int32_t)digH0) >> 4));
    v_x1 = (v_x1 < 0 ? 0 : v_x1);
    v_x1 = (v_x1 > 419430400 ? 419430400 : v_x1);

    return( ((float)(v_x1 >> 12)) / 1024.0f );
}

float BME280::measure_pressure()
{
    unsigned int raw_pressure = 0;
    int32_t var1, var2;
    unsigned int press;

    raw_pressure = (wiringPiI2CReadReg8(devfd, 0xf7) << 12)
                    | (wiringPiI2CReadReg8(devfd, 0xf8) << 4)
                    | (wiringPiI2CReadReg8(devfd, 0xf9) >> 4);
//    printf("raw_pressure = %04x\n", raw_pressure);

    var1 = (t_fine >> 1) - 64000;
    var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * digP[5];
    var2 = var2 + ((var1 * digP[4]) << 1);
    var2 = (var2 >> 2) + (digP[3] << 16);
    var1 = (((digP[2] * (((var1 >> 2)*(var1 >> 2)) >> 13)) >> 3) + ((digP[1] * var1) >> 1)) >> 18;
    var1 = ((32768 + var1) * digP0) >> 15;
    if (var1 == 0) {
        return 0;
    }
    press = (((1048576 - raw_pressure) - (var2 >> 12))) * 3125;
    if(press < 0x80000000) {
        press = (press << 1) / var1;
    } else {
        press = (press / var1) * 2;
    }

    var1 = ((int32_t)digP[8] * ((int32_t)(((press >> 3) * (press >> 3)) >> 13))) >> 12;
    var2 = (((int32_t)(press >> 2)) * (int32_t)digP[7]) >> 13;
    press = (press + ((var1 + var2 + digP[6]) >> 4));

    return( ((float)press) / 100.0f );
}