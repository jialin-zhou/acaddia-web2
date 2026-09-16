#ifndef DLG_TQML_H
#define DLG_TQML_H

#include <QWidget>
#include <QDateTime>
#include <QTimer>

namespace Ui
{
    class Dlg_TQML;
}

/**
 * @class Dlg_TQML
 * @brief "同期命令" 设置对话框。
 *
 * 该窗口用于显示和下发与设备同步相关的命令和参数。
 * 它能从设备获取接收(REC)、发送(SENT)和时间(TIME)三种记录，
 * 并能执行应用(Apply)和重置(Reset)操作。
 * 数据通过祖父窗口 new1Dlg 与硬件进行通信。
 */
class Dlg_TQML : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    explicit Dlg_TQML(QWidget *parent = nullptr);
    /**
     * @brief 析构函数
     */
    ~Dlg_TQML();
    uchar ValidData[280]; // 有效数据数组
    uint Function_ID;     // 功能ID，用于区分不同的操作（如应用、获取）

private:
    Ui::Dlg_TQML *ui;
    QTimer *m_timer; // 定时器

    // --- 成员变量 ---
    // 状态与控制
    int m_nTQML_REC;          // 单选按钮状态 (0: Rec, 1: Sent, 2: Time)
    bool m_BOOLTQML_REC_OK;   // 接收成功标志
    bool m_BOOLTQML_SENT_OK;  // 发送成功标志
    bool m_BOOLTQML_TIME_OK;  // 时间记录成功标志
    bool m_BOOLTQML_UNFINISH; // 未完成标志
    bool m_BOOLTQML_RESET;    // 已重置标志

    // "接收(REC)" 分组框数据
    QDateTime m_DateTimeTQML_REC_DATE; // 接收日期时间
    uint m_nTQML_REC_MS;               // 接收毫秒
    float m_floatTQML_REC_DA;          // 接收DA
    float m_floatTQML_REC_DF;          // 接收DF
    float m_floatTQML_REC_DU;          // 接收DU
    float m_floatTQML_REC_F1;          // 接收F1
    float m_floatTQML_REC_F2;          // 接收F2
    float m_floatTQML_REC_U1;          // 接收U1
    float m_floatTQML_REC_U2;          // 接收U2
    float m_floatTQML_REC_DF_DT;       // 接收DF/DT
    float m_floatTQML_REC_F1_DT;       // 接收F1/DT
    float m_floatTQML_REC_F2_DT;       // 接收F2/DT

    // "发送(SENT)" 分组框数据
    QDateTime m_DateTimeTQML_SENT_DATE; // 发送日期时间
    uint m_nTQML_SENT_MS;               // 发送毫秒
    float m_floatTQML_SENT_DA;          // 发送DA
    float m_floatTQML_SENT_DF;          // 发送DF
    float m_floatTQML_SENT_DU;          // 发送DU
    float m_floatTQML_SENT_F1;          // 发送F1
    float m_floatTQML_SENT_F2;          // 发送F2
    float m_floatTQML_SENT_U1;          // 发送U1
    float m_floatTQML_SENT_U2;          // 发送U2
    float m_floatTQML_SENT_DF_DT;       // 发送DF/DT
    float m_floatTQML_SENT_F1_DT;       // 发送F1/DT
    float m_floatTQML_SENT_F2_DT;       // 发送F2/DT

    // "时间(TIME)" 分组框数据
    QDateTime m_DateTimeTQML_TIME_DATE; // 时间日期
    uint m_nTQML_TIME_MS;               // 时间毫秒
    float m_floatTQML_TIME_DA;          // 时间DA
    float m_floatTQML_TIME_DF;          // 时间DF
    float m_floatTQML_TIME_DU;          // 时间DU
    float m_floatTQML_TIME_F1;          // 时间F1
    float m_floatTQML_TIME_F2;          // 时间F2
    float m_floatTQML_TIME_U1;          // 时间U1
    float m_floatTQML_TIME_U2;          // 时间U2
    float m_floatTQML_TIME_DF_DT;       // 时间DF/DT
    float m_floatTQML_TIME_F1_DT;       // 时间F1/DT
    float m_floatTQML_TIME_F2_DT;       // 时间F2/DT

    // --- 内部辅助函数 ---
    /**
     * @brief 初始化窗口数据和状态
     */
    void initData();
    /**
     * @brief 用当前成员变量的值更新UI界面显示
     */
    void updateUI();
    /**
     * @brief 解析从设备接收到的 ValidData 原始数据
     */
    void parseData();
    /**
     * @brief [新增] 重置所有数据显示字段和状态标志
     * @details 用于统一处理初始化、复位命令等场景下的数据清零操作，避免代码重复。
     */
    void resetDataFields();
    /**
     * @brief [新增] 从原始字节数组中解析出日期和时间
     * @param data 包含原始数据的字节数组 (ValidData)
     * @param dayOffset 存储"天数"的起始字节偏移量
     * @param msOffset 存储"毫秒数"的起始字节偏移量
     * @return 解析得到的 QDateTime 对象
     */
    QDateTime parseDateTimeFromData(const uchar *data, int dayOffset, int msOffset);

private slots:

    void onButtonApplyClicked(); // 应用按钮点击事件
    void onButtonFetchClicked(); // 获取按钮点击事件
    void on_pushButton_Cancel_clicked();

    // 单选按钮改变事件
    void onRadioRecChecked();
    void onRadioSentChecked();
    void onRadioTimeChecked();

    void onShowWindow(bool visible); // 窗口显示事件
    void onTimerTimeout();           // 定时器事件
};

#endif // DLG_TQML_H
