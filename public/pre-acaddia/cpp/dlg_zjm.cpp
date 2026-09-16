#include "Dlg_ZJM.h"
#include "ui_Dlg_ZJM.h"
#include "Dlg_AD1.h"
#include "Dlg_ADAdjust1.h"
#include "new1Dlg.h"
#include "qmessagebox.h"
#include <QFileDialog>
#include "Dlg_TimeSet.h"
#include "Dlg_Message.h"
#include "Dlg_BaseAngle.h"
#include "DLg_DIMset.h"
#include "Dlg_TQCS1.h"
#include "Dlg_TQML.h"
#include "Dlg_ComSet.h"
#include <QScreen>
#include <QDebug>
#include <QDate>
#include <QCloseEvent>

// 初始化静态数据成员
uchar Dlg_ZJM ::ADCacul_ValidData[280] = { 0 };
// uchar Dlg_ZJM :: MsgList_h_ValidData[280];
// uchar Dlg_ZJM :: MsgList_ValidData[280];

Dlg_ZJM::Dlg_ZJM(QWidget *parent) : QMainWindow(parent), ui(new Ui::Dlg_ZJM)
{
    // connect(ui->IDC_ZJM_AD1,&QPushButton::clicked,this,&Dlg_ZJM::on_AD1_clicked);

    // --- UI 字体和DPI缩放设置 ---
    ui->setupUi(this); // 如果一个槽函数的名字是 on_<objectName>_<signalName>()，Qt就会自动将名为 <objectName> 的控件的 <signalName> 信号连接到这个槽上
    qreal pixelRatio = qApp->devicePixelRatio();
    qreal dpiRatio = QGuiApplication::primaryScreen()->logicalDotsPerInch() / 96.0; // 获取逻辑DPI缩放比（考虑系统字体缩放）
    qreal scaleFactor = qMin(pixelRatio, dpiRatio);                                 // 综合缩放因子
    QFont font = this->font();                                                      // 获取基准字体
    font.setPixelSize(qRound(12 * scaleFactor));                                    // 基于DPI缩放
    this->setFont(font);

    // --- 初始化成员变量 ---
    m_boolZJM_Line1 = false;
    m_boolZJM_Line2 = false;
    m_boolZJM_DC = false;
    FETCH_REFRESH = 0;

    // 初始化数组
    // for(int j = 0; j < 280; j++) {
    //     ADCacul_ValidData[j] = 0;
    // }

    setWindowTitle("ACAD DIA");
    setupUI(); // 调用UI设置函数
    // updateData();

    // --- 加载并显示背景图片 ---
    QPixmap pixmap(":/picture/CEE_1.png"); // 加载图片1
    ui->label->setPixmap(pixmap);
    ui->label->resize(pixmap.size());
    ui->label->show();

    QPixmap pixmap1(":/picture/CEE_2.png"); // 加载图片2
    ui->label_2->setPixmap(pixmap1);
    ui->label_2->resize(pixmap1.size());
    ui->label_2->lower();
    ui->label_2->show();
}

/**
 * @brief Dlg_ZJM 类的析构函数。
 * @details 负责清理和释放由该类管理的资源，主要是 Qt Designer 生成的 UI 对象。
 */
Dlg_ZJM::~Dlg_ZJM()
{
    delete ui;
}

/**
 * @brief 连接所有UI控件的信号与槽
 */
