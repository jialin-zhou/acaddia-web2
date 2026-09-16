#ifndef DLG_BASEANGLE_H
#define DLG_BASEANGLE_H

#include <QWidget>

namespace Ui
{
    class Dlg_BaseAngle;
}

class Dlg_BaseAngle : public QWidget
{
    Q_OBJECT

public:
    explicit Dlg_BaseAngle(QWidget *parent = nullptr);
    ~Dlg_BaseAngle();

    uchar DATA[256];
    uchar *pTX_DATA;
    uint ANGLE_FUNCTION;
    uchar ValidData[280];

    float m_float_da;
    float m_float_I1a;
    float m_float_I1b;
    float m_float_I1c;
    float m_float_I2a;
    float m_float_I2b;
    float m_float_I2c;
    float m_float_U1a;
    float m_float_U1b;
    float m_float_U1c;
    float m_float_U2a;
    float m_float_U2c;
    float m_float_U2b;

private:
    Ui::Dlg_BaseAngle *ui;
private slots:
    void BaseAngleConnection();
    void onButtonFetchClicked();
    void onButtonCancelClicked();
    void Dlg_BaseAngle_update();
    void lineEdit_show();
};

#endif // DLG_BASEANGLE_H
