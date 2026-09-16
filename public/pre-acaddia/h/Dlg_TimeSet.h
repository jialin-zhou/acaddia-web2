#ifndef DLG_TIMESET_H
#define DLG_TIMESET_H

#include <QWidget>
#include <QDateTime>
#include <QTimer>

namespace Ui
{
    class Dlg_TimeSet;
}

/**
 * @class Dlg_TimeSet
 * @brief 时间设置对话框。
 *
 * 该窗口负责显示和设置设备的时间。
 * 它支持两种模式：
 * 1. 自动更新模式：实时显示当前系统时间。
 * 2. 手动模式：允许用户从设备获取时间、修改时间，并将修改后的时间发送回设备。
 * 时间数据通过父窗口(Dlg_ZJM)和祖父窗口(new1Dlg)与硬件进行通信。
 */
class Dlg_TimeSet : public QWidget
{
    Q_OBJECT

public:
    // --- 公有成员变量 ---
    uint Function_ID;     // 功能ID，用于区分不同的操作（如获取时间、设置时间）
    uchar ValidData[280]; // 用于从父窗口接收设备返回的原始字节数据

    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    explicit Dlg_TimeSet(QWidget *parent = nullptr);
    /**
     * @brief 析构函数
     */
    ~Dlg_TimeSet();

private slots:
    // --- 槽函数 ---
    void onTimer();                              // 定时器超时处理，用于自动更新时间显示
    void onSetTimeButtonClicked();               // “设置时间”按钮点击事件
    void onGetTimeButtonClicked();               // “获取时间”按钮点击事件
    void onExitButtonClicked();                  // “退出”按钮点击事件
    void onAutoUpdateCheckBoxChanged(int state); // “自动更新”复选框状态改变事件

    // 日期或时间控件的值被用户手动修改时触发
    // 对应MFC的OnDatetimechangeDatetimepickerTime和OnDatetimechangeDatetimepickerData
    void onDateTimeChanged();

private:
    // --- 私有辅助函数 ---
    void setupConnections();                    // 统一设置所有信号槽连接
    void processDeviceData();                   // 处理从设备接收到的原始数据
    void calculateDateTimeFromDeviceData();     // [重构] 从设备数据中解析出日期和时间
    void sendTimeToDevice();                    // 将UI上的时间打包并发送到设备
    void syncUIToDateTime(const QDateTime &dt); // [新增] 将给定的QDateTime同步到UI控件上，减少代码重复

private:
    // --- 私有成员变量 ---
    Ui::Dlg_TimeSet *ui;
    QTimer *m_timer;             // 定时器，用于驱动自动更新
    QDateTime m_currentDateTime; // 存储当前界面显示的日期时间
    bool m_autoUpdateEnabled;    // 标志位，表示是否启用自动更新
    bool m_isUserModified;       // 标志位，表示用户是否在手动模式下修改了时间
};
#endif // DLG_TIMESET_H