void Dlg_ZJM::setupUI()
{
    // 连接功能按钮
    // connect(ui->IDC_ZJM_TimeSetBtn, &QPushButton::clicked, this, &Dlg_ZJM::on_IDC_ZJM_TimeSetBtn_clicked);
    // connect(ui->IDC_ZJM_Message, &QPushButton::clicked, this, &Dlg_ZJM::on_IDC_ZJM_Message_clicked);
    // connect(ui->IDC_ZJM_Angle, &QPushButton::clicked, this, &Dlg_ZJM::on_IDC_ZJM_Angle_clicked);
    // connect(ui->IDC_VIEW_Set, &QPushButton::clicked, this, &Dlg_ZJM::on_IDC_VIEW_Set_clicked);
    // connect(ui->IDC_ZJM_TQCSBtn, &QPushButton::clicked, this, &Dlg_ZJM::on_IDC_ZJM_TQCSBtn_clicked);
    // connect(ui->IDC_ZJM_TQMLBtn, &QPushButton::clicked, this, &Dlg_ZJM::on_IDC_ZJM_TQMLBtn_clicked);
    // connect(ui->IDC_ZJM_ComSetBtn, &QPushButton::clicked, this, &Dlg_ZJM::on_IDC_ZJM_ComSetBtn_clicked);
    // [修正] 移除了对 IDC_ZJM_AD1 和 IDC_ZJM_ADAdjust 的手动connect
    // 因为它们的槽函数命名遵循Qt约定，ui->setupUi(this); 会自动连接
    // connect(ui->IDC_ZJM_AD1, &QPushButton::clicked, this, &Dlg_ZJM::on_AD1_clicked);
    // connect(ui->IDC_ZJM_ADAdjust, &QPushButton::clicked, this, &Dlg_ZJM::on_ADAdjust1_clicked);

    // 一个名为 setupCheckbox 通用的"安装" Lambda 函数，它的作用是简化为一个 QCheckBox 控件和一个 bool 成员变量设置信号槽连接的过程。
    // [&] 表示以引用方式捕获所有外部变量，主要是为了捕获 this 指针。
    auto setupCheckbox = [&](QCheckBox *checkbox, bool &member) {
        // 使用 Qt 的 connect 函数连接信号和槽。
        connect(
            // 当复选框状态改变时，此信号会被发射。
            checkbox, &QCheckBox::stateChanged,

            // 上下文对象：当前类的实例。
            this,

            // 槽函数：这里使用另一个 Lambda 表达式作为槽。
            // [this, &member] 是捕获列表，作为为 让内部 lambda 表达式可以使用外部变量
            // 捕获 this 是为了能调用成员函数 updateData()。
            // 捕获 &member 是为了能直接修改对应的类成员变量（如 m_boolZJM_Line1）。
            [this, &member](int state) {
                // 核心逻辑：根据信号传递来的 state 更新布尔成员变量的值。
                // 如果 state 是 Qt::Checked（值为2），则表达式为 true，否则为 false。
                member = (state == Qt::Checked);

                // 在状态变量更新后，立即调用 updateData() 刷新UI，
                // 以便表格内容能够根据复选框的最新状态进行显示。
                updateData();
            });
    };
    setupCheckbox(ui->checkBox, m_boolZJM_Line1);
    setupCheckbox(ui->checkBox_2, m_boolZJM_Line2);
    setupCheckbox(ui->checkBox_3, m_boolZJM_DC);

    // 连接底部操作按钮
    connect(ui->pushButton_10, &QPushButton::clicked, this, &Dlg_ZJM::OnButtonZjmFetch); // 获取信息
    connect(ui->pushButton_11, &QPushButton::clicked, this, &Dlg_ZJM::OnButtonZjmSave);  // 保存参数
    connect(ui->pushButton_12, &QPushButton::clicked, this, &Dlg_ZJM::OnButtonZjmOpen);  // 打开文件
    connect(ui->pushButton_13, &QPushButton::clicked, this, &Dlg_ZJM::OnButtonZjmExit);  // 退出
}

/**
 * @brief [新增] 处理单行测量数据的辅助函数
 * @details 从原始数据计算出物理值、百分比和计数值，并将其添加到UI表格中。
 * 此函数旨在消除 updateData() 中的代码重复。
 * @param channelName 通道名称 (e.g., "Line1", "Line2")
 * @param itemIndex 测量项的索引 (0-13)
 * @param rawData 16位的原始ADC数据
 */
