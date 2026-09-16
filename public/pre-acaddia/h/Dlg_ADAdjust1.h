#ifndef DLG_ADADJUST1_H
#define DLG_ADADJUST1_H

#include <QRadioButton>
#include <QList>

#include <QWidget>

namespace Ui
{
    class Dlg_ADAdjust1;
}

struct ChannelData
{
    int Index;
    float f_para1;
    QString Cs_para2;
    QString Cs_para3;
    QString Cs_para4;
    QString Cs_para5;
    bool b_paraOK;
};

class Dlg_ADAdjust1 : public QWidget
{
    Q_OBJECT

public:
    explicit Dlg_ADAdjust1(QWidget *parent = nullptr);
    ~Dlg_ADAdjust1();
    QString FormChang(float INPUT_float);

    bool m_boolADAdjust_Ia_OK;
    bool m_boolADAdjust_Ib_OK;
    bool m_boolADAdjust_Ic_OK;
    bool m_boolADAdjust_Ua_OK;
    bool m_boolADAdjust_Ub_OK;
    bool m_boolADAdjust_Uc_OK;

    int m_nADAdjust_ACChannel;
    int m_nADAdjust_Channel;
    int m_nADAdjust_DCChannel;

    QString m_CStringADAdjust_Ia_2;
    QString m_CStringADAdjust_Ia_3;
    QString m_CStringADAdjust_Ia_4;
    QString m_CStringADAdjust_Ia_5;
    QString m_CStringADAdjust_Ib_2;
    QString m_CStringADAdjust_Ib_3;
    QString m_CStringADAdjust_Ib_4;
    QString m_CStringADAdjust_Ib_5;
    QString m_CStringADAdjust_Ic_2;
    QString m_CStringADAdjust_Ic_3;
    QString m_CStringADAdjust_Ic_4;
    QString m_CStringADAdjust_Ic_5;
    QString m_CStringADAdjust_Ua_2;
    QString m_CStringADAdjust_Ua_3;
    QString m_CStringADAdjust_Ua_4;
    QString m_CStringADAdjust_Ua_5;
    QString m_CStringADAdjust_Ub_2;
    QString m_CStringADAdjust_Ub_3;
    QString m_CStringADAdjust_Ub_4;
    QString m_CStringADAdjust_Ub_5;
    QString m_CStringADAdjust_Uc_2;
    QString m_CStringADAdjust_Uc_3;
    QString m_CStringADAdjust_Uc_4;
    QString m_CStringADAdjust_Uc_5;

    float m_floatADAdjust_Uc_1;
    float m_floatADAdjust_Ub_1;
    float m_floatADAdjust_Ua_1;
    float m_floatADAdjust_Ic_1;
    float m_floatADAdjust_Ib_1;
    float m_floatADAdjust_Ia_1;

    unsigned int CHECK_STATUS;

    ChannelData Line1[6];
    ChannelData Line2[6];
    ChannelData DC[8];

    uint Function_ID;
    uchar ValidData[280];
    uchar ParaData[50];

private slots:
    void onRadioChannelline1Toggled(); // line1
    void onRadioChannelLine2Toggled(); // line2
    void onRadioChannelDCToggled();    // DC
    // line1 or line2 Ua\Ub\Uc\Ia\Ib\Ic
    void onRadioACChannelUaToggled();
    void onRadioACChannelUbToggled();
    void onRadioACChannelUcToggled();
    void onRadioACChannelIaToggled();
    void onRadioACChannelIbToggled();
    void onRadioACChannelIcToggled();
    // DC 零漂
    void onRadioDCChannelVDC1Toggled();
    void onRadioDCChannelVDC2Toggled();
    void onRadioDCChannelVDC3Toggled();
    void onRadioDCChannelVDC4Toggled();

    void onRadioDCChannelVDC5Toggled();
    void onRadioDCChannelVDC6Toggled();
    void onRadioDCChannelVDC7Toggled();
    void onRadioDCChannelVDC8Toggled();

    // DC 系数

    void onRadioDCChannelCDC1Toggled();
    void onRadioDCChannelCDC2Toggled();
    void onRadioDCChannelCDC3Toggled();
    void onRadioDCChannelCDC4Toggled();
    void onRadioDCChannelCDC5Toggled();
    void onRadioDCChannelCDC6Toggled();
    void onRadioDCChannelCDC7Toggled();
    void onRadioDCChannelCDC8Toggled();

    void onEditUa1EditingFinished();
    void onEditUb1EditingFinished();
    void onEditUc1EditingFinished();
    void onEditIa1EditingFinished();
    void onEditIb1EditingFinished();
    void onEditIc1EditingFinished();
    void update_LineEdit();
    void onButtonApplyClicked();
    void onButtonCancelClicked();
    // void onButtonNewParApplyClicked();
    void control();
    void setRadioButtonsEnabled(const QList<QRadioButton *> &buttons, bool enabled);
    void setRadioButtonsChecked(const QList<QRadioButton *> &buttons, bool checked);
    void setRadioButtonsExclusived(const QList<QRadioButton *> &buttons, bool Exclusived);

private:
    Ui::Dlg_ADAdjust1 *ui;

    void saveData();
};

#endif // DLG_ADADJUST1_H
