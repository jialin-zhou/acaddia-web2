#ifndef TELEPROCESS_H
#define TELEPROCESS_H
#include <QtGlobal>

typedef struct
{
    uchar Station_adr; // station_adr
    uchar F1_field;    // F1_field
    uchar F2_field;    // F2_field
    uchar Telegram_Nr; // Telegram_Nr
    uchar *Data;       // Valid data
    //	UCHAR Data1[256];    //VALID DATA (BAK)
} TeleBlock;

class TeleProcess
{
public:
    TeleBlock TelegramBlock;
    bool ACK1;       // 用于确认是否是短应答信号slave->master,初始化位0，解包是若是被置为1
    bool ACK2;       // 用于确认是否是短应答信号master->slave,初始化位0，解包是若是被置为1
    uchar Block_len; // number of zero in Valid data
    uchar Telegram_Nr;
    uchar Station_adr;
    uchar TXDataArray[280];    // Data Telegram array
    uchar Telegarm_Array[256]; // valid data array
    uchar *P_ValidData;        // valid data pointer
    uchar Msg_cntr;
    uchar ValidDataToPack[256];
    bool PACK_SELECT;
    uchar *TelePack(uchar Station_adr, uchar Telegram_Nr, TeleBlock TelegramBlock, int Zero);
    bool TeleUnpack(uchar *Telegram1);
    TeleProcess();
    virtual ~TeleProcess();
};

#endif // TELEPROCESS_H
