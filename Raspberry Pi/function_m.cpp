/*
 Rpsberry Pi function_m.cpp
 */


#include <iostream>
#include <math.h>
#include <string.h>
#include <wiringPi.h>
#include <pthread.h>            //12/11
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include <wiringPi.h>
#include<wiringPiI2C.h>

#include "common.h"
#include "lock.h"
#include "expression.h"
#include "logic.h"
#include "varmap.h"
#include "sentence.h"
#include "function.h"
#include "function_m.h"


//ファイル-----------------------------------------------------
//----------------------------------------------------------

FileObject::FileObject(char *name, char *mode){
        _state = 0;
        _fname = new char[strlen(name) + 1];
        strcpy(_fname, name);
        _mode = *mode;
        _fp = fopen(name, mode);
        if (_fp != NULL)
                _state = 1;
}

FileObject::~FileObject(){
#ifdef DEBUG
        printf("delete File(%s, %c)\n", _fname, _mode);
#endif
}

char *FileObject::getText(){
        return _fname;
}

char *FileObject::getType(){
        return (char *)"File";
}

Value *FileObject::evaluate(){
        return new Integer(_state);
}

void FileObject::print(){
        printf("File(%s, %c)\n", _fname, _mode);
}

Value *FileObject::methodCall(char *name, List *params){
        char buff[512];

#ifdef DEBUG
        printf("FileObject::methodCall(%s)\n", name);
#endif
        if (strcmp(name, "eof") == 0){
                if (feof(_fp)){
                        return new Bool(true);
                }
                return new Bool(false);
        } else if (strcmp(name, "read") == 0){
                if (fgets(buff, 512, _fp) == NULL){
                        return NULL;
                }
                return new Text(buff);
        } else if (strcmp(name, "write") == 0){
                int n = params->size();
                if (n == 0){
                        printf("Parameter error.");
                        return NULL;
                }
                char *s = params->getElement(0)->getText();
                int state = fputs(s, _fp);
                return new Integer(state);
        } else if (strcmp(name, "close") == 0){
                fclose(_fp);
                _state = 0;
                return NULL;
        } else if (strcmp(name, "rewind") == 0){
                if (_state == 1){
                        rewind(_fp);
                }
                return NULL;
        } else {
                printf("Undefined function: %s\n", name);
        }
        return NULL;
}

//----------------------------------------------------------

Value *FileOpen::evaluate(){
        int n = _list->size();
        if (n != 2){
                printf("Invalid number of parameters\n");
                throw ErrorException();
        }
        char *name = _list->getElement(0)->getText();
        char *mode = _list->getElement(1)->getText();
        return new FileObject(name, mode);
}

//--------------------
//リスト-------------------------------------------------
void ObjList::add(Value *obj){
        ObjNode *node = search(obj);
        if (node == NULL){
                _top = new ObjNode(obj, _top);
        }
}
void ObjList::remove(Value *obj){
        ObjNode *node, *next, *prev;
        prev = NULL;
        for(node = _top; node ; node = next){
                next = node->_next;
                if (node->_obj == obj)
                        break;
                prev = node;
        }
        if (node){
                if (node == _top){      // or if (prev == NULL)
                        _top = _top->_next;
                } else {
                        prev->_next = node->_next;
                }
                delete node;
        }
}
ObjNode *ObjList::search(Value *obj){
        ObjNode *node;
        for(node = _top; node ; node = node->_next){
                if (node->_obj == obj)
                        return node;
        }
        return NULL;
}


void ObjList::clearNodes(){
        ObjNode *node, *next;
        for(node = _top; node ; node = next){
                next = node->_next;
                delete node;
        }
}
//Observer--------------------------------------------------------------
ButtonList              _buttonList;
TimerList               _timerlist;
SimpleLocker    _timerlist_locker;
SimpleLocker    _buttonlist_locker;
pthread_mutex_t mutex;



char _timerName[] = "Timer";

void buttonObserver(void *args){
        while(1){
                if (_buttonlist_locker.lock(400)){
                        _buttonList.scan();
                        _buttonlist_locker.unlock(400);
                }
                delay(10);
        }
}

void timeObserver(void *args){
        while(1){
                if (_timerlist_locker.lock(300)){
                        _timerlist.dispatch();
                        _timerlist_locker.unlock(300);
                }
                delay(1);
        }
}


//GPIO関数----------------------------------------
Value *PinMode::evaluate(){
        int pin,io;
        pin = _list->getElement(0)->getInt();
        io = _list->getElement(1)->getInt();
        if(io==0){
                pinMode(pin,OUTPUT);
                printf("OUTPUT\n");
        }else if(io==1){
                pinMode(pin,INPUT);
                printf("INPUT\n");
        }else if(io==2){                        //2024/10/29
                pinMode(pin,INPUT);
                pullUpDnControl(pin, PUD_UP);
                printf("INPUT_PULLUP\n");
        }else{
                printf("Invalid Parameter(pinMode)\n");
        }

        return NULL;
}

