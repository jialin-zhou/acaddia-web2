#ifndef DLG_AD1_H
#define DLG_AD1_H
#define DIM_US   0x1BB7 //0x24F3*0x1BB7/0x2000H=0x2000H
//#define DIM_UL  0x3000
//#define DIM_UL  0x1BB7
#define DIM_IS   0x1B70 //0x2553*0x1B70/0x2000H=0x2000H
#define DIM_DCS  0x4FEC
#define DIM_U   0x6EDC //0x24F3*0x6EDC/0x4000H=0x4000H
//#define DIM_UL  0xC000
//#define DIM_UL  0x6EDC
/*
    1A-3.5V	  --0x5B74
    1A-1V(1A) --0x15D87
    1A-1V(5A) --0x45E8
*/
#define DIM_I   0x45E8

#include <QWidget>

namespace Ui {
class Dlg_AD1;
}

class Dlg_AD1 : public QWidget
{
    Q_OBJECT

public:
    explicit Dlg_AD1(QWidget *parent = nullptr);
    ~Dlg_AD1();
    uchar DATA[256];
    static uchar ValidData[280];
    uint AD1_FUNCTION;

    short	m_shortAD1_I1a;
    short	m_shortAD1_I1b;
    short	m_shortAD1_I1c;
    short	m_shortAD1_I2a;
    short	m_shortAD1_I2b;
    short	m_shortAD1_I2c;
    short	m_shortAD1_U1a;
    short	m_shortAD1_U1b;
    short	m_shortAD1_U1c;
    short	m_shortAD1_U2a;
    short	m_shortAD1_U2b;
    short	m_shortAD1_U2c;


    int	m_shortAD1_C_DC1;
    int	m_shortAD1_C_DC2;
    int	m_shortAD1_C_DC3;
    int	m_shortAD1_C_DC4;
    int	m_shortAD1_C_DC5;
    int	m_shortAD1_C_DC6;
    int	m_shortAD1_C_DC7;
    int	m_shortAD1_C_DC8;

    short m_shortAD1_V_DC1;
    short m_shortAD1_V_DC2;
    short m_shortAD1_V_DC3;
    short m_shortAD1_V_DC4;
    short m_shortAD1_V_DC5;
    short m_shortAD1_V_DC6;
    short m_shortAD1_V_DC7;
    short m_shortAD1_V_DC8;


    short m_shortAD1_V_LINE1;
    short m_shortAD1_V_LINE2;
    short m_shortAD1_C_LINE1;
    short m_shortAD1_C_LINE2;

    int	m_Model_Sec;
    int	m_Delay;
    int	m_Primary_Current_Scaling;
    int	m_nAD1_SecVNV = 0;


    int m_nAD1_DCSS;
    int m_nAD1_DCSS2;
    int m_nAD1_DCSS3;
    int m_nAD1_DCSS4;
    int m_nAD1_DCSS5;
    int m_nAD1_DCSS6;
    int m_nAD1_DCSS7;
    int m_nAD1_DCSS8;

    int	m_Channel_Sec1;
    int	m_Channel_Sec2;
    int	m_Channel_Sec3;
    int	m_Channel_SecX;
    int	m_Delay_A;
    int	m_Delay_B;
    int	m_Delay_C;
    int	m_Delay_X;

    void processDeviceData();
    void updateUiFromData();
private slots:
    void setupConnections();
    void onradioButtonToggled(bool checked);
    void onradioButton_2Toggled(bool checked);
    void onradioButton_3Toggled(bool checked);
    void onradioButton_4Toggled(bool checked);
    void onradioButton_5Toggled(bool checked);
    void onradioButton_6Toggled(bool checked);
    void onradioButton_7Toggled(bool checked);
    void onradioButton_8Toggled(bool checked);
    void onradioButton_9Toggled(bool checked);
    void onradioButton_10Toggled(bool checked);
    void onradioButton_11Toggled(bool checked);
    void onradioButton_12Toggled(bool checked);
    void onradioButton_13Toggled(bool checked);
    void onradioButton_14Toggled(bool checked);
    void onradioButton_15Toggled(bool checked);
    void onradioButton_16Toggled(bool checked);
    void onpushButtonAd1ApplyClicked();
    void onpushButton_2Ad1FetchClicked();
    void onpushButton_3Ad1DefaultClicked();
    void onpushButton_4AdExitClicked();
private:
    Ui::Dlg_AD1 *ui;

};

#endif // DLG_AD1_H