void Dlg_ZJM::processMeasurementRow(const QString &channelName, int itemIndex, unsigned short rawData)
{
    qDebug() << u8"调用 updateData 刷新UI";
    new1Dlg *pParent = qobject_cast<new1Dlg *>(parent());

    if (!pParent)
        return;

    // 1. 计算 Count (原始计数值)
    QString countStr = QString::number(rawData);

    // 2. 标准化原始数据
    float normalizedValue = rawData;
    // 注意: Line1 使用固定值 16899，而 Line2 使用父窗口的 DIM_public。
    // 这是一个潜在的逻辑不一致点，此处按原逻辑实现。
    normalizedValue /= (channelName == "Line1") ? 16899.0f : static_cast<float>(pParent->DIM_public);

    // 3. 计算物理单位值 (Second) 并处理正负号
    float physicalValue = normalizedValue;
    QString unit;
    bool isNegative = false;
    unsigned short absRawData = rawData;

    // 统一处理可能为负的 P, Q, Cos phi
    if (itemIndex == 9 || itemIndex == 10 || itemIndex == 13)
    {
        if (absRawData > 32767)
        {
            isNegative = true;
            absRawData = ~absRawData + 1; // 补码转为绝对值（正数）
            // 用绝对值重新计算标准化值
            normalizedValue = absRawData / static_cast<float>(pParent->DIM_public);
        }
    }

    // 使用 switch 结构根据测量项索引进行计算，提高可读性
    switch (itemIndex)
    {
    case 0:
    case 1:
    case 2: // Ua, Ub, Uc
        physicalValue *= 57.74f;
        unit = "V";
        break;
    case 3:
    case 4:
    case 5: // Uab, Ubc, Uca
        physicalValue *= (pParent->DIM_Line_Vot_style == 1) ? 57.74f : 100.00f;
        unit = "V";
        break;
    case 6:
    case 7:
    case 8: // Ia, Ib, Ic
        physicalValue *= (pParent->Current_Style == 0) ? 5.0f : 1.0f;
        unit = "A";
        break;
    case 9:
    case 10:
    case 11: // P, Q, S
    {
        float powerNormalizedValue = absRawData / static_cast<float>(pParent->DIM_public);
        physicalValue = powerNormalizedValue * ((pParent->Current_Style == 0) ? 865.5f : 173.1f);
        if (itemIndex != 11 && isNegative)
            physicalValue *= -1; // S总是正值
        if (itemIndex == 9)
            unit = "W";
        else if (itemIndex == 10)
            unit = "Var";
        else
            unit = "VA";
        break;
    }
    case 12: // f
        physicalValue *= 50.0f;
        unit = "Hz";
        break;
    case 13: // Cos phi
        physicalValue = absRawData / static_cast<float>(pParent->DIM_public);
        if (isNegative)
            physicalValue *= -1;
        unit = ""; // 无单位
        break;
    }

    // 4. 使用QString进行安全、便捷的格式化
    QString percentStr = QString::asprintf("%.4f%%", normalizedValue * 100.0f);
    QString secondStr = QString::asprintf("%.4f %s", physicalValue, unit.toStdString().c_str()).trimmed();

    // 5. 更新UI表格
    const QString str_Item[14] = { "Ua", "Ub", "Uc", "Uab", "Ubc", "Uca", "Ia", "Ib", "Ic", "P", "Q", "S", "f", "Cos phi" };
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);
    ui->tableWidget->setItem(row, 0, new QTableWidgetItem(channelName));
    ui->tableWidget->setItem(row, 1, new QTableWidgetItem(str_Item[itemIndex]));
    ui->tableWidget->setItem(row, 2, new QTableWidgetItem(secondStr));
    ui->tableWidget->setItem(row, 3, new QTableWidgetItem(percentStr));
    ui->tableWidget->setItem(row, 4, new QTableWidgetItem(countStr));
}

/**
 * @brief [重构] 更新UI显示的核心函数
 */