Value *DigitalWrite::evaluate(){
        int pin,HL;
        pin = _list-> getElement(0)->getInt();
        HL = _list-> getElement(1)->getInt();
        if(HL == 0){
                digitalWrite(pin,LOW);
        }else if(HL == 1){
                digitalWrite(pin,HIGH);
        }else{
                printf("Invalid Parameter(digitalWrite)...HIGH or LOW\n");
        }
        return NULL;
}

Value *AnalogRead::evaluate(){
        int pin;
        pin = _list-> getElement(0) ->getInt();
        int analogValue = analogRead(pin);
        return new Integer(analogValue);
}

Value *DigitalRead::evaluate(){
        int pin;
        pin = _list-> getElement(0) ->getInt();
        int digitalValue = digitalRead(pin);
        return new Integer(digitalValue);
};
//---------------------------------------------------------------------

uint8_t _letters[][8] = {                                   //tatch
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Space
//{ 0x00, 0x02, 0x06, 0x06, 0x04, 0x04, 0x00, 0x04 }, // !
{ 0x00, 0x04, 0x0e, 0x0e, 0x04, 0x04, 0x00, 0x04 }, // !
{ 0x00, 0x0a, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00 }, // "
{ 0x00, 0x0a, 0x1f, 0x0a, 0x0a, 0x1f, 0x0a, 0x0a }, // #
{ 0x00, 0x0e, 0x15, 0x14, 0x0e, 0x05, 0x15, 0x0e }, // $
{ 0x00, 0x00, 0x09, 0x02, 0x04, 0x08, 0x12, 0x00 }, // %
{ 0x00, 0x0c, 0x12, 0x12, 0x0c, 0x0d, 0x12, 0x1d }, // &
{ 0x00, 0x08, 0x08, 0x04, 0x00, 0x00, 0x00, 0x00 }, // '
{ 0x00, 0x0c, 0x10, 0x10, 0x10, 0x10, 0x10, 0x0c }, // (
{ 0x00, 0x18, 0x04, 0x04, 0x04, 0x04, 0x04, 0x18 }, // )
{ 0x00, 0x04, 0x15, 0x0e, 0x04, 0x0e, 0x15, 0x04 }, // *
{ 0x00, 0x04, 0x04, 0x04, 0x1f, 0x04, 0x04, 0x04 }, // +
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x06, 0x08 }, // ,
{ 0x00, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00 }, // -
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x0c }, // .
{ 0x00, 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x00 }, // /
{ 0x00, 0x0e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e }, // 0
{ 0x00, 0x04, 0x0c, 0x04, 0x04, 0x04, 0x04, 0x0e }, // 1
{ 0x00, 0x0e, 0x11, 0x02, 0x04, 0x08, 0x11, 0x1f }, // 2
{ 0x00, 0x0e, 0x11, 0x01, 0x06, 0x01, 0x11, 0x0e }, // 3
{ 0x00, 0x14, 0x14, 0x14, 0x14, 0x1f, 0x04, 0x04 }, // 4
{ 0x00, 0x1f, 0x10, 0x10, 0x1e, 0x01, 0x01, 0x1e }, // 5
{ 0x00, 0x0e, 0x11, 0x10, 0x1e, 0x11, 0x11, 0x0e }, // 6
{ 0x00, 0x1f, 0x11, 0x11, 0x02, 0x04, 0x04, 0x04 }, // 7
{ 0x00, 0x0e, 0x11, 0x11, 0x0e, 0x11, 0x11, 0x0e }, // 8
{ 0x00, 0x0e, 0x11, 0x11, 0x0f, 0x01, 0x01, 0x0e }, // 9
{ 0x00, 0x00, 0x04, 0x04, 0x00, 0x04, 0x04, 0x00 }, // :
{ 0x00, 0x00, 0x04, 0x04, 0x00, 0x04, 0x04, 0x08 }, // ;
{ 0x00, 0x01, 0x02, 0x04, 0x08, 0x04, 0x02, 0x01 }, // <
{ 0x00, 0x00, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x00 }, // =
{ 0x00, 0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08 }, // >
{ 0x00, 0x0e, 0x11, 0x13, 0x04, 0x04, 0x00, 0x04 }, // ?
{ 0x00, 0x0e, 0x13, 0x15, 0x15, 0x16, 0x11, 0x0e }, // @
{ 0x00, 0x0e, 0x11, 0x11, 0x11, 0x1f, 0x11, 0x11 }, // A
{ 0x00, 0x1e, 0x11, 0x11, 0x1e, 0x11, 0x11, 0x1e }, // B
{ 0x00, 0x0e, 0x11, 0x11, 0x10, 0x10, 0x11, 0x0e }, // C
{ 0x00, 0x1e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1e }, // D
{ 0x00, 0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x1f }, // E
{ 0x00, 0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x10 }, // F
{ 0x00, 0x0e, 0x11, 0x11, 0x10, 0x17, 0x11, 0x0f }, // G
{ 0x00, 0x11, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11 }, // H
{ 0x00, 0x0e, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0e }, // I
{ 0x00, 0x03, 0x01, 0x01, 0x01, 0x11, 0x11, 0x0e }, // J
{ 0x00, 0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11 }, // K
{ 0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x11, 0x1f }, // L
{ 0x00, 0x11, 0x1b, 0x1b, 0x15, 0x15, 0x15, 0x15 }, // M
{ 0x00, 0x11, 0x19, 0x15, 0x15, 0x13, 0x11, 0x11 }, // N
{ 0x00, 0x0e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e }, // O
{ 0x00, 0x1e, 0x11, 0x11, 0x1e, 0x10, 0x10, 0x10 }, // P
{ 0x00, 0x0e, 0x11, 0x11, 0x11, 0x15, 0x13, 0x0d }, // Q
{ 0x00, 0x1c, 0x12, 0x12, 0x1c, 0x14, 0x12, 0x11 }, // R
{ 0x00, 0x0e, 0x11, 0x10, 0x0e, 0x01, 0x11, 0x0e }, // S
{ 0x00, 0x1f, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 }, // T
{ 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e }, // U
{ 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0a, 0x04 }, // V
{ 0x00, 0x11, 0x15, 0x15, 0x15, 0x15, 0x15, 0x0a }, // W
{ 0x00, 0x11, 0x11, 0x0a, 0x04, 0x0a, 0x11, 0x11 }, // X
{ 0x00, 0x11, 0x11, 0x11, 0x0a, 0x04, 0x04, 0x04 }, // Y
{ 0x00, 0x0f, 0x11, 0x02, 0x04, 0x08, 0x11, 0x1f }, // Z
{ 0x00, 0x0e, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0e }, // [
{ 0x00, 0x00, 0x10, 0x08, 0x04, 0x02, 0x01, 0x00 }, //
{ 0x00, 0x0e, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0e }, // ]
{ 0x00, 0x04, 0x0a, 0x11, 0x00, 0x00, 0x00, 0x00 }, // ^
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f }, // _
{ 0x00, 0x0c, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00 }, // `
{ 0x00, 0x02, 0x04, 0x04, 0x08, 0x04, 0x04, 0x02 }, // {
{ 0x00, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 }, // |
{ 0x00, 0x08, 0x04, 0x04, 0x02, 0x04, 0x04, 0x08 }, // }
{ 0x00, 0x00, 0x0a, 0x15, 0x11, 0x0a, 0x04, 0x00 }, // ~(Heart)
{ 0x00, 0x00, 0x04, 0x0a, 0x11, 0x0a, 0x04, 0x00 }  // Diamond
};





