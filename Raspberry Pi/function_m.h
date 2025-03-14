/*
 Rapsberry Pi function_m.h
*/
#include <wiringPi.h>
#include "BME280.h"
//-------------------------------------------------------------------------------------------------
// オブジェクト生成クラス

class AbstructObject : public Value{
public:
        void    add(Value *val) {}
        void    sub(Value *val) {}
        void    mul(Value *val) {}
        void    div(Value *val) {}
        void    mod(Value *val) {}
        void    power(Value *val) {}
        void    increment(bool ch) {}
        void    decrement(bool ch) {}
        void    minus() {}
        void    printTree(int s);
        // Implement next method:
        // getInt(), getFloat(), getText(), getBool(), getType()
        // evaluate(), print()
};

//----------------------------------------------------------------
class FileObject : public AbstructObject{
private:
        FILE                    *_fp;
        char                    *_fname;
        char                     _mode;
        unsigned char    _state;        // 0:close 1:open

public:
        FileObject(char *name, char *mode);
        ~FileObject();

        long getInt(){
                return _state;
        }

        double getFloat(){
                return (double)_state;
        }

        bool getBool(){
                return _state != 0;
        }

        char    *getText();
        char    *getType();
        Value   *evaluate();
        Value   *methodCall(char *name, List *params);
        void     print();

        unsigned char status(){
                return _state;
        }
};

class FileOpen : public AbstructFunction{
public:
        FileOpen(char *name) : AbstructFunction(name){}

        int getParamCount(){
                return 2;
        }

        Value *evaluate();
};

//-------------------------------------------------------------------------------------
class ObjNode {
public:
        Value   *_obj;
        ObjNode *_next;

        ObjNode(Value *obj, ObjNode *next){
                _obj = obj;
                _next = next;
        }
};

class ObjList {
public:
        ObjNode *_top;

        ObjList(){
                _top = NULL;
        }

        virtual ~ObjList(){
                clearNodes();
        }

        void     clearNodes();
        ObjNode *search(Value *obj);
        void     add(Value *obj);
        void     remove(Value *obj);
};

/*
class ButtonList : public ObjList{
public:
        void scan();
};
*/
//GPIO関係-----------------------------------------------------------------------------------------

class PinMode : public AbstructFunction {
public:
        PinMode(char *name) : AbstructFunction(name) {}
        int getParamCount(){return 2;}
        Value *evaluate();
};

class DigitalWrite : public AbstructFunction {
public:
        DigitalWrite(char *name) : AbstructFunction(name) {}
        int getParamCount(){return 2;}
        Value *evaluate();
};

class AnalogRead :  public AbstructFunction {
public:
        AnalogRead(char *name) : AbstructFunction(name){}

        Value *evaluate();
};

class DigitalRead :  public AbstructFunction {
public:
        DigitalRead(char *name) : AbstructFunction(name){}

        Value *evaluate();
};

//OUTPUT関連------------------------------------------------------------

class Output :  public AbstructObject{
private:
        uint8_t                 _pin;
        uint8_t                 _state;

public:
        Output(uint8_t pin, uint8_t state = 0){
                _pin = pin;
                _state = state;
                pinMode(_pin, OUTPUT);
                digitalWrite(_pin, _state);
        }

        long getInt(){
                return _state;
        }

        double getFloat(){
                return (double)_state;
        }

        bool getBool(){
                return _state != 0;
        }

        char    *getText();
        char    *getType();
        Value   *evaluate();
        void     print();
        Value   *methodCall(char *name, List *params);

        void high(){
                _state = 1;
                digitalWrite(_pin, _state);
        }

        void low(){
                _state = 0;
                digitalWrite(_pin, _state);
        }

        void toggle(){
                _state = 1 - _state;
                digitalWrite(_pin, _state);
        }

        uint8_t status(){
                return _state;
        }
};

class OutputGen : public AbstructFunction {
public:
        OutputGen(char *name) : AbstructFunction(name){}

        Value *evaluate();
};