void Dlg_ZJM::updateData()
{
    qDebug() << u8"调用 updateData 刷新UI";
    new1Dlg *pParent = qobject_cast<new1Dlg *>(parent());
  
    if (!pParent)
    {
        QMessageBox::warning(this, u8"错误", u8"无法获取new1Dlg父窗口");
        return;
    }

    // --- 准备表格 ---
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnCount(5);
    ui->tableWidget->setHorizontalHeaderLabels({ "Channel", "ShowItem", "Second", "%", "Count" });

    // --- 处理 Line1 ---
    if (m_boolZJM_Line1)
    {
        for (int j = 0; j < 14; j++)
        {
            unsigned short rawData = (ADCacul_ValidData[21 + 2 * j] << 8) | ADCacul_ValidData[20 + 2 * j];
            processMeasurementRow("Line1", j, rawData);
        }
    }

    // --- 处理 Line2 ---
    if (m_boolZJM_Line2)
    {
        for (int j = 0; j < 14; j++)
        {
            unsigned short rawData = (ADCacul_ValidData[49 + 2 * j] << 8) | ADCacul_ValidData[48 + 2 * j];
            processMeasurementRow("Line2", j, rawData);
        }
    }

    // --- 处理 DC ---
    if (m_boolZJM_DC)
    {
        const QString str_DCItem[8] = { "DC1", "DC2", "DC3", "DC4", "DC5", "DC6", "DC7", "DC8" };
        for (int j = 0; j < 8; j++)
        {
            unsigned short rawData = (ADCacul_ValidData[5 + 2 * j] << 8) | ADCacul_ValidData[4 + 2 * j];
            float normalizedValue = rawData / static_cast<float>(pParent->DIM_public);
            float physicalValue = normalizedValue * ((pParent->DC_V_I_select[j] == 0) ? 5.0f : 10.0f);
            QString unit = (pParent->DC_V_I_select[j] == 0) ? "V" : "mA";

            int row = ui->tableWidget->rowCount();
            ui->tableWidget->insertRow(row);
            ui->tableWidget->setItem(row, 0, new QTableWidgetItem("DC"));
            ui->tableWidget->setItem(row, 1, new QTableWidgetItem(str_DCItem[j]));
            ui->tableWidget->setItem(row, 2, new QTableWidgetItem(QString::asprintf("%.4f %s", physicalValue, unit.toStdString().c_str()).trimmed()));
            ui->tableWidget->setItem(row, 3, new QTableWidgetItem(QString::asprintf("%.4f%%", normalizedValue * 100.0f)));
            ui->tableWidget->setItem(row, 4, new QTableWidgetItem(QString::number(rawData)));
        }
    }

    // --- 处理消息列表 ---
    // 条件：父窗口的 Down 标志为 1005，这通常意味着消息数据已经获取完毕
    if (pParent->Down == 1005)
    {
        // 清空第二个表格 (tableWidget_2) 并设置表头
        ui->tableWidget_2->clear();
        ui->tableWidget_2->setRowCount(0);
        ui->tableWidget_2->setColumnCount(5);                                                     // 设置列数
        ui->tableWidget_2->setHorizontalHeaderLabels({ "MsgID", "Date", "Time", "ms", "Event" }); // 设置表头
        int nListCount_Msg = ui->tableWidget_2->rowCount();                                       // 获取行数

        // 定义用于解析消息的临时变量
        uint Temp_MS, ms_UINT;
        unsigned short MsgID, Temp_Date_short, Temp_short; // DayofYear_short:表示一年中的第几天
        uchar Temp_UCHAR, Msg_NR, Temp_Date_UCHAR;
        QString MsgID_CString, Date_CString, Time_CString, ms_CString, Event_CString;
        // MsgList_h_ValidData[13]=12;

        //////////////////////////////////////////////////////////////////////////
        //               处理 MsgList_h_ValidData[] (带报文头的消息列表)
        //////////////////////////////////////////////////////////////////////////
        Msg_NR = MsgList_h_ValidData[13] & 0x7F; // 从字节数组的第13个位置获取消息数量，并用 & 0x7F 清除最高位，确保数值正确
        for (int k = 0; k < Msg_NR; k++)
        {
            // MsgID 处理
            Temp_UCHAR = MsgList_h_ValidData[14 + 8 * k];   // 低字节
            MsgID = MsgList_h_ValidData[15 + 8 * k];        // 高字节
            MsgID = (MsgID << 8) + Temp_UCHAR;              // 组合成16位MsgID
            MsgID_CString = QString::number(MsgID - 32768); // 减去偏移量32768后转为字符串

            // 在表格中插入新行并设置MsgID
            ui->tableWidget_2->insertRow(nListCount_Msg);
            ui->tableWidget_2->setItem(nListCount_Msg, 0, new QTableWidgetItem(MsgID_CString));

            // --- 解析日期 ---
            const QDate baseDate1984(1984, 1, 1); // 定义基准日期
            Temp_Date_UCHAR = MsgList_h_ValidData[16 + 8 * k];
            Temp_Date_short = MsgList_h_ValidData[17 + 8 * k];
            Temp_Date_short = (Temp_Date_short << 8) + Temp_Date_UCHAR;                     // 组合成16位的天数
            Date_CString = getDateStringFromElapsedDays(baseDate1984, Temp_Date_short + 1); // 调用辅助函数，计算从基准日期过了 Temp_Date_short + 1 天后的日期
            ui->tableWidget_2->setItem(nListCount_Msg, 1, new QTableWidgetItem(Date_CString));

            // --- 解析时间 ---
            // 从4个字节组合成一个32位的总毫秒数 (从当天零点开始计时)
            Temp_UCHAR = MsgList_h_ValidData[20 + 8 * k];
            Temp_short = MsgList_h_ValidData[21 + 8 * k];
            Temp_MS = (static_cast<uint>(Temp_short) << 8) + Temp_UCHAR;
            Temp_MS <<= 16;
            Temp_UCHAR = MsgList_h_ValidData[18 + 8 * k];
            Temp_short = MsgList_h_ValidData[19 + 8 * k];
            Temp_MS += (static_cast<uint>(Temp_short) << 8) + Temp_UCHAR;
            // 调用辅助函数，将总毫秒数格式化为 "HH:MM:SS" 字符串，并提取出剩余的毫秒数
            Time_CString = formatEventTime(Temp_MS, ms_UINT);
            ms_CString = QString::number(ms_UINT);
            ui->tableWidget_2->setItem(nListCount_Msg, 2, new QTableWidgetItem(Time_CString));
            ui->tableWidget_2->setItem(nListCount_Msg, 3, new QTableWidgetItem(ms_CString));

            // --- 解析 Event (事件状态) ---
            // 如果原始MsgID大于32767（即最高位为1），表示发生/置位，记为"+"
            Event_CString = (MsgID > 32767) ? "+" : "-";
            ui->tableWidget_2->setItem(nListCount_Msg, 4, new QTableWidgetItem(Event_CString));

            nListCount_Msg++; // 行计数器加一
        }

        //////////////////////////////////////////////////////////////////////////
        //               处理 MsgList_ValidData[] (无报文头的消息列表)
        //////////////////////////////////////////////////////////////////////////
        // 循环处理，消息数量由 MsgList_ValidData[4] 决定
        for (int kk = 0; kk < (MsgList_ValidData[4] - 128); kk++)
        {
            // --- 解析 MsgID ---
            // 注意：这里的字节组合方式与上面不同，且高字节似乎来自 MsgList_h_ValidData 数组
            Temp_UCHAR = MsgList_ValidData[5 + 8 * kk];
            MsgID = MsgList_h_ValidData[6 + 8 * kk];
            MsgID = (MsgID << 8) + Temp_UCHAR;
            MsgID_CString = QString::number(MsgID - 32768);

            ui->tableWidget_2->insertRow(nListCount_Msg);
            ui->tableWidget_2->setItem(nListCount_Msg, 0, new QTableWidgetItem(MsgID_CString));

            // --- 解析日期 (逻辑同上) ---
            const QDate baseDate1984(1984, 1, 1);
            Temp_Date_UCHAR = MsgList_h_ValidData[7 + 8 * kk];
            Temp_Date_short = MsgList_h_ValidData[8 + 8 * kk];
            Temp_Date_short = (Temp_Date_short << 8) + Temp_Date_UCHAR;
            Date_CString = getDateStringFromElapsedDays(baseDate1984, Temp_Date_short + 1); // 调用辅助函数，计算从基准日期过了 Temp_Date_short + 1 天后的日期
            ui->tableWidget_2->setItem(nListCount_Msg, 1, new QTableWidgetItem(Date_CString));

            // --- 解析时间 (逻辑同上) ---
            Temp_UCHAR = MsgList_h_ValidData[11 + 8 * kk];
            Temp_short = MsgList_h_ValidData[12 + 8 * kk];
            Temp_MS = (static_cast<uint>(Temp_short) << 8) + Temp_UCHAR;
            Temp_MS <<= 16;
            Temp_UCHAR = MsgList_h_ValidData[9 + 8 * kk];
            Temp_short = MsgList_h_ValidData[10 + 8 * kk];
            Temp_MS += (static_cast<uint>(Temp_short) << 8) + Temp_UCHAR;
            Time_CString = formatEventTime(Temp_MS, ms_UINT);
            ms_CString = QString::number(ms_UINT);
            ui->tableWidget_2->setItem(nListCount_Msg, 2, new QTableWidgetItem(Time_CString));
            ui->tableWidget_2->setItem(nListCount_Msg, 3, new QTableWidgetItem(ms_CString));

            // --- 解析 Event (事件状态) ---
            // 如果原始MsgID大于32767（即最高位为1），表示发生/置位，记为"+"
            Event_CString = (MsgID > 32767) ? "+" : "-";
            ui->tableWidget_2->setItem(nListCount_Msg, 4, new QTableWidgetItem(Event_CString));

            // nListCount_Msg++;
        }
    }
}