//PWM制御-------------------------------------------------------------------------------------
Value *PWM::methodCall(char *name, List *params){
#ifdef DEBUG
        printf("PWM::methodCall(%s)\n", name);
#endif
        int range = 0;
        double clock ;
        double count_f ;
        double  period ;
        double h_period;
        if(strcmp(name, "pulse") == 0){
                uint32_t        freq = params->getElement(0)->getInt();
                double          state = params->getElement(1)->getFloat();
//              printf("周波数 = %d[Hz]\n",freq);
//              printf("パルス幅 = %f[ms]\n",state);
                clock  = 19.2e6 / (freq * _range);
//              printf(".cpp_range = %d\n", _range);
//              printf("clock = %f\n",clock);
                count_f =19.2e6 / clock;
//              printf(" count_f = %f\n",count_f);
                range = count_f * (state * 10e-4);

                pwmSetClock(clock);
                pwmSetRange(_range);

                pwmWrite(_pin,range);
//              printf("range = %d\n",range);

        }else if(strcmp(name, "start") == 0){
                uint32_t        freq = params->getElement(0)->getInt();
                double          state = params->getElement(1)->getFloat();
                if(state > 1.0 || state < 0.0){
                        printf("Runtime Error :: Duty range is 0.0 - 1.0\n");
                        return NULL;
                }
                period = 1.0 / freq;
//              printf("周期 = %f[s]\n",period);
                h_period = period * state;
//              printf("High = %f[s]\n",h_period);
//              printf("周波数 = %d[Hz]\n",freq);
                clock  = 19.2e6 / (freq * _range);
//              printf(".cpp_range = %d\n", _range);
//              printf("clock = %f\n",clock);
                count_f =19.2e6 / clock;
//              printf(" count_f = %f\n",count_f);
                range = count_f * h_period;

                pwmSetClock(clock);
                pwmSetRange(_range);

                pwmWrite(_pin,range);
//              printf("range = %d\n",range);

        }else if(strcmp(name,"stop") == 0){
                pinMode(_pin,OUTPUT);
                pwmWrite(_pin,0);
        }

        return NULL;
}

Value *PWMGen::evaluate(){
        int pin,freq,bits;
        pin = _list->getElement(0)->getInt();
        bits = _list->getElement(1)->getInt();

        return new PWM(pin, bits);
}
//サーボモーター--------------------------------------------------------------------

