#ifndef DLG_MESSAGE_H
#define DLG_MESSAGE_H

#include <QWidget>

namespace Ui
{
    class Dlg_Message;
}

class Dlg_Message : public QWidget
{
    Q_OBJECT

public:
    explicit Dlg_Message(QWidget *parent = nullptr);
    ~Dlg_Message();

    uint Message_FUNCTION;
    uchar DATA[256];
    uchar *pTX_DATA;

    uchar ValidData[280];

    QString m_Message_Lenth;
    QString m_Message_LNName;
    QString m_Message_DataSetName;
    QString m_Message_LDName;
    QString m_Message_SmpCtr;

    QString m_Message_PhsAArtg;
    QString m_Message_PhsAVrtg;
    QString m_Message_NeutArtg;
    QString m_Message_tdr;

    QString m_Message_StatusWord1;
    QString m_Message_StatusWord2;

    QString m_Message_Channel1;
    QString m_Message_Channel10;
    QString m_Message_Channel11;
    QString m_Message_Channel12;
    QString m_Message_Channel13;
    QString m_Message_Channel14;
    QString m_Message_Channel15;
    QString m_Message_Channel16;
    QString m_Message_Channel17;
    QString m_Message_Channel18;
    QString m_Message_Channel19;
    QString m_Message_Channel2;
    QString m_Message_Channel20;
    QString m_Message_Channel21;
    QString m_Message_Channel22;
    QString m_Message_Channel3;
    QString m_Message_Channel4;
    QString m_Message_Channel5;
    QString m_Message_Channel6;
    QString m_Message_Channel7;
    QString m_Message_Channel8;
    QString m_Message_Channel9;

    bool m_Message_DataInvalid;
    bool m_Message_Interpolation;
    bool m_Message_Mode;
    bool m_Message_OutputType;
    bool m_Message_PHHealth;
    bool m_Message_RangeFlag;
    bool m_Message_SysInvalid;

    int m_Message_DEC;
    void Fetch_data();

private:
    Ui::Dlg_Message *ui;

private slots:
    void MessageConnections();
    void onRadioButton_DEC_Clicked();
    void onRadioButton_HEX_Clicked();
    void OnBUTTONMessageFetch();
    void onButtonCancelClicked();
    void message_show_Data();
};

#endif // DLG_MESSAGE_H