/**
 * @brief 计算从一个指定的起始日期开始，经过若干天后的日期。
 * @param startDate 起始日期
 * @param daysElapsed 从起始日期开始所经过的天数
 * @return 格式为 "yyyy-MM-dd" 的日期字符串
 */
QString Dlg_ZJM::getDateStringFromElapsedDays(const QDate &startDate, int daysElapsed)
{
    // 使用 QDate::addDays() 方法，它会自动处理所有复杂的日期和闰年逻辑
    QDate targetDate = startDate.addDays(daysElapsed);

    // 将计算出的日期转换为标准格式的字符串
    return targetDate.toString("yyyy-MM-dd");
}

/**
 * @brief 将一天内的总毫秒数转换为 "HH:MM:SS" 格式的字符串
 * @param msSinceMidnight 从零点开始的总毫秒数
 * @param milliseconds [out] 用于接收剩余的毫秒部分
 * @return 格式化后的时间字符串
 */
QString Dlg_ZJM::formatEventTime(unsigned int msSinceMidnight, unsigned int &milliseconds)
{
    uint totalSeconds = msSinceMidnight / 1000;
    milliseconds = msSinceMidnight % 1000;

    uchar hours = (totalSeconds / 3600) % 24;
    uchar minutes = (totalSeconds % 3600) / 60;
    uchar seconds = totalSeconds % 60;

    return QString("%1:%2:%3").arg(hours, 2, 10, QChar('0')).arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
}