Value *SG90::methodCall(char *name, List *params){
#ifdef DEBUG
        printf("SG90::methodCall(%s)\n", name);
#endif
        double state;
        double range;
        double clock ;
        double count_f ;
        double freq = 50;
        _range = 1024;
        if(strcmp(name, "angle") == 0){
                uint32_t        angle = params->getElement(0)->getInt();
//              printf("angle = %d\n",angle);
                clock   = 19.2e6 / (freq * _range);
                count_f = 19.2e6 / clock;
                state   = angle * 0.0104 + 0.5;
//              printf("state = %f\n",state);
                range   = count_f * (state * 10e-4);

                pwmSetClock(clock);
                pwmSetRange(_range);

                pwmWrite(_pin,range);

        }else if(strcmp(name,"stop") == 0){
                pinMode(_pin,OUTPUT);
                pwmWrite(_pin,0);
        }

        return NULL;
}

Value *SG90Gen::evaluate(){
        int pin;
        pin = _list->getElement(0)->getInt();

        return new SG90(pin);
}

//---------------------------------------------------------------------
void AbstructObject::printTree(int s){
        printSpace(s);
        print();
        printf("\n");
}
//OUTPUT関連-----------------------------------------------------------

char _high[] = "HIGH";
char _low[] = "LOW";

char *Output::getText() {
        return _state ? _high : _low;
}

char *Output::getType() {
        return (char*)"Output";
}

Value *Output::evaluate() {
        return new Output(_pin);
}

void Output::print() {
        printf("{Output(%d) state = %d}", _pin, _state);
}

Value *Output::methodCall(char *name, List *params) {
#ifdef DEBUG
        printf("Output::methodCall(%s)\n", name);
#endif
        if (strcmp(name, "high") == 0) {
                high();
                return new Integer(1);
        } else if (strcmp(name, "low") == 0) {
                low();
                return new Integer(0);
        } else if (strcmp(name, "toggle") == 0) {
                toggle();
                return new Integer(_state);
        } else if (strcmp(name, "status") == 0){
                return new Integer(_state);
        } else {
                printf("Undefined function: %s\n", name);
        }
        return NULL;
}

Value *OutputGen::evaluate() {
        int n = _list->size();
        if (n == 0){
                printf("Insufficient number of parameters\n");
                throw ErrorException();
        }
        int port = _list->getElement(0)->getInt();
        uint8_t state = 0;
        if (n >= 2){
                state = _list->getElement(1)->getInt();
        }
        return new Output(port, state);
}

//ボタン-------------------------------------------------------------

Button::Button(uint8_t  pin) {
        _pin = pin;
        pinMode(_pin, INPUT);
        pullUpDnControl(_pin, PUD_UP); // 内部抵抗を使う場合
        _state = digitalRead(_pin);
        _fOnPress = NULL;
        _fOnRelease = NULL;
}

Button::~Button(){
        delete _fOnPress;
        delete _fOnRelease;
}

char *Button::getText() {
        static char high[] = "HIGH";
        static char low[] = "LOW";
        return _state ? high : low;
}

char *Button::getType() {
        return (char*)"Button";
}
Value *Button::evaluate() {
        return new Button(_pin);
}

void Button::print() {
        printf("Button(%d)", _pin);
}

Value *Button::methodCall(char *name, List *params) {
#ifdef DEBUG
        printf("Button::methodCall(%s)\n", name);
#endif

        int n = params->size();
        if (strcmp(name, "onPress") == 0) {
                if (n == 0){
                        onPress(NULL);
                } else {
                        Value *v = params->getElement(0);
                        onPress(v->getText());
                }
        } else if (strcmp(name, "onRelease") == 0){
                if (n == 0){
                        onRelease(NULL);
                } else {
                        Value *v = params->getElement(0);
                        onRelease(v->getText());
                }
        } else {
                printf("Undefined function: %s\n", name);
        }

        return NULL;

}

void Button::onPress(char *fname) {
        if (_fOnPress){
                delete _fOnPress;
                _fOnPress = NULL;
        }
        if (fname == NULL){
                if (_fOnRelease == NULL){
                        _buttonlist_locker.waitForUnlock(410);
                        _buttonList.remove(this);
                        _buttonlist_locker.unlock(410);
                }
        } else {
                _fOnPress = new FuncName(fname);
                _buttonlist_locker.waitForUnlock(420);
                _buttonList.add(this);
                _buttonlist_locker.unlock(420);
        }
}


void Button::onRelease(char *fname) {
        if (_fOnRelease){
                delete _fOnRelease;
                _fOnRelease = NULL;
        }
        if (fname == NULL){
                if (_fOnPress == NULL){
                        _buttonlist_locker.waitForUnlock(430);
                        _buttonList.remove(this);
                        _buttonlist_locker.unlock(430);
                }
        } else {
                _fOnRelease = new FuncName(fname);
                _buttonlist_locker.waitForUnlock(440);
                _buttonList.add(this);
                _buttonlist_locker.unlock(440);
        }
}

void ButtonList::scan(){
        ObjNode *node;
        Button *button;
        for(node = _top; node ; node = node->_next){
                button = (Button *)node->_obj;
                button->scan();
        }
}