//PWM制御---------------------------------------------------------------------------
class PWM : public AbstructObject {
private:
        uint8_t         _pin;
        int             _range;
        uint8_t         _bits;
public:
        PWM(int pin,int bits){
                _pin = pin;
                _bits = bits;
                _range = 1;
                if(!(_pin == 18 || _pin == 19)){
                        printf("Setpin Error:: Can use 18 or 19 pin!!\n");
                }
                if(_bits <= 10 && _bits >= 1){
                        for(int i = 0; i < _bits; i++){
                                _range *= 2;
//                              printf("%d\n",_range);
                        }
                }else{
                        printf("Setpin Error::Bits is 1 - 10 !!\n");
                }

                pinMode(_pin,PWM_OUTPUT);
                pwmSetMode(PWM_MODE_MS) ;
                printf("bit = %d\n",_range);
//              pwmSetRange(_range);
        }

        long getInt(){
                return _pin;
        }

        double getFloat(){
                return (double)_pin;
        }

        bool getBool(){
                return _pin != 0;
        }

        char    *getText() {
                return (char*)"PWM";
        }
        char    *getType() {
                return (char*)"typePWM";
        }
        Value   *evaluate() {
                return new PWM(_pin, _bits);
        }
        void     print() {
                printf("print->{PWM(%d),bits = %d}\n",_pin,_bits);
        }

        Value   *methodCall(char *name, List *params);
};

class PWMGen : public AbstructFunction {
public:
        PWMGen(char *name) : AbstructFunction(name){}

        int getParamCount(){return 2;}

        Value *evaluate();
};


//SG90---------------------------------------------------------------------------
class SG90 : public AbstructObject {
private:
        uint8_t         _pin;
        int             _range;
        uint8_t         _bits;
public:
        SG90(int pin){
                _pin = pin;
                if(!(_pin == 18 || _pin == 19)){
                        printf("Setpin Error:: Can use 18 or 19 pin!!\n");
                }

                pinMode(_pin,PWM_OUTPUT);
                pwmSetMode(PWM_MODE_MS) ;
        }

        long getInt(){
                return _pin;
        }

        double getFloat(){
                return (double)_pin;
        }

        bool getBool(){
                return _pin != 0;
        }

        char    *getText() {
                return (char*)"SG90";
        }
        char    *getType() {
                return (char*)"typeSG90";
        }
        Value   *evaluate() {
                return new SG90(_pin);
        }
        void     print() {
                printf("print->{SG90(%d),bits = %d}\n",_pin,_bits);
        }

        Value   *methodCall(char *name, List *params);
};

class SG90Gen : public AbstructFunction {
public:
        SG90Gen(char *name) : AbstructFunction(name){}

        int getParamCount(){return 1;}

        Value *evaluate();
};





/*
class LedcAttach :  public AbstructFunction {
public:
        LedcAttach(char *name) : AbstructFunction(name){}
        int getParamCount(){ return 3;}
        Value *evaluate();
};

class LedcDetach :  public AbstructFunction {
public:
        LedcDetach(char *name) : AbstructFunction(name){}
        int getParamCount(){ return 0;}
        Value *evaluate();
};

class LedcWrite :  public AbstructFunction {
public:
        LedcWrite(char *name) : AbstructFunction(name){}
        int getParamCount(){ return 2;}
        Value *evaluate();
};

class LedcWriteTone :  public AbstructFunction {
public:
        LedcWriteTone(char *name) : AbstructFunction(name){}

        Value *evaluate();
};

class LedcRead :  public AbstructFunction {
public:
        LedcRead(char *name) : AbstructFunction(name){}

        Value *evaluate();
};

class LedcReadFreq :  public AbstructFunction {
public:
        LedcReadFreq(char *name) : AbstructFunction(name){}

        Value *evaluate();
};
*/
//ボタン----------------------------------------------------------------------
class Button : public AbstructObject{
private:
        uint8_t                 _pin;
        FuncName                *_fOnPress;
        FuncName                *_fOnRelease;
        uint8_t                 _state;

public:
        Button(uint8_t  pin);
        ~Button();
        void    onPress(char *fname);
        void    onRelease(char *fname);
        void    scan();
        void    clear();

        long getInt(){
                return _state;
        }

        double getFloat(){
                return (double)_state;
        }

        bool getBool(){
                return _state != 0;
        }