bool dataReceived = false; // 一个全局布尔变量，似乎未在当前代码片段中使用
/**
 * @brief “获取信息”按钮的点击事件处理槽函数。
 * @details 此函数负责从硬件设备获取最新的数据。
 * 它首先初始化通信状态和各种数据请求报文结构体，
 * 然后通过父窗口的 Fun_Connect 函数发送第一个数据请求（AD计算值回读）。
 * 最后，它启动一个一次性定时器，在短暂延迟后调用 updateData() 函数来刷新界面显示。
 */
void Dlg_ZJM::OnButtonZjmFetch()
{
    uchar TX[280];
    TX[0] = 0XA2;
    TX[1] = '\0';

    new1Dlg *pParent = (new1Dlg *)parent(); // 获取父窗口 (new1Dlg) 的指针，以便访问其通信功能和状态变量
    if (!pParent)
    {
        QMessageBox::warning(this, u8"错误", u8"无法获取new1Dlg父窗口");
    }
    // --- 初始化通信状态 ---
    pParent->Connect_Index = 0;  // 重置通信步骤索引
    pParent->ErrorCounter = 0;   // 重置错误计数器
    pParent->Down = 1005;        // 设置状态标志为 1005，表示正在执行 "Fetch" 操作
    pParent->RX_Msg_End = false; // 标记消息接收未完成
    pParent->Msg_Len = 0;        // 重置已接收消息的长度

    // updateData();
    // 检查 FETCH_REFRESH 标志，防止在一次获取过程中重复发送请求
    if (FETCH_REFRESH == 0)
    {
        // 初始化TQCS_block_35 (同期参数)
        pParent->TQCS_block_35.Telegram_Nr = 35;
        pParent->TQCS_block_35.Station_adr = 0;
        pParent->TQCS_block_35.F1_field = 0;
        pParent->TQCS_block_35.F2_field = 0x80;
        pParent->TQCS_block_35.Data = (uchar *)"";

        // 初始化ACAD_block_33 (交流AD参数回读)
        pParent->ACAD_block_33.Telegram_Nr = 33;
        pParent->ACAD_block_33.Station_adr = 0;
        pParent->ACAD_block_33.F1_field = 0;
        pParent->ACAD_block_33.F2_field = 0x80;
        pParent->ACAD_block_33.Data = (uchar *)"";
        //	ACAD_block_33.Data[0]='\0';

        // 初始化ADData_block_37 (主页面AD计算值回读)
        pParent->ADData_block_37.Telegram_Nr = 37;
        pParent->ADData_block_37.Station_adr = 0;
        pParent->ADData_block_37.F1_field = 0;
        pParent->ADData_block_37.F2_field = 0x80;
        pParent->ADData_block_37.Data = (uchar *)"";

        // 初始化Message_block_49 (通信报文 message回读)
        pParent->Message_block_49.Telegram_Nr = 49;
        pParent->Message_block_49.Station_adr = 0;
        pParent->Message_block_49.F1_field = 0;
        pParent->Message_block_49.F2_field = 0x80;
        pParent->Message_block_49.Data = (uchar *)"";

        // 初始化AC_angle_block_47 (角度矢量 交流AD角度回读)
        pParent->AC_angle_block_47.Telegram_Nr = 47; // 角度矢量 交流AD角度回读
        pParent->AC_angle_block_47.Station_adr = 0;
        pParent->AC_angle_block_47.F1_field = 0;
        pParent->AC_angle_block_47.F2_field = 0x80;
        pParent->AC_angle_block_47.Data = (uchar *)"";

        // 初始化Msg_block_13 (PC 请求告警信息，无head)
        pParent->Msg_block_13.Telegram_Nr = 13; // PC 请求告警信息（无head）
        pParent->Msg_block_13.Station_adr = 0;
        pParent->Msg_block_13.F1_field = 0;
        pParent->Msg_block_13.F2_field = 0x80;
        pParent->Msg_block_13.Data = (uchar *)"";

        // 重复初始化Msg_block_13，可能是代码冗余
        pParent->Msg_block_13.Telegram_Nr = 13; // PC 请求告警信息（无head）
        pParent->Msg_block_13.Station_adr = 0;
        pParent->Msg_block_13.F1_field = 0;
        pParent->Msg_block_13.F2_field = 0x80;
        pParent->Msg_block_13.Data = (uchar *)"";

        // 清空用于打包数据的数组
        pParent->Connect_Telegram.ValidDataToPack[0] = 0;
        pParent->Connect_Telegram.ValidDataToPack[1] = 0;
        pParent->Connect_Telegram.ValidDataToPack[2] = 0;
        pParent->Connect_Telegram.ValidDataToPack[3] = 0;
        pParent->Connect_Telegram.ValidDataToPack[4] = 0;
        pParent->Connect_Telegram.ValidDataToPack[5] = 0;
        pParent->Connect_Telegram.ValidDataToPack[6] = '\0';

        // 发送第一个数据请求：获取AD计算值 (报文号37)
        pParent->Fun_Connect(pParent->ADData_block_37, 0);
        // pParent->Fun_Connect(pParent->Msg_block_13,0);
        // FETCH_REFRESH=1;
        // fetchTimer = new QTimer(this);//定时器
        // connect(fetchTimer, &QTimer::timeout, this, &Dlg_ZJM::onFetchTimer);
        // fetchTimer->start(1999);

        // 启动一个999毫秒的一次性定时器。
        // 定时器到期后，将执行 lambda 表达式内的代码，即调用 updateData() 来刷新界面。
        // 这种异步方式可以给设备足够的时间来响应和发送数据。
        QTimer::singleShot(999, this, [this]() {
            updateData(); // 准备处理接收到的设备数据并更新UI
        });
    }
}