void Button::scan() {
        Value *v;
        uint8_t state = digitalRead(_pin);
        if (_state == HIGH && state == LOW) {                   // Press
                if (_fOnPress) {
                        try{
                                v = _fOnPress->evaluate();
                                delete v;
                        }catch(ErrorException e){}
                }
        } else if (_state == LOW && state == HIGH) {            // Release
                if (_fOnRelease) {
                        try{
                                v = _fOnRelease->evaluate();
                                delete v;
                        }catch(ErrorException e){}
                }
        }
        _state = state;
}


void Button::clear() {
        _fOnPress = NULL;
        _fOnRelease = NULL;
//      _btnlist.remove(this);
}


Value *ButtonGen::evaluate() {
        printf("ButttonGen\n");
        int n = _list->size();
        if (n == 0){
                printf("Insufficient number of parameters\n");
                throw ErrorException();
        }
        int port = _list->getElement(0)->getInt();
        return new Button(port);
}
//-------------------------------------------------------------------

//------------------------------------------------------------------タイマー 2024/11/20
void TimerList::dispatch(){

        ObjNode *node;
        Timer *timer;
        for(node = _top; node ; node = node->_next){
                timer = (Timer *)node->_obj;
                timer->dispatch();
        }
}


Timer::Timer(){
        _func = NULL;
        _lastMils = millis();
        _interval = 0;
        _count = 0;
        _isRunning = false;
}

Timer::Timer(unsigned long interval, unsigned long count){
        _func = NULL;
        _lastMils = millis();
        _interval = interval;
        _count = count;
        _isRunning = false;
        printf("Timer(unsigned long)\n");
}

bool Timer::isRunning(){
        return _isRunning;
}

void Timer::dispatch(){
        if (_isRunning){
                unsigned long now = millis();
                if (now - _lastMils >= _interval){
                        _lastMils = now;
                        try{
                                Value *v = _func->evaluate();
                                delete v;
                        }catch(ErrorException e){}
                }
        }
}

char *Timer::getText() {
        return _timerName;
}

char *Timer::getType() {
        return _timerName;
}

Value *Timer::evaluate() {
        return new Timer(_interval, _count);
}

void Timer::print() {
        printf("Timer: interval = %d\n", _interval);
}

Value *Timer::methodCall(char *name, List *params) {
#ifdef DEBUG
        printf("Timer::methodCall(%s)\n", name);
#endif
        int n = params->size();
        if (strcmp(name, "setTimer") == 0) {
                if (n < 2){
                        printf("Insufficient number of parameters.\n");
                        throw ErrorException();
                }
                char *fname = params->getElement(0)->getText();
                if (fname == NULL){
                        printf("Invalid function name.\n");
                        throw ErrorException();
                }
                _func = new FuncName(fname);
                _interval = params->getElement(1)->getInt();
                if (n >= 3){
                        _count = params->getElement(2)->getInt();
                }
                if (_func && _interval > 0){
                        _isRunning = true;
                        _timerlist_locker.waitForUnlock(310);
                        _timerlist.add(this);
                        _timerlist_locker.unlock(310);
                } else {
                        printf("Invalid parameter.\n");
                }
        } else if (strcmp(name, "stop") == 0){
                _timerlist_locker.waitForUnlock(320);
                _timerlist.remove(this);
                _timerlist_locker.unlock(320);
                _isRunning = false;
        } else if (strcmp(name, "resume") == 0){
                if (_isRunning){
                        printf("Already running.\n");
                        return NULL;
                }
                if (_func && _interval > 0){
                        _isRunning = true;
                        _timerlist_locker.waitForUnlock(330);
                        _timerlist.add(this);
                        _timerlist_locker.unlock(330);
                } else {
                        printf("Invalid parameter.\n");
                }
        } else {
                printf("Undefined function: %s\n", name);
        }
        printf("%d\n",_isRunning);
        return NULL;
}

Value *TimerGen::evaluate() {
        return new Timer();
}

//BME280--------------------------------------------------


void BME280Obj::print(){
        printf("AE-BME280");
}

Value *BME280Obj::methodCall(char *name, List *params) {
#ifdef DEBUG
        printf("BME280Obj::methodCall(%s)\n", name);
#endif
        if (strcmp(name, "humidity") == 0) {
                float humid = _bme280.measure_humidity();
                return new Float(humid);
        } else if (strcmp(name, "temperature") == 0) {
                float temp = _bme280.measure_Temperature();
                return new Float(temp);
        } else if (strcmp(name, "pressure") == 0) {
                float press = _bme280.measure_pressure();
                return new Float(press);
        } else {
                printf("Undefined function: %s\n", name);
        }
        return NULL;
}

Value *BME280Obj::evaluate(){
        return NULL;
}

void BME280Obj::begin(){
//      _bme280.begin();
}

Value *BME280Gen::evaluate(){

        BME280Obj *bme280 = new BME280Obj();
        bme280->begin();

        return bme280;
}

//HC-SR04---------------------------------------------------------
void SR04Obj::print(){
        printf("HC-SR04");
}

