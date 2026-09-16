#ifndef DLG_TQCS1_H
#define DLG_TQCS1_H

#include <QWidget>

// 前置声明，避免在头文件中包含不必要的重量级头文件
QT_BEGIN_NAMESPACE
namespace Ui
{
    class Dlg_TQCS1;
}
QT_END_NAMESPACE

class QTimer; // 前置声明QTimer

/**
 * @class Dlg_TQCS1
 * @brief 同期参数配置对话框。
 *
 * 负责显示和设置设备的同期参数。
 * 用户可以通过此界面获取、应用、恢复默认参数设置。
 */
class Dlg_TQCS1 : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数。
     * @param parent 父窗口指针，默认为nullptr。
     */
    explicit Dlg_TQCS1(QWidget *parent = nullptr);

    /**
     * @brief 析构函数。
     */
    ~Dlg_TQCS1();

    // --- 公共数据成员 ---
    uint TQCS_FUNCTION;          ///< 功能码，用于标识当前操作（如获取、应用等）
    static uchar ValidData[280]; ///< 静态缓冲区，用于存储从设备接收到的原始字节数据

private slots:
    // --- 按钮点击事件槽函数 ---
    void onButtonApplyClicked();   ///< “应用”按钮点击槽
    void onButtonFetchClicked();   ///< “获取”按钮点击槽
    void onButtonDefaultClicked(); ///< “默认”按钮点击槽
    void onButtonCancelClicked();  ///< “取消”按钮点击槽

    // --- 控件状态改变事件槽函数 ---
    void onControlStateChanged(); ///< [重构] 统一处理所有影响控件启用/禁用状态的事件

    // --- 定时器事件槽函数 ---
    void onFetchDataTimeout(); ///< “获取”数据后的延时处理槽

private:
    /**
     * @brief 初始化所有信号与槽的连接。
     */
    void setupConnections();

    /**
     * @brief [重构] 从UI控件读取数据并更新到成员变量中。
     */
    void updateMembersFromUi();

    /**
     * @brief [重构] 根据成员变量的值，更新整个UI界面的显示。
     */
    void updateUiFromMembers();

    /**
     * @brief [重构] 更新所有依赖于当前状态的控件的启用/禁用状态。
     */
    void updateControlStates();

    /**
     * @brief [重构] 从 ValidData 字节数组中解析数据，填充到成员变量。
     */
    void parseDataFromBuffer();

    /**
     * @brief [重构] 将成员变量中的数据打包成字节流，存入 DATA 数组。
     * @param dataBuffer 用于存放打包后字节数据的缓冲区。
     */
    void packDataToBuffer(uchar *dataBuffer);

    /**
     * @brief 将所有参数恢复为出厂默认值，并更新UI。
     */
    void setDefaultValues();

    /**
     * @brief [辅助函数] 从指定的字节缓冲区和偏移量读取一个ushort (16位无符号整数)。
     * @param buffer 数据源缓冲区。
     * @param offset 读取的起始位置（字节索引）。
     * @return 读取到的ushort值。
     */
    ushort readShort(const uchar *buffer, int offset);

    /**
     * @brief [辅助函数] 将一个ushort值写入指定的字节缓冲区和偏移量。
     * @param buffer 目标缓冲区。
     * @param offset 写入的起始位置（字节索引）。
     * @param value 要写入的ushort值。
     */
    void writeShort(uchar *buffer, int offset, ushort value);

private:
    Ui::Dlg_TQCS1 *ui;        ///< Qt Designer 生成的UI类指针
    QTimer *m_fetchDataTimer; ///< 用于异步获取数据的定时器

    // --- 成员变量 (用于存储参数的当前值) ---
    // 使用 m_ 前缀，并与UI控件名和参数名保持一致性
    // 布尔与整型参数
    bool m_funcEnabled;
    bool m_l1DeadVoltCheck;
    bool m_l2DeadVoltCheck;
    bool m_allDeadVoltCheck;
    int m_phaseSelect;   // 0:A, 1:B, 2:C
    int m_routineSelect; // 0:Auto, 1:Manual
    int m_syncStyle;     // 0:ASYN, 1:SYN

    // 数值型参数
    short m_tcb;
    float m_damax;
    float m_duMax;
    float m_dfMax;
    float m_dfptMax;
    float m_uDead;
    float m_trU1U2;
    float m_uMin;
    float m_uMax;
    float m_fMax;
    float m_fMin;
    float m_tsynDuration;
    float m_synchron;
    float m_dFmax2;
    float m_dUmax2;
    float m_synchron2;
};

#endif // DLG_TQCS1_H