/*
void Dlg_ZJM::onFetchTimer()
{

    fetchTimer->stop(); // Equivalent of KillTimer
    FETCH_REFRESH = 0;
    QMessageBox::warning(this, u8"错误", u8"获取参数超时");
}
*/

/**
 * @brief “保存信息”按钮的点击事件处理槽函数。
 * @details 此函数将当前AD参数（从AD1_dlg.DATA中获取）保存到一个二进制文件中。
 * 它会弹出一个文件保存对话框，让用户选择保存位置和文件名。
 */
void Dlg_ZJM::OnButtonZjmSave() // 保存信息
{
    // TODO: Add your control notification handler code here
    uchar *Save_Data;
    Save_Data = (uchar *)malloc(93); // 分配93字节的内存来存储要保存的数据
    for (int tt = 0; tt < 93; tt++)  // 循环将 AD1 对话框中的数据复制到 Save_Data 缓冲区
    {
        // if(tt<59)
        *(Save_Data + tt) = AD1_dlg.DATA[tt];
        // else
        //  *(Save_Data+tt)=TQCS_dlg.DATA[tt-59];
    }

    QString fileName = QFileDialog::getSaveFileName(this,              // 父窗口
        tr(u8"参数保存为"),                                            // 对话框标题
        QString(),                                                     // 初始目录（空表示默认）
        tr("Binary File (*.bin);;Text File (*.txt);;All Files (*.*)"), // 过滤器
        nullptr,                                                       // 默认选择的过滤器
        QFileDialog::Options()                                         // 选项
    );

    // 如果用户选择了文件名（而不是取消）
    if (!fileName.isEmpty())
    {
        // 如果用户没有输入扩展名，自动添加.bin
        if (QFileInfo(fileName).suffix().isEmpty())
        {
            fileName += ".bin";
        }

        // 创建QFile对象并尝试以只写模式打开
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly))
        {
            file.write((const char *)Save_Data, 93); // 写入数据
            file.close();
        }
        else
        {
            // 如果文件打开失败，弹出警告
            QMessageBox::warning(this, tr("错误"), tr("无法保存文件"));
        }
    }
}