Value *SR04Obj::methodCall(char *name, List *params) {
#ifdef DEBUG
        printf("SR04Obj::methodCall(%s)\n", name);
#endif
        if (strcmp(name, "getDistance") == 0) {
//              printf("できた\n");
                digitalWrite(_trig_pin, HIGH);
                delayMicroseconds(5);
                digitalWrite(_trig_pin, LOW);


                long beginTime = micros();
                long endTime = 0;

                beginTime = micros();
//              printf("%d\n",beginTime);
                while(digitalRead(_echo_pin) == LOW){
                        endTime = micros() - beginTime;
                        if(endTime == 23323){
                                delay(1);
                                printf("Can not catch pulse\n");
                                break;
                        }
                }


                long startTime = micros();


                beginTime = micros();
                while(digitalRead(_echo_pin) == HIGH){
                        endTime = micros() - beginTime;
                        if(endTime == 23323){
                                delay(1);
                                break;
                        }
                }



                long travelTime = micros() - startTime;

//              printf("できた4\n");

                // 1秒あたりのtravelTimeを秒単位に変換
                double travelTimeInSeconds = travelTime / 1000000.0;
//              printf("%d\n",travelTime);

                // 音速は34300 cm/s
                long distance = (travelTimeInSeconds * 34300) / 2;  // 片道分にするために2で割る
                if(distance >= 400 || distance <= 2){
                        return 0;
                }else {
                        return new Integer(distance);
                }

//              printf("距離: %ld cm\n", distance);
                return new Integer(distance);
        }
        printf("終わり\n");
        return NULL;
}

Value *SR04Obj::evaluate(){
        return NULL;
}

void SR04Obj::begin(){
}

Value *SR04Gen::evaluate(){
        int n = _list->size();
        if (n == 0){
                printf("Insufficient number of parameters\n");
                throw ErrorException();
        }
        int trig = _list->getElement(0)->getInt();
        int echo = _list->getElement(1)->getInt();
        return new SR04Obj( trig, echo);
}

//ドットマトリクスLED----------------------------------------------
/*
#define MAX_MSG     (32)
#define DOT_SIZE    (8)

void DotMatrix::nextLetter(){
        char c = _msg[_next_char];
        if (c >= 'a' && c <= 'z')
                c -= 'a' - 'A';
        if (c < ' ' || c > '~')
                c = 0x7f;

        if (c >= ' ' && c <= '`'){
                _next_index = c - ' ';
        } else if (c >= '{' && c <= '~'){
                _next_index = c - ' ' - 26;
        } else {
                _next_index = 0x7f - ' ' - 26; // Diamond
        }
        _next_col = 2;
        _count = 0;
        if (_msg[_next_char] == '\0'){
                _next_char = 0;
        } else {
                _next_char++;
                if (_msg[_next_char] == '\0')
                        _next_char = 0;
        }
}
void DotMatrix::setText(char *msg){
        strncpy(_msg, msg, MAX_MSG);
        _msg[MAX_MSG - 1] = '\0';
  printf("%s\n", _msg);
        _next_char = 0;
        nextLetter();
}

void DotMatrix::lightRows(uint8_t row){
        uint8_t c = _dotBuff[row];
        uint8_t i, bit;
        for(i = 0; i < DOT_SIZE; i++){
                bit = (c >> (DOT_SIZE - 1 - i)) & 1;
                digitalWrite(_rows[i], bit);
        }
}

void DotMatrix::lightLED(){
        uint8_t i;
        for(i = 0; i < DOT_SIZE; i++){
                lightRows(i);
                digitalWrite(_cols[i], LOW);
                delay(2);
                digitalWrite(_cols[i], HIGH);
        }
}

void DotMatrix::shiftLeft(){
        uint8_t i, bit, index;
        for(i = 0; i < DOT_SIZE; i++){
                _dotBuff[i] <<= 1;
                bit = (_letters[_next_index][i] >> (DOT_SIZE - 1 - _next_col)) & 1;
                _dotBuff[i] |= bit;
        }
        if (_next_col >= 7){
                nextLetter();
        } else {
                _next_col++;
        }
}
void DotMatrix::scroll(){
        lightLED();
        _count++;
        if (_count >= 20){
                _count = 0;
                shiftLeft();
        }
}

Value *DotMatrix::methodCall(char *name, List *params){
        if(strcmp("on",name) == 0){
                int col = params->getElement(0)->getInt();
                Value *v = params->getElement(1);

                digitalWrite(_cols[col],LOW);
                for(int i=0; i<8; i++){
                        if(v->getElement(i)->getInt()){
                                digitalWrite(_rows[i],HIGH);
                        }
                }
        }else if(strcmp("off",name) == 0){
                int col = params->getElement(0)->getInt();
                digitalWrite(_cols[col],HIGH);
                for(int i=0; i<8; i++){
      digitalWrite(_rows[i],LOW);
                }
        }else if(strcmp("set",name) == 0){
    Value *data = params->getElement(0);
    if(strcmp("string", data->getType()) == 0){
      setText(data->getText());
    }else{
      _datalist.setNode(params->getElement(0));
    }
        }else if(strcmp("print",name) == 0){
      DotListNode *data;
      long now, past;
      data = _datalist.getNode(params->getElement(0)->getInt());
      if(data){
        past = now = millis();
        while(now - past <= params->getElement(1)->getInt()){
          for(int i=0; i<8; i++){
            for(int j=0; j<8; j++){
              digitalWrite(_rows[j], data->_data[i][j]);
            }
            digitalWrite(_cols[i], LOW);
            delay(2);
            digitalWrite(_cols[i], HIGH);
          }
          now = millis();
        }
      }
        }else if(strcmp("scroll",name) == 0){
    long time = params->getElement(0)->getInt();
    long now, past;
    past = now = millis();
    while(now - past <= time){
      scroll();
      now = millis();
    }
  }
        return NULL;
}

Value *DotMatrixGen::evaluate(){
  uint8_t rows[dotSize];
  uint8_t cols[dotSize];
        Value *r = _list->getElement(0);
        Value *c = _list->getElement(1);

  for(int i=0; i<dotSize; i++){
    rows[i] = r->getElement(i)->getInt();
    cols[i] = c->getElement(i)->getInt();
  }
        return new DotMatrix(rows,cols);
}
*/
//キャラクタLCD----------------------------------------------------------
Value   *AQM1602obj::methodCall(char *name, List *params){
  if(strcmp(name, "begin") == 0){
    begin();
  }else if(strcmp("clear", name) == 0){
    if(!(params->size() == 0)){
      printf("This has no params\n");
    }
    clear();
  }else if(strcmp("cursor", name) == 0){
    if(params->size() != 2){
      printf("not match params\n");
      return NULL;
    }
    int x = params->getElement(0)->getInt();
    int y = params->getElement(1)->getInt();
    cursor(x,y);
  }else if(strcmp("print", name) == 0){
    if(params->size() != 1){
      printf("not match params\n");
      return NULL;
    }
    Value *v = params->getElement(0);
    if(strcmp("Integer", v->getType()) == 0){
      print(v->getInt());
    }else{
      print(v->getText());
    }
  }else if(strcmp("putChar", name) == 0){
    if(params->size() != 1){
      printf("not match params\n");
      return NULL;
    }
    char *c = params->getElement(0)->getText();
    putChar(*c);
  }else if(strcmp("setContrast", name) == 0){
    if(params->size() != 1){
      printf("not match params\n");
      return NULL;
    }
    int contrast = params->getElement(0)->getInt();
    if(contrast < 0 || contrast > 63){
      printf("contrast : 0 - 63\n");
      return NULL;
    }
    setContrast(contrast);
  }else{
    printf("not found such name of method\n");
  }
  return NULL;
}
/*
 * コマンドを送る。
 */