        char    *getText();
        char    *getType();
        Value   *evaluate();
        void     print();
        Value   *methodCall(char *name, List *params);
};

class ButtonGen : public AbstructFunction {
public:
        ButtonGen(char *name) : AbstructFunction(name){}

        Value *evaluate();
};

class ButtonList : public ObjList{
      public:
              void scan();
};
//タイマー--------------------------------------------------------------------------

class TimerList : public ObjList{
public:
        void dispatch();
};


class Timer : public AbstructObject{
private:
        FuncName                *_func;
        unsigned long   _lastMils;
        unsigned long   _interval;
        unsigned long   _count;
        bool                    _isRunning;

public:
        Timer();
        Timer(unsigned long interval, unsigned long count);
        ~Timer(){
                delete _func;
        }
        void    dispatch();
        bool    isRunning();

        long getInt(){
                return 0;
        }
        double getFloat(){
                return 0.0;
        }
        bool getBool(){
                return false;
        }

        char    *getText();
        char    *getType();
        Value   *evaluate();
        void     print();
        Value   *methodCall(char *name, List *params);
};

class TimerGen : public AbstructFunction {
public:
        TimerGen(char *name) : AbstructFunction(name){}
        int getParamCount(){ return 0;}
        Value *evaluate();
};

//BME280------------------------------------------------------------------

class BME280Obj : public AbstructObject{        //12/16 private消してます
private:
        BME280  _bme280;
public:

        BME280Obj(){
                _bme280 = BME280();
        }

        void begin();

        char    *getType(){
                return (char*)"AE-BME280";
        }

        long getInt(){
                return 0;
        }

        double getFloat(){
                return 0.0;
        }

        bool getBool(){
                return false;
        }

        char    *getText(){
                return NULL;
        }

        Value   *evaluate();

        void     print();
        Value   *methodCall(char *name, List *params);
};

class BME280Gen : public AbstructFunction {
public:
        BME280Gen(char *name) : AbstructFunction(name){}
        int getParamCount(){ return 0;}
        Value *evaluate();
};

//HC-SR04---------------------------------------------------------------


class SR04Obj : public AbstructObject{
private:
        int     _trig_pin;
        int     _echo_pin;
public:

        SR04Obj(int trig,int echo){
                _trig_pin = trig;
                _echo_pin = echo;
                pinMode(_trig_pin,OUTPUT);
                pinMode(_echo_pin,INPUT);
                digitalWrite(_trig_pin,LOW);
                delay(30);
        }

        void begin();

        char    *getType(){
                return (char*)"SR04";
        }

        long getInt(){
                return 0;
        }

        double getFloat(){
                return 0.0;
        }

        bool getBool(){
                return false;
        }

        char    *getText(){
                return NULL;
        }

        Value   *evaluate();
        void     print();
        Value   *methodCall(char *name, List *params);
};

