////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
Begin license text.

Copyright 2023- MicomScript Project

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the ÅgSoftwareÅh), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED ÅgAS ISÅh, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

End license text.
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * observer.cpp
 */

//#include <Arduino.h>
#include <wiringPi.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <sched.h>
#include "common.h"
#include "observer.h"
#include <pthread.h>

//TaskHandle_t  _th1;   // ButtonObserver
//TaskHandle_t  _th2;   // TimeObserver
//TaskHandle_t  _th3;   // UDPObserver

pthread_mutex_t mtx;


void *buttonObserver(void *args);
void *timeObserver(void *args);
void udpObserver(void *args);



void init_observer(){
        pthread_t       _th1;
        pthread_t       _th2;
        pthread_t       _th3;



        pthread_create(&_th1,NULL,buttonObserver, NULL);
        pthread_create(&_th2,NULL,timeObserver, NULL);
//      pthread_create(&_th3,NULL,udpObserver, NULL);
        printf("Ç¢ÇØÇΩ\n");





/*
        xTaskCreateUniversal(buttonObserver, "ButtonObserver", 4096, NULL, 3, &_th1, 1);
        xTaskCreateUniversal(timeObserver, "TimeObserver", 4096, NULL, 5, &_th2, 1);
        xTaskCreateUniversal(udpObserver, "UDPObserver", 4096, NULL, 3, &_th3, 0);
        xTaskCreateUniversal(blinkObserver, "BlinkObserver", 4096, NULL, 4, &_th, 1);
*/

}