void AQM1602obj::writeCommand(int command){

//      Wire.beginTransmission(_address);
//      wiringPiI2CWriteReg8(fd, 0x00, 0x00);
        wiringPiI2CWriteReg8(fd, _address, command);
//      Wire.endTransmission();
        delay(10);

}
/*
 * データを送る。
 */
void AQM1602obj::writeData(int data){
//      Wire.beginTransmission(_address);
        wiringPiI2CWriteReg8(fd, _address, 0x38);
        wiringPiI2CWriteReg8(fd, _address, 0x40);
        wiringPiI2CWriteReg8(fd, _address, data);
//      Wire.write(data);
//      Wire.endTransmission();
        delay(1);
}
/*
 * 画面を消去する。
 */
void AQM1602obj::clear(){
        writeCommand(0x01);
}
/*
 * カーソルを(x,y)へ移動する。
 */
void AQM1602obj::cursor(int x, int y){
        unsigned char address = 0x40 * x + y;
        writeCommand(address | 0x80);
}
/*
 * 文字列sを表示する。
 */
void AQM1602obj::print(char *s){
  char c;
  while (c = *s++) {
    int result = wiringPiI2CWriteReg8(fd, 0x40, c);
    usleep(1000);
  }
/*
        for( ; *s ; s++){
                writeData(*s);
        }
*/
}
/*
 * 文字cを表示する。
 */
void AQM1602obj::putChar(char c){
        writeData(c);
}
/*
 * 整数nを十進数で表示する。
 */
void AQM1602obj::print(int n){
        sprintf((char*)_buff, "%d", n);
        print(_buff);
}
/*
 * コントラストを設定する。
 * c: 0～63
 */
void AQM1602obj::setContrast(int c){
        unsigned char c1 = (unsigned char)(0x70 | (c & 0x0f));
        unsigned char c2 = (unsigned char)(0x50 | 0x04 | ((c & 0x30) >> 4));
        writeCommand(0x39);     // Function set: N=1 DH=0 IS=1
        writeCommand(c1);       // Contrast set:(C3,C2,C1,C0)=(x,x,x,x)
        writeCommand(c2);       // Contrast set:(Ion,Bon,C5,C4)=(0,1,x,x)
        writeCommand(0x38);     // Function set: N=1 DH=0 IS=0

}
/*
 * LCDを初期化する。
 */
