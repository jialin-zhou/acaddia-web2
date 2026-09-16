#include "TeleProcess.h"

// 构造函数：在创建 TeleProcess 对象时被调用，用于初始化所有成员变量
TeleProcess::TeleProcess()
{
    // 初始化用于打包的数据块结构体
    TelegramBlock.Data = (uchar *)""; // 数据指针指向一个空字符串，防止野指针
    TelegramBlock.Station_adr = 0;    // 站地址默认为0
    TelegramBlock.F1_field = 0;       // 功能码1默认为0
    TelegramBlock.F2_field = 0x80;    // 功能码2默认为0x80
    TelegramBlock.Telegram_Nr = 0;    // 报文序列号默认为0

    // 初始化类的内部状态变量
    Msg_cntr = 0;        // 消息计数器，用于生成报文中的F1字段
    ACK1 = false;        // “从站到主站”的应答标志，初始化为false
    ACK2 = false;        // “主站到从站”的应答标志，初始化为false
    PACK_SELECT = false; // 数据源选择标志，false表示使用传入的参数作为数据源
    P_ValidData = NULL;  // 指向解包后有效数据的指针，初始化为NULL
}

TeleProcess::~TeleProcess()
{
}

/**
 * @brief 将应用数据（“有效数据”）封装成一个完整的、可以发送的通信数据帧。
 * @param Station_adr 目标站地址
 * @param Telegram_Nr 报文序列号
 * @param TelegramBlock1 包含要发送数据的结构体
 * @param Zero 需要在有效数据后补充的0字节个数
 * @return 返回一个指向局部数组的指针
 */
uchar *TeleProcess::TelePack(uchar Station_adr, uchar Telegram_Nr, TeleBlock TelegramBlock1, int Zero)
{
    uchar *p_Telegram = NULL;          // 用于返回的指针
    uchar DataLenth = 0;               // 记录有效数据的长度
    uchar F1 = (Msg_cntr << 5) | 0x09; // 根据消息计数器 Msg_cntr 生成 F1 字段，这是一种将序列号和固定功能码组合的方式
    // 开始计算校验和，首先将报文头部的几个字段值累加起来
    uchar Result_CRC = Station_adr + Telegram_Nr + F1 + TelegramBlock1.F2_field;
    uchar a[280] = ""; // 在栈上创建一个局部数组'a'，用于构建完整的报文

    if (PACK_SELECT == false) // 根据 PACK_SELECT 标志位判断数据源
    {
    LOOP:
        while (*(TelegramBlock1.Data + DataLenth) != '\0') // 循环处理来自参数 TelegramBlock1.Data 的数据
        {
            Result_CRC += *(TelegramBlock1.Data + DataLenth);      // 计算校验和
            a[8 + DataLenth] = *(TelegramBlock1.Data + DataLenth); // 将有效数据存储到数组a里,从a[8]开始
            DataLenth++;                                           // 计算有效数据个数
        }
        // 处理补零逻辑
        Zero--; // 补零计数器减一
        if (Zero >= 0)
        {
            a[8 + DataLenth] = 0; //*(TelegramBlock1.Data+DataLenth); // 在数据末尾添加一个0字节
            DataLenth++;          // 数据长度也要相应增加
            goto LOOP;            // 跳转回LOOP位置，继续检查是否还有数据需要补0
        }
    }
    else if (PACK_SELECT == true) // 逻辑同上，但数据源是类的成员变量 ValidDataToPack
    {
    LOOP2:
        while (ValidDataToPack[DataLenth] != '\0')
        {
            Result_CRC += ValidDataToPack[DataLenth];      // 计算校验和
            a[8 + DataLenth] = ValidDataToPack[DataLenth]; // 将有效数据存储到数组a里,从a[8]开始
            DataLenth++;                                   // 计算有效数据个数
        }
        Zero--;
        if (Zero >= 0)
        {
            a[8 + DataLenth] = 0; //*(TelegramBlock1.Data+DataLenth); // 在数据末尾添加一个0字节
            DataLenth++;          // 数据长度也要相应增加
            goto LOOP2;
        }
    }
    PACK_SELECT = false;            // 数据处理完毕后重置选择标志
    a[0] = 0x68;                    // STX 68H 帧起始符 STX (Start of Text)
    a[1] = 4 + DataLenth;           // Block_len 数据块长度 (从站地址到有效数据的结束，共4个头部字段 + 数据长度)
    a[2] = 4 + DataLenth;           // Block_len
    a[3] = 0x68;                    // STX 68H
    a[4] = Station_adr;             // Station_adr 站地址
    a[5] = F1;                      // F1_field
    a[6] = TelegramBlock1.F2_field; // F2_field
    a[7] = Telegram_Nr;             // Telegram_Nr 报文序列号
    // strcat(a,b);                                   //Valide Data
    a[8 + DataLenth] = Result_CRC; // 校验和
    a[9 + DataLenth] = 0x16;       // ETX 16H 帧结束符

    p_Telegram = a;
    for (int tt = 0; tt < (a[2] + 6); tt++)
        TXDataArray[tt] = (uchar)a[tt];
    return p_Telegram;
}

