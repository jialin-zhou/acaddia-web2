#ifndef NEW1DLG_H
#define NEW1DLG_H
#include "TeleProcess.h"
#include <QWidget>
#include "Dlg_ZJM.h"
#include <QtSerialPort/QSerialPort>
#include <QTimer>

namespace Ui
{
    class new1Dlg;
}

class new1Dlg : public QWidget
{
    Q_OBJECT

public:
    explicit new1Dlg(QWidget *parent = nullptr);
    ~new1Dlg();
    TeleBlock Msg_block_32, Msg_block_34, ACAD_block_33, Msg_h_block_12, Msg_block_13, ADAdjust_block_38, ADData_block_37, TimeSet_block_29, TimeSet_block_28,
        Message_block_49, AC_angle_block_47, TQCS_block_35;
    uint Down;
    bool DC_V_I_select[8];
    int Current_Style;
    int DIM_public;
    int DIM_Line_Vot_style;
    volatile uint Connect_Index;
    uint ErrorCounter;
    uint index;
    uint Msg_Len;
    TeleProcess Connect_Telegram;
    int Zero_NR;
    bool Fun_Connect(TeleBlock TeleBlock1, int zero_NR);
    uchar TXDATA1[280]; // Array of the sent data
    uchar RXTele_Nr;
    Dlg_ZJM ZJM_dlg;

    QString m_CStringStatus;
    void SentData(uchar TXData[]);
    uchar RXDAT[280];
    bool RX_Msg_End;

    QSerialPort *m_serialPort = nullptr;

signals:
    void finished();

private:
    Ui::new1Dlg *ui;

    QTimer *m_timer11;
    QTimer *m_timer12;
    // int m_connectIndex = 0;
    void showZjmDialog();

private slots:
    void setupConnections_new1Dlg();
    void initSerialPort();
    void handleReadyRead();
    void initTimers();

    void handleTimer11();
    void handleTimer12();

protected:
    void OnOffline();
    void OnConnect();
    void OnExit();

signals:
    void serialPortConnected(const QString &portName);
};

#endif // NEW1DLG_H
