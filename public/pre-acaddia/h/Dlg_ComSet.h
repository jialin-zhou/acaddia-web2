#ifndef DLG_COMSET_H
#define DLG_COMSET_H

#include <QWidget>
#include <QString>
#include <QSerialPort>
#include <QSerialPortInfo>

namespace Ui
{
    class Dlg_ComSet;
}

class Dlg_ZJM;
class new1Dlg;

class Dlg_ComSet : public QWidget
{
    Q_OBJECT

public:
    explicit Dlg_ComSet(QWidget *parent = nullptr);
    ~Dlg_ComSet();

    int m_nComSetCom;    // COM口
    QString settingpara; // 串口设置参数字符串

private slots:
    void onOkButtonClicked();     // 确定按钮点击
    void onCancelButtonClicked(); // 退出按钮点击
    void onGetComButtonClicked(); // 获取串口按钮点击

private:
    Ui::Dlg_ComSet *ui;

    // 串口参数
    int m_nComSetBaud;  // 波特率
    int m_nComSetCheck; // 校验位
    int m_nComSetData;  // 数据位
    int m_nComSetStop;  // 停止位
    // int m_nComSetCom;    // COM口

    void showEvent(QShowEvent *event) override; // 窗口显示事件
    void initComboBoxes();                      // 初始化下拉框
    void updatePortSettings();                  // 更新端口设置

    // 获取父窗口指针
    Dlg_ZJM *getParentZJM();
    new1Dlg *getGrandParent();
};

#endif // DLG_COMSET_H