/**
 * @brief 接收一个原始的数据帧，检查其完整性和正确性，并提取出其中的应用数据。
 * @param Telegram 指向接收到的原始数据字节数组的指针
 * @return 如果报文有效或被识别，则返回 true；否则返回 false。
 */
bool TeleProcess::TeleUnpack(uchar Telegram[]) // 当为数据报文时：将有效数据送出，其他时直接将接收数据送出,在P_ValidData中
{
    int counter = 0;
    uchar Check1 = 0; // 用于在本地计算校验和，以便与接收到的校验和进行比对
    ACK1 = false;     // 每次解包前，都重置应答标志
    ACK2 = false;
    int tt;

    // --- 判断报文类型 ---
    // 通过检查接收数组的第一个字节来识别报文类型
    if ((uchar)0xe5 == Telegram[0]) // 类型 1: 可能是从站(slave)发来的消息
    {
        // 进一步判断这是否是一个完整的数据报文（而不是简单的应答）
        // 检查帧结构：起始符0x68，结束符0x16是否在正确的位置上
        // Telegram 是基地址，*(Telegram + 2) 是解引操作，取出数据块长度
        if ((Telegram[1] == (uchar)0x68) && (Telegram[4] == (uchar)0x68) && (*(Telegram + 6 + *(Telegram + 2))) == (uchar)0x16) // 数据报文
        {
            Block_len = (uchar) * (Telegram + 2);          // 从报文中提取数据块长度
            uchar Check_Sum = *(Telegram + 5 + Block_len); // 从报文中提取接收到的校验和

            // 循环遍历数据块，同时完成两件事：
            // 1. 将数据块内容复制到成员变量 Telegarm_Array 中
            // 2. 在本地计算校验和 Check1
            for (tt = 0; tt < Block_len; tt++)
            {
                Telegarm_Array[tt] = *(Telegram + 5 + tt);
                Check1 += (uchar) * (Telegram + 5 + tt);
            }
            Telegarm_Array[tt] = '\0'; // 在数据末尾添加字符串结束符
            if (Check1 == Check_Sum)   // 校验和正确？
            {
                // 校验通过，报文有效
                Station_adr = Telegarm_Array[0]; // 提取站地址
                Telegram_Nr = Telegarm_Array[3]; // 提取报文序列号
                P_ValidData = Telegarm_Array;    // 让公共指针指向存储了有效数据的内部数组
                ACK1 = ACK2 = false;             // 这不是ACK报文，清除标志
                return true;                     // 返回成功
            }
            else
            {
                return false; // 校验和错误，报文无效
            }
        }
        else // slave -> master // 如果不是完整的数据报文，则认为是"从站到主站"的简单应答
        {
            // AfxMessageBox("Telegram Unpack enter3!: slave -> master");         //DEBUG
            P_ValidData = Telegram; // 让指针直接指向原始接收数据
            // Telegarm_Array=Telegram;
            ACK1 = true; // 设置对应的应答标志位
            return true; // 报文被识别，返回成功
        }
    }

    else if ((uchar)0xa2 == *Telegram) // master -> slave // 类型 2: 可能是主站(master)发来的应答
    {
        // AfxMessageBox("Telegram Unpack enter4!:  master -> slave");         //DEBUG
        P_ValidData = Telegram;
        // Telegarm_Array=Telegram;
        ACK2 = true;
        return true;
    }
    else // 其他未知类型的报文
    {
        // 类型 2: 可能是主站(master)发来的应答
        // AfxMessageBox("Telegram Unpack enter5!:  other");
        ACK1 = false;
        ACK2 = false;
        return false; // 报文无法识别 返回失败
    }
}
