
#ifndef __COMMAND_H__
#define __COMMAND_H__

#define ARROW_UP        (21)
#define ARROW_DOWN      (22)
#define ARROW_RIGHT     (23)
#define ARROW_LEFT      (24)
#define HOME            (25)
#define INSERT          (26)
#define END                     (27)
#define UnknownKey      (99)

const int Com_Buff_Size = 128;  // １行の上限
const int Hist_Max = 5; // コマンド履歴の上限

class Command{
private:
        char    _inBuffer[Com_Buff_Size];       // 入力バッファ
        int             _pos = 0;               // カーソルの位置
        int             _cnt;                   // 文字数

        // コマンド履歴関連
        char    _histBuff[Hist_Max][Com_Buff_Size];     // 履歴バッファ
        int             _histCur;               // 現在の履歴位置
        int             _histNext;              // 次の履歴位置

        int  onKeyIn(char code);                // Input Char
        void onEscSequence(int code);   // ESC Sequence
        int  judgeKey(char *buff);
        bool isNormalChar(char c);      // 一般の文字（20～7E）
        void shiftRight();                      // カーソル以降を右へシフト
        void shiftLeft();                       // カーソル以降を左へシフト
        void delChar();                         // カーソル上の文字を削除
        void insChar(char code);        // カーソル位置に文字を挿入
        void backSpace();                       // カーソルの左の文字を削除
        void printRight();                      // カーソル以降を表示

        // 履歴機能 -----------------------------------------------------------------
        void restoreHist();                     // 履歴を復元する（履歴バッファ→入力バッファ）
        void pushHistory();                     // 履歴を保存する（入力バッファ→履歴バッファ）
        void clearHistory();            // 履歴をクリアする

public:
        Command(){
//              initCommand();
        }

        void initCommand(bool prompt = true);
        int  readLine(char *buff, int n, bool prompt = true);
};

#endif

