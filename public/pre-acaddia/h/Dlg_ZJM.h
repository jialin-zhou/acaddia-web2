#ifndef DLG_ZJM_H
#define DLG_ZJM_H
#include "Dlg_AD1.h"
#include <QTableWidget>
#include <QMainWindow>
#include "Dlg_ADAdjust1.h"
#include "Dlg_Message.h"
#include "Dlg_BaseAngle.h"
#include "Dlg_TQCS1.h"
#include "Dlg_TQML.h"
#include "Dlg_TimeSet.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class Dlg_ZJM;
}
QT_END_NAMESPACE

/**
 * @class Dlg_ZJM
 * @brief 主界面窗口类，负责显示从设备获取的各类实时数据（如电压、电流、消息等）。
 *
 * 该类管理着主界面的UI元素，包括数据显示表格和功能按钮。
 * 它通过父窗口 new1Dlg 获取通信数据，并通过 updateData() 函数刷新界面。
 */
class Dlg_ZJM : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    Dlg_ZJM(QWidget *parent = nullptr);
    /**
     * @brief 析构函数
     */
    ~Dlg_ZJM();

    // UI状态标志
    bool m_boolZJM_Line1; // Line1复选框是否选中
    bool m_boolZJM_Line2; // Line2复选框是否选中
    bool m_boolZJM_DC;    // DC复选框是否选中

    QTableWidget m_clistctrlMsgList;
    QTableWidget m_clistctrlADCacul;

    // 各子对话框实例，用于数据交换
    Dlg_AD1 AD1_dlg;
    Dlg_ADAdjust1 ADAdjust_dlg;
    Dlg_Message Message_dlg;
    Dlg_BaseAngle BaseAngle_dlg;
    Dlg_TQCS1 TQCS_dlg;
    Dlg_TQML TQML_dlg;
    Dlg_TimeSet TimeSet_dlg;

    // 通信和刷新标志
    int FETCH_REFRESH;

    // 静态数据缓冲区，用于存储从设备接收的原始字节数据
    static uchar ADCacul_ValidData[280]; // 存储AD计算值的原始数据
    uchar MsgList_h_ValidData[1280];     // 存储带报文头的消息列表数据
    uchar MsgList_ValidData[1280];       // 存储无报文头的消息列表数据

    // void OnButtonZjmExit();
    // void closeEvent(QCloseEvent *event); // 关联关闭窗口发射信号函数

protected:
    /**
     * @brief 重写的窗口关闭事件处理函数
     * @param event 关闭事件对象
     */
    void closeEvent(QCloseEvent *event) override; // 关联关闭窗口发射信号函数

signals:
    /**
     * @brief 窗口关闭时发射的信号
     *
     * 该信号用于通知父窗口（new1Dlg），本窗口已被关闭，以便父窗口执行退出逻辑。
     */
    void windowClosed();

private slots:
    // --- UI初始化与数据更新 ---
    void setupUI();    // 连接所有UI控件的信号与槽
    void updateData(); // 核心函数，用最新数据刷新UI表格

    // --- 按钮点击事件槽函数 ---
    void OnButtonZjmFetch();
    void OnButtonZjmSave();
    void OnButtonZjmOpen();
    void OnButtonZjmExit();
    void on_IDC_ZJM_AD1_clicked();      // 原为 on_AD1_clicked
    void on_IDC_ZJM_ADAdjust_clicked(); // 原为 on_ADAdjust1_clicked
    void on_IDC_ZJM_TimeSetBtn_clicked();
    void on_IDC_ZJM_Message_clicked();
    void on_IDC_ZJM_Angle_clicked();
    void on_IDC_VIEW_Set_clicked();
    void on_IDC_ZJM_TQCSBtn_clicked();
    void on_IDC_ZJM_TQMLBtn_clicked();
    void on_IDC_ZJM_ComSetBtn_clicked();

    // --- 日期和时间格式化辅助函数 ---
    /**
     * @brief 计算从一个指定的起始日期开始，经过若干天后的日期。
     * @param startDate 起始日期
     * @param daysElapsed 从起始日期开始所经过的天数
     * @return 格式为 "yyyy-MM-dd" 的日期字符串
     */
    QString getDateStringFromElapsedDays(const QDate &startDate, int daysElapsed);
    /**
     * @brief 将一天内的总毫秒数转换为 "HH:MM:SS" 格式的字符串
     * @param msSinceMidnight 从零点开始的总毫秒数
     * @param milliseconds [out] 用于接收剩余的毫秒部分
     * @return 格式化后的时间字符串
     */
    QString formatEventTime(unsigned int msSinceMidnight, unsigned int &milliseconds);

private:
    /**
     * @brief [新增] 处理单行测量数据的辅助函数
     * @details 从原始数据计算出物理值、百分比和计数值，并将其添加到UI表格中。
     * 此函数旨在消除 updateData() 中的代码重复。
     * @param channelName 通道名称 (e.g., "Line1", "Line2")
     * @param itemIndex 测量项的索引 (0-13)
     * @param rawData 16位的原始ADC数据
     */
    void processMeasurementRow(const QString &channelName, int itemIndex, unsigned short rawData);

    /**
     * @brief [新增] 创建并显示对话框的模板函数
     * @details 这是一个通用的模板函数，用于创建、配置和显示任何类型的对话框，
     * 以避免在每个按钮点击槽函数中重复相同的代码。
     * @tparam DialogType 要创建的对话框的类名
     * @param parent 父窗口指针
     */
    template <typename DialogType>
    void showDialog(QWidget *parent)
    {
        DialogType *dialog = new DialogType(parent);
        dialog->setWindowFlags(Qt::Window);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->show();
    }

    Ui::Dlg_ZJM *ui;
    QTimer *fetchTimer; // 定时器，似乎在当前逻辑中未使用
};
#endif // DLG_ZJM_H
