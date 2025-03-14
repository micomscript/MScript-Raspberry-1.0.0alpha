
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

const int Com_Buff_Size = 128;  // �P�s�̏��
const int Hist_Max = 5; // �R�}���h�����̏��

class Command{
private:
        char    _inBuffer[Com_Buff_Size];       // ���̓o�b�t�@
        int             _pos = 0;               // �J�[�\���̈ʒu
        int             _cnt;                   // ������

        // �R�}���h�����֘A
        char    _histBuff[Hist_Max][Com_Buff_Size];     // �����o�b�t�@
        int             _histCur;               // ���݂̗����ʒu
        int             _histNext;              // ���̗����ʒu

        int  onKeyIn(char code);                // Input Char
        void onEscSequence(int code);   // ESC Sequence
        int  judgeKey(char *buff);
        bool isNormalChar(char c);      // ��ʂ̕����i20�`7E�j
        void shiftRight();                      // �J�[�\���ȍ~���E�փV�t�g
        void shiftLeft();                       // �J�[�\���ȍ~�����փV�t�g
        void delChar();                         // �J�[�\����̕������폜
        void insChar(char code);        // �J�[�\���ʒu�ɕ�����}��
        void backSpace();                       // �J�[�\���̍��̕������폜
        void printRight();                      // �J�[�\���ȍ~��\��

        // �����@�\ -----------------------------------------------------------------
        void restoreHist();                     // �����𕜌�����i�����o�b�t�@�����̓o�b�t�@�j
        void pushHistory();                     // ������ۑ�����i���̓o�b�t�@�������o�b�t�@�j
        void clearHistory();            // �������N���A����

public:
        Command(){
//              initCommand();
        }

        void initCommand(bool prompt = true);
        int  readLine(char *buff, int n, bool prompt = true);
};

#endif

