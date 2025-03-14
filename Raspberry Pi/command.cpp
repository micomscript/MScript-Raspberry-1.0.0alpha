#include <wiringPi.h>
#include<wiringPiI2C.h>
#include <string.h>
#include "command.h"
#include <stdio.h>
#include <cstdio>

extern int _sentence_depth;
extern int _system_mode;  // 0:Dialog Mode  1:Script Mode

char _cursorLeft[] = { 0x5B, 0x44, 0x00 };
char _cursorRight[] = { 0x5B, 0x43, 0x00 };
char _cursorUp[] = { 0x5B, 0x41, 0x00 };
char _cursorDown[] = { 0x5B, 0x42, 0x00 };
char _home[] = { 0x5B, 0x31, 0x7E, 0x00 };
char _end[] = { 0x5B, 0x34, 0x7E, 0x00 };

void Command::initCommand(bool prompt){
        _pos = 0;
        _cnt = 0;
        _inBuffer[0] = '\0';
        if (prompt){
                if (_sentence_depth >= 1){
                        printf("  ");
                } else {
                        printf("$ ");
                }
        }
}

/*
 * 一般の文字(20～7E)ならばtrueを返す。
 */
bool Command::isNormalChar(char c){
        return (c >= 0x20) && (c <= 0x7e);
}
/*
 * pos から行末までを右へシフトする
 */
void Command::shiftRight(){
        int i;
        for(i = _cnt; i >= _pos; i--){
                _inBuffer[i+1] = _inBuffer[i];
        }
}
/*
 * pos から行末までを左へシフトする
 */
void Command::shiftLeft(){
        for(int i = _pos; _inBuffer[i-1] = _inBuffer[i] ; i++){
        }
}

/*
 * pos から行末までを表示
 */
void Command::printRight(){
        int i, n = 0;
        for(i = _pos; _inBuffer[i] != '\0'; i++){
                printf("%c",_inBuffer[i]);
                n++;
        }
        for(i = 0; i < n; i++)
                printf("%c",'\b');      // BackSpace
}

void Command::delChar(){
        if (_inBuffer[_pos] == '\0')
                return;
        _pos++;
        shiftLeft();
        _pos--;
        printf("\x1b[K");
        printRight();
        _cnt--;
}

void Command::insChar(char code){
        if (_cnt >= Com_Buff_Size - 1)
                return;
        shiftRight();
        _inBuffer[_pos] = code;
        printRight();
        printf("%c",_inBuffer[_pos++]);
        _cnt++;
}

void Command::backSpace(){
        if (_pos == 0)
                return;
        shiftLeft();
        _pos--;
        printf("%c",'\b');
        printf("\x1b[K");
        printRight();
        _cnt--;
}

int Command::judgeKey(char *buff){
        if (strcmp(buff, _cursorUp) == 0)
                return ARROW_UP;
        if (strcmp(buff, _cursorDown) == 0)
                return ARROW_DOWN;
        if (strcmp(buff, _cursorLeft) == 0)
                return ARROW_LEFT;
        if (strcmp(buff, _cursorRight) == 0)
                return ARROW_RIGHT;
        if (strcmp(buff, _home) == 0)
                return HOME;
        if (strcmp(buff, _end) == 0)
                return END;
        return UnknownKey;
}

/*
 * 履歴バッファをクリアする
 */
void Command::clearHistory(){
        int i;
        for(i = 0; i < Hist_Max; i++)
                _histBuff[i][0] = '\0';
        _histCur = _histNext = 0;
}

/*
 * 入力されたコマンドを履歴バッファへコピーする
 */
void Command::pushHistory(){
        strcpy(_histBuff[_histNext], _inBuffer);
        _histNext = (_histNext == Hist_Max - 1) ? 0 : _histNext + 1;
        _histCur = _histNext;
}

/*
 * 履歴バッファを入力領域へコピーする
 */
void Command::restoreHist(){
        strcpy(_inBuffer, _histBuff[_histCur]);
        Serial.print("\x0d");
        Serial.print("\x1b[K");
        if (_sentence_depth >= 1){
                Serial.print("  ");
        } else {
                Serial.print("$ ");
        }
        _cnt = 0;
        for(_pos = 0; _inBuffer[_pos] ; _pos++){
                Serial.print(_inBuffer[_pos]);
                _cnt++;
        }
}

int Command::onKeyIn(char code){
        // 通常の文字
        if (isNormalChar(code)){
                insChar(code);
                return 0;
        }
        // BS(BackSpace)
        if (code == 0x08){
                backSpace();
                return 0;
        }
        // DEL
        if (code == 0x7f){
                delChar();
                return 0;
        }
        // TAB
        if (code == 0x09){
                int n  = 4 - _pos % 4;
                for(int i = 0;  i < n ; i++){
                        insChar(' ');
                }
                return 0;
        }
        // ESC Sequence
        if (code == 0x1b){
                char buff[8];
                int i = 0;
                while (Serial.available())
                        buff[i++] = Serial.read();
                buff[i] = '\0';
                int code = judgeKey(buff);
                if (code != UnknownKey){
                        onEscSequence(code);
                }
                return 0;
        }
        // 残りの文字は無視
        return 0;
}

void Command::onEscSequence(int code){
        if (code == ARROW_LEFT){
                if (_pos >= 1){
                        Serial.print('\b');
                        _pos--;
                }
                return;
        }
        if (code == ARROW_RIGHT){
                if (_pos < Com_Buff_Size - 1){
                        if (_inBuffer[_pos] == '\0')
                                return;
                        Serial.print(_inBuffer[_pos++]);
                }
                return;
        }
        if (code == HOME){
                Serial.print("\x0d");
                if (_sentence_depth >= 1){
                        Serial.print("  ");
                } else {
                        Serial.print("$ ");
                }
                _pos = 0;
                return;
        }
        if (code == END){
                for( ; _inBuffer[_pos] != '\0' ; _pos++){
                        Serial.print(_inBuffer[_pos]);
                }
                return;
        }
        if (code == ARROW_UP){
                _histCur = (_histCur == 0) ? Hist_Max - 1 : _histCur - 1;
                restoreHist();
                return;
        }
        if (code == ARROW_DOWN){
                _histCur = (_histCur == Hist_Max - 1) ? 0 : _histCur + 1;
                restoreHist();
                return;
        }
}

int Command::readLine(char *buff, int n, bool prompt){
        char lineBuff[100];
        int  cnt;
        char ch;
        initCommand(prompt);
        while(true){
                if (Serial.available()){
                        ch = Serial.read();
                        if (ch == '\r')
                                break;
                        onKeyIn(ch);
                }
        }
        pushHistory();
        strncpy(buff, _inBuffer, n);
        Serial.println();
        return strlen(buff);
}