void AQM1602obj::begin(){
 // Function set: DL=1, N=1, DH=0, IS=0
  wiringPiI2CWriteReg8(fd, 0x00, 0x38);
  usleep(1000);

  // Function set: DL=1, N=1, DH=0, IS=1
  wiringPiI2CWriteReg8(fd, 0x00, 0x39);
  usleep(1000);

  // Internal OSC frequency: BS=0, F2=1, F1=0, F0=0
  wiringPiI2CWriteReg8(fd, 0x00, 0x14);
  usleep(1000);

  // Contrast set: C3=0, C2=0, C1=0, C0=0
  wiringPiI2CWriteReg8(fd, 0x00, 0x73);
  usleep(1000);

  // Power/ICON/Contrast control: Ion=0, Bon=1, C5=1, C4=0
  wiringPiI2CWriteReg8(fd, 0x00, 0x56);
  usleep(1000);

  // Follower control: Fon=1, Rab2=1, Rab1=0, Rab0=0
  wiringPiI2CWriteReg8(fd, 0x00, 0x6c);
  usleep(1000 * 200);

  // Function set: DL=1, N=1, DH=0, IS=0
  wiringPiI2CWriteReg8(fd, 0x00, 0x38);
  usleep(1000);

  // Clear Display
  wiringPiI2CWriteReg8(fd, 0x00, 0x01);
  usleep(1000);

  // Display ON/OFF control: D=1, C=0, B=0
  wiringPiI2CWriteReg8(fd, 0x00, 0x0C);
  usleep(1000 * 1000);
}


Value *AQM1602Gen::evaluate(){
  uint8_t address = _list->getElement(0)->getInt();
  return new AQM1602obj(address);
}

/*
//DHT20---------------------------------------------------------------------
void DHT20::begin(){
//      Wire.begin();
}

int DHT20::scan(){
        uint8_t buff[8];
        long value;
        bool loop = true;
        uint8_t cnt = 0;

        while (loop) {
                wiringPiI2CWriteReg8(fd, 0x38, 0xAC);
                wiringPiI2CWriteReg8(fd, 0x38, 0x33);
                wiringPiI2CWriteReg8(fd, 0x38, 0x00);
                delay(100);

//              Wire.requestFrom(0x38, 7);
                for (uint8_t i = 0; i < 7; i++){
                        buff[i] = Wire.read();
                }
                if (buff[0] & 0x80){
                        Serial.println("Sensor is busy...");
                        cnt++;
                        if (cnt >= 5){
                                loop = false;
                        }
                } else {
                        loop = false;
                }
        }
        // Humidity
        value = buff[1];
        value <<= 8;
        value |= buff[2];
        value <<= 4;
        value |= ((buff[3] >> 4) & 0x0f);
        _humid = value / 10485.76;

        // Temperature
        value = (buff[3] & 0xf);
        value <<= 8;
        value |= buff[4];
        value <<= 8;
        value |= buff[5];
        _temp = value / 5242.88 - 50;

        // CRC
        uint16_t crc = 0xff;
        for(int i = 0; i < 6; i++){
                crc ^= buff[i];
                for(int j = 0; j < 8; j++){
                        crc <<= 1;
                        if (crc > 0xff){
                                crc ^= 0x31;
                        }
                        crc &= 0xff;
                }
        }
        if (crc == buff[6]){
                _status = 1;    // Sucess
        } else {
                _status = 2;    // fail
        }
        return _status;
}

float DHT20::getHumid(){
  scan();
        return _humid;
}

float DHT20::getTemper(){
  scan();
        return _temp;
}

int DHT20::getStatus(){
  scan();
  printf("SDA %d,SCL %d\n",_sda,_scl);
        return _status;
}

Value *DHT20::evaluate() {
        return new DHT20(_sda, _scl);
}

void DHT20::print() {
  printf("DHT20");
}

Value *DHT20::methodCall(char *name, List *params) {
#ifdef DEBUG
        printf("DHT20::methodCall(%s)\n", name);
#endif
        if (strcmp(name, "getHumid") == 0) {
                float humid = getHumid();
    //Serial.println("hu");
                return new Float(humid);
        } else if (strcmp(name, "getTemper") == 0) {
                float temp = getTemper();
                return new Float(temp);
        } else if (strcmp(name, "getStatus") == 0) {
      int status = getStatus();
      return new Integer(status);
  } else {
                printf("Undefined function: %s\n", name);
        }
        return NULL;
}

Value *DHT20Gen::evaluate(){
  int sda = _list->getElement(0)->getInt();
  int scl = _list->getElement(1)->getInt();
        DHT20 *dht20 = new DHT20(sda,scl);
        dht20->begin();
  //Serial.printfwiringPiI2CWriteReg8(fd, 0x00, 0x00);("%d,%d",sda,scl);
  //Wire.setPins(sda,scl);
        return dht20;
}

*/