class SR04Gen : public AbstructFunction {
public:
        SR04Gen(char *name) : AbstructFunction(name){}
        int getParamCount(){ return 2; }
        Value *evaluate();
};
//DotMatrixLED--------------------------------------------------------
/*
class DotListNode {
public:
  int   _data[8][8];
  DotListNode *_next;
public:
  DotListNode(Value *data, DotListNode *next){
    for(int i=0; i<8; i++){
      for(int j=0; j<8; j++){
        _data[i][j] = data->getElement(i)->getElement(j)->getInt();
      }
    }
    _next = next;
  }

  void print(){
    for(int i=0; i<8; i++){
      for(int j=0; j<8; j++){
        int d = _data[i][j];
        printf("%d ",d);
      }
      printf("\n");
    }
  }
};
class DotList {
private:
  DotListNode *_top;
  DotListNode *_last;
public:
  DotList(){
    _top = _last = NULL;
  }
  void setNode(Value *data){
    if(_top == NULL){
      _top =_last = new DotListNode(data, NULL);
    }else{
      _last->_next = new DotListNode(data, NULL);
      _last = _last->_next;
    }
  }
  DotListNode *getNode(int n){
    DotListNode *node;
    int i=0;
    for(node = _top; node; node = node->_next){
      if(i == n){
        return node;
      }
      i++;
    }
    return NULL;
  }
};
class DotMatrix : public AbstructObject {
private:
        const uint8_t   dotSize = 8;
        uint8_t         _rows[8];
        uint8_t         _cols[8];
        DotList   _datalist;
        char    _msg[MAX_MSG];
        uint8_t _next_char;
        uint8_t _next_index;
        uint8_t _dotBuff[DOT_SIZE];
        uint8_t _next_col;
  uint8_t _count;

        void    nextLetter();
        void    lightRows(uint8_t row);
        void    lightLED();

public:
        DotMatrix(uint8_t *rows, uint8_t *cols){
                uint8_t i;

                // set pinMode
                for(i = 0; i < dotSize; i++){
                        _cols[i] = cols[i];
                        _rows[i] = rows[i];
                        pinMode(_cols[i], OUTPUT);
                        digitalWrite(cols[i], HIGH);
                        pinMode(_rows[i], OUTPUT);
                        digitalWrite(rows[i], LOW);
      _dotBuff[i] = 0x00;
                }
    setText("HELLO");
        }
        void    setText(char *msg);
        void    scroll();
        void    shiftLeft();

        long getInt(){
                return 8;
        }

        double getFloat(){
                return (double)8;
        }

        bool getBool(){
                return 8 != 0;
        }

        char    *getText() {
                return "textDotMatrix";
        }
        char    *getType() {
                return "typeDotMatrix";
        }
        Value   *evaluate() {
                return new DotMatrix(_rows,_cols);
        }
        void     print() {
                printf("printDotMatrix");
        }

        Value   *methodCall(char *name, List *params);
};
class DotMatrixGen : public AbstructFunction {
private:
  const uint8_t dotSize = 8;
public:
        DotMatrixGen(char *name) : AbstructFunction(name){}

        int getParamCount(){return 2;}

        Value *evaluate();
};
*/
//キャラクタLCD---------------------------------------------------------------
class AQM1602obj : public AbstructObject {
private:
  uint8_t   _address;
  char      _buff[16];
  int       fd;

        void writeCommand(int command);
        void writeData(int data);
public:
  AQM1602obj(uint8_t address){
    _address = address;
    fd = wiringPiI2CSetup(0x3e);

  }
        void begin();
        void clear();
        void cursor(int x, int y);
        void print(char *s);
        void print(int n);
        void putChar(char c);
        void setContrast(int c);

        long getInt(){
                return 0;
        }
        double getFloat(){
                return (double)0;
        }
        bool getBool(){
                return 0;
        }
        char    *getText() {
                return (char*)"textAQM1602";
        }
        char    *getType() {
                return (char*)"typeAQM1602";
        }
        Value   *evaluate() {
                return new AQM1602obj(_address);
        }
        void     print() {
                printf("printAQM1602");
        }

        Value   *methodCall(char *name, List *params);

};
class AQM1602Gen : public AbstructFunction {
private:
public:
  AQM1602Gen(char *name) : AbstructFunction(name){}

  int getParamCount(){ return 1; }

  Value *evaluate();
};

/*
//DHT20------------------------------------------------------------------------------

class DHT20 : public AbstructObject {
private:
  int           fd;
  uint8_t _sda, _scl;
  float _humid;         // Humidity
        float   _temp;          // Temperature
        int             _status;        // ステータス  0:Not Yet  1:Success  2:Fail
public:
        DHT20(int sda, int scl){
    _sda = sda;
    _scl = scl;
        _status = 0;
        fd = wiringPiI2CSetup(0x38);
        }
        char    *getType(){
                return "DHT20";
        }

        long getInt(){
                return 0;
        }

        double getFloat(){
                return 0.0;
        }

        bool getBool(){
                return false;
        }

        char    *getText(){
                return NULL;
        }

        void    begin();
        int             scan();
        float   getHumid();
        float   getTemper();
        int             getStatus();
  Value *evaluate();
        void     print();
        Value   *methodCall(char *name, List *params);

};

class DHT20Gen : public AbstructFunction {
public:
        DHT20Gen(char *name) : AbstructFunction(name){}

  int getParamCount(){
    return 2;
  }

        Value *evaluate();
};

*/