/**
 * @brief “打开文件”按钮的点击事件处理槽函数。
 * @details 此函数从一个二进制文件中加载参数。
 * 它会弹出一个文件打开对话框，让用户选择要加载的文件，
 * 然后读取文件内容并将其填充到 AD1_dlg 的数据区。
 */
void Dlg_ZJM::OnButtonZjmOpen()
{
    // 弹出文件打开对话框
    QString fileName = QFileDialog::getOpenFileName(this, tr(u8"打开参数为"), QString(), tr("Binary File (*.bin);;Text File (*.txt);;All Files (*.*)"));
    // 如果用户选择了文件
    if (!fileName.isEmpty())
    {
        // 创建QFile对象并尝试以只读模式打开
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly))
        {
            // 读取文件数据
            QByteArray fileData = file.readAll();
            file.close();

            // 检查文件大小是否足够
            if (fileData.size() < 93)
            {
                QMessageBox::warning(this, tr("错误"), tr("文件大小不足93字节"));
                return;
            }

            // 将读取的数据加载到 AD1 对话框的相应数据结构中
            for (int tt = 0; tt < 93; tt++)
            {
                // if (tt < 59)
                //{
                AD1_dlg.AD1_FUNCTION = 1126;
                AD1_dlg.ValidData[4 + tt] = fileData[tt]; // 将文件数据复制到 ValidData 数组中（从第4个字节开始）
                // }
                // else
                // {
                // TQCS_dlg.TQCS_FUNCTION = 1126;
                // TQCS_dlg.ValidData[tt - 55] = fileData[tt];
                // }
            }
        }
        else
        {
            // 如果文件打开失败，弹出警告
            QMessageBox::warning(this, tr("错误"), tr("无法打开文件"));
        }
    }
}

/**
 * @brief 窗口关闭事件处理器。
 * @param event 包含了关闭事件的详细信息。
 * @details 这是一个重写的虚函数，当用户尝试关闭窗口时被Qt框架自动调用。
 * 它会发射一个 `windowClosed()` 信号，通知父窗口本窗口已关闭，
 * 然后调用 `event->accept()` 来确认关闭操作。
 */
void Dlg_ZJM::closeEvent(QCloseEvent *event) // 关联关闭窗口发射信号
{
    emit windowClosed(); // 发射信号
    event->accept();     // 接受关闭事件
}

/**
 * @brief “退出”按钮的点击事件处理槽函数。
 * @details 当用户点击“退出”按钮时，调用此函数来关闭当前窗口。
 */
void Dlg_ZJM::OnButtonZjmExit()
{
    this->close();
}

/**
 * @brief [简化] 打开 AD参数 对话框
 */
void Dlg_ZJM::on_IDC_ZJM_AD1_clicked()
{
    showDialog<Dlg_AD1>(this);
}

/**
 * @brief [简化] 打开 通道校准 对话框
 */
void Dlg_ZJM::on_IDC_ZJM_ADAdjust_clicked()
{
    showDialog<Dlg_ADAdjust1>(this);
}

/**
 * @brief [简化] 打开 时间设置 对话框
 */
void Dlg_ZJM::on_IDC_ZJM_TimeSetBtn_clicked()
{
    showDialog<Dlg_TimeSet>(this);
}

/**
 * @brief [简化] 打开 通信报文 对话框
 */
void Dlg_ZJM::on_IDC_ZJM_Message_clicked()
{
    showDialog<Dlg_Message>(this);
}

/**
 * @brief [简化] 打开 角度矢量 对话框
 */
void Dlg_ZJM::on_IDC_ZJM_Angle_clicked()
{
    showDialog<Dlg_BaseAngle>(this);
}

/**
 * @brief [简化] 打开 标幺设置 对话框
 */
void Dlg_ZJM::on_IDC_VIEW_Set_clicked()
{
    showDialog<DLg_DIMset>(this);
}

/**
 * @brief [简化] 打开 同期参数 对话框
 */
void Dlg_ZJM::on_IDC_ZJM_TQCSBtn_clicked()
{
    showDialog<Dlg_TQCS1>(this);
}

/**
 * @brief [简化] 打开 同期命令 对话框
 */
void Dlg_ZJM::on_IDC_ZJM_TQMLBtn_clicked()
{
    showDialog<Dlg_TQML>(this);
}

/**
 * @brief [简化] 打开 通信设置 对话框
 */
void Dlg_ZJM::on_IDC_ZJM_ComSetBtn_clicked()
{
    showDialog<Dlg_ComSet>(this);
}
