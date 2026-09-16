#include "Dlg_TQML.h"
#include "ui_Dlg_TQML.h"
#include "Dlg_ZJM.h"
#include "new1Dlg.h"
#include <QMessageBox>
#include <QDebug>
#include <QScreen>
Dlg_TQML::Dlg_TQML(QWidget *parent) : QWidget(parent), ui(new Ui::Dlg_TQML)
{
    ui->setupUi(this);

    // --- UI 字体和DPI缩放设置 ---
    qreal pixelRatio = qApp->devicePixelRatio();
    qreal dpiRatio = QGuiApplication::primaryScreen()->logicalDotsPerInch() / 96.0; // 获取逻辑DPI缩放比（考虑系统字体缩放）
    qreal scaleFactor = qMin(pixelRatio, dpiRatio);                                 // 综合缩放因子
    QFont font = this->font();                                                      // 获取基准字体
    font.setPixelSize(qRound(12 * scaleFactor));                                    // 基于DPI缩放
    this->setFont(font);

    setWindowTitle(u8"同期命令设置"); // 设置窗口标题

    // --- UI 控件初始状态设置 ---
    // 将显示结果的复选框设为只读
    ui->checkBox_Rec_OK->setEnabled(false);
    ui->checkBox_Sent_OK->setEnabled(false);
    ui->checkBox_Time_OK->setEnabled(false);
    ui->checkBox_Unfinish->setEnabled(false);
    ui->checkBox_Reset->setEnabled(false);

    // --- 信号槽连接 ---
    m_timer = new QTimer(this); // 初始化定时器
    connect(m_timer, &QTimer::timeout, this, &Dlg_TQML::onTimerTimeout);
    connect(ui->pushButton_Apply, &QPushButton::clicked, this, &Dlg_TQML::onButtonApplyClicked);
    connect(ui->pushButton_Fetch, &QPushButton::clicked, this, &Dlg_TQML::onButtonFetchClicked);
    connect(ui->radioButton_Rec, &QRadioButton::clicked, this, &Dlg_TQML::onRadioRecChecked);
    connect(ui->radioButton_Sent, &QRadioButton::clicked, this, &Dlg_TQML::onRadioSentChecked);
    connect(ui->radioButton_Time, &QRadioButton::clicked, this, &Dlg_TQML::onRadioTimeChecked);

    // 初始化数据
    initData();
}

Dlg_TQML::~Dlg_TQML()
{
    delete ui;
}

/**
 * @brief [新增] 从原始字节数组中解析出日期和时间。
 * @details 采用 Qt 内置函数进行计算，替代了原有的复杂手动循环，代码更健壮、简洁。
 */
QDateTime Dlg_TQML::parseDateTimeFromData(const uchar *data, int dayOffset, int msOffset)
{
    // 1. 解析天数 (自 1984-01-01 起)
    // 从 data 数组中提取2个字节(小端模式)组成一个16位的天数
    unsigned short days = data[dayOffset] | (data[dayOffset + 1] << 8);

    // 2. 解析毫秒数 (自当天 00:00:00.000 起)
    // 从 data 数组中提取4个字节(小端模式)组成一个32位的毫秒数
    uint ms = data[msOffset] | (data[msOffset + 1] << 8) | (data[msOffset + 2] << 16) | (data[msOffset + 3] << 24);

    // 3. 计算最终日期和时间
    QDate date = QDate(1984, 1, 1).addDays(days);
    QTime time = QTime::fromMSecsSinceStartOfDay(ms % 86400000); // 对一天总毫秒数取模，防止溢出

    return QDateTime(date, time);
}

/**
 * @brief [新增] 重置所有数据显示字段和状态标志。
 */
void Dlg_TQML::resetDataFields()
{
    m_DateTimeTQML_REC_DATE = QDateTime(QDate(1000, 10, 10), QTime(10, 10, 10));
    m_DateTimeTQML_SENT_DATE = QDateTime(QDate(1000, 10, 10), QTime(10, 10, 10));
    m_DateTimeTQML_TIME_DATE = QDateTime(QDate(1000, 10, 10), QTime(10, 10, 10));
    m_BOOLTQML_REC_OK = false;
    m_BOOLTQML_SENT_OK = false;
    m_BOOLTQML_TIME_OK = false;
    m_BOOLTQML_UNFINISH = false;
    m_BOOLTQML_RESET = false;

    m_floatTQML_REC_DA = 0.0f;
    m_floatTQML_REC_DF = 0.0f;
    m_floatTQML_REC_DU = 0.0f;
    m_floatTQML_REC_F1 = 0.0f;
    m_floatTQML_REC_F2 = 0.0f;
    m_floatTQML_REC_U1 = 0.0f;
    m_floatTQML_REC_U2 = 0.0f;
    m_floatTQML_REC_DF_DT = 0.0f;
    m_floatTQML_REC_F1_DT = 0.0f;
    m_floatTQML_REC_F2_DT = 0.0f;

    m_floatTQML_SENT_DA = 0.0f;
    m_floatTQML_SENT_DF = 0.0f;
    m_floatTQML_SENT_DU = 0.0f;
    m_floatTQML_SENT_F1 = 0.0f;
    m_floatTQML_SENT_F2 = 0.0f;
    m_floatTQML_SENT_U1 = 0.0f;
    m_floatTQML_SENT_U2 = 0.0f;
    m_floatTQML_SENT_DF_DT = 0.0f;
    m_floatTQML_SENT_F1_DT = 0.0f;
    m_floatTQML_SENT_F2_DT = 0.0f;

    m_floatTQML_TIME_DA = 0.0f;
    m_floatTQML_TIME_DF = 0.0f;
    m_floatTQML_TIME_DU = 0.0f;
    m_floatTQML_TIME_F1 = 0.0f;
    m_floatTQML_TIME_F2 = 0.0f;
    m_floatTQML_TIME_U1 = 0.0f;
    m_floatTQML_TIME_U2 = 0.0f;
    m_floatTQML_TIME_DF_DT = 0.0f;
    m_floatTQML_TIME_F1_DT = 0.0f;
    m_floatTQML_TIME_F2_DT = 0.0f;

    m_nTQML_REC_MS = 0;
    m_nTQML_SENT_MS = 0;
    m_nTQML_TIME_MS = 0;
}

/**
 * @brief 初始化窗口数据。
 */
void Dlg_TQML::initData()
{
    // 调用辅助函数清空所有数据字段
    resetDataFields();

    // 设置初始状态
    m_nTQML_REC = 0; // 默认选中 "Rec"
    Function_ID = 0;

    updateUI();
}

/**
 * @brief 用当前成员变量的值更新UI界面显示。
 */
void Dlg_TQML::updateUI()
{
    // 更新单选按钮状态
    ui->radioButton_Rec->setChecked(m_nTQML_REC == 0);
    ui->radioButton_Sent->setChecked(m_nTQML_REC == 1);
    ui->radioButton_Time->setChecked(m_nTQML_REC == 2);

    // 更新日期时间显示
    ui->dateTimeEdit_Rec->setDateTime(m_DateTimeTQML_REC_DATE);
    ui->dateTimeEdit_Sent->setDateTime(m_DateTimeTQML_SENT_DATE);
    ui->dateTimeEdit_Time->setDateTime(m_DateTimeTQML_TIME_DATE);

    // 更新状态复选框
    ui->checkBox_Rec_OK->setChecked(m_BOOLTQML_REC_OK);
    ui->checkBox_Sent_OK->setChecked(m_BOOLTQML_SENT_OK);
    ui->checkBox_Time_OK->setChecked(m_BOOLTQML_TIME_OK);
    ui->checkBox_Unfinish->setChecked(m_BOOLTQML_UNFINISH);
    ui->checkBox_Reset->setChecked(m_BOOLTQML_RESET);

    // --- 更新 "接收" 分组框中的所有 LineEdit ---
    ui->lineEdit_Rec_F1->setText(QString::number(m_floatTQML_REC_F1, 'f', 3));
    ui->lineEdit_Rec_F2->setText(QString::number(m_floatTQML_REC_F2, 'f', 3));
    ui->lineEdit_Rec_DF->setText(QString::number(m_floatTQML_REC_DF, 'f', 3));
    ui->lineEdit_Rec_DA->setText(QString::number(m_floatTQML_REC_DA, 'f', 2));
    ui->lineEdit_Rec_DU->setText(QString::number(m_floatTQML_REC_DU, 'f', 2));
    ui->lineEdit_Rec_U1->setText(QString::number(m_floatTQML_REC_U1, 'f', 2));
    ui->lineEdit_Rec_U2->setText(QString::number(m_floatTQML_REC_U2, 'f', 2));
    ui->lineEdit_Rec_F1_DT->setText(QString::number(m_floatTQML_REC_F1_DT, 'f', 2));
    ui->lineEdit_Rec_F2_DT->setText(QString::number(m_floatTQML_REC_F2_DT, 'f', 2));
    ui->lineEdit_Rec_DF_DT->setText(QString::number(m_floatTQML_REC_DF_DT, 'f', 2));
    ui->lineEdit_Rec_MS->setText(QString::number(m_nTQML_REC_MS));

    // --- 更新 "发送" 分组框中的所有 LineEdit ---
    ui->lineEdit_Sent_F1->setText(QString::number(m_floatTQML_SENT_F1, 'f', 3));
    ui->lineEdit_Sent_F2->setText(QString::number(m_floatTQML_SENT_F2, 'f', 3));
    ui->lineEdit_Sent_DF->setText(QString::number(m_floatTQML_SENT_DF, 'f', 3));
    ui->lineEdit_Sent_DA->setText(QString::number(m_floatTQML_SENT_DA, 'f', 2));
    ui->lineEdit_Sent_DU->setText(QString::number(m_floatTQML_SENT_DU, 'f', 2));
    ui->lineEdit_Sent_U1->setText(QString::number(m_floatTQML_SENT_U1, 'f', 2));
    ui->lineEdit_Sent_U2->setText(QString::number(m_floatTQML_SENT_U2, 'f', 2));
    ui->lineEdit_Sent_F1_DT->setText(QString::number(m_floatTQML_SENT_F1_DT, 'f', 2));
    ui->lineEdit_Sent_F2_DT->setText(QString::number(m_floatTQML_SENT_F2_DT, 'f', 2));
    ui->lineEdit_Sent_DF_DT->setText(QString::number(m_floatTQML_SENT_DF_DT, 'f', 2));
    ui->lineEdit_Sent_MS->setText(QString::number(m_nTQML_SENT_MS));

    // --- 更新 "时间" 分组框中的所有 LineEdit ---
    ui->lineEdit_Time_F1->setText(QString::number(m_floatTQML_TIME_F1, 'f', 3));
    ui->lineEdit_Time_F2->setText(QString::number(m_floatTQML_TIME_F2, 'f', 3));
    ui->lineEdit_Time_DF->setText(QString::number(m_floatTQML_TIME_DF, 'f', 3));
    ui->lineEdit_Time_DA->setText(QString::number(m_floatTQML_TIME_DA, 'f', 2));
    ui->lineEdit_Time_DU->setText(QString::number(m_floatTQML_TIME_DU, 'f', 2));
    ui->lineEdit_Time_U1->setText(QString::number(m_floatTQML_TIME_U1, 'f', 2));
    ui->lineEdit_Time_U2->setText(QString::number(m_floatTQML_TIME_U2, 'f', 2));
    ui->lineEdit_Time_F1_DT->setText(QString::number(m_floatTQML_TIME_F1_DT, 'f', 2));
    ui->lineEdit_Time_F2_DT->setText(QString::number(m_floatTQML_TIME_F2_DT, 'f', 2));
    ui->lineEdit_Time_DF_DT->setText(QString::number(m_floatTQML_TIME_DF_DT, 'f', 2));
    ui->lineEdit_Time_MS->setText(QString::number(m_nTQML_TIME_MS));
}

/**
 * @brief [重构] 解析从设备接收到的 ValidData 原始数据。
 */
void Dlg_TQML::parseData()
{
    // 仅当操作为 "获取" (ID=1032) 时才执行解析
    if (Function_ID != 1032)
        return;

    // --- 解析公共数据部分 (电压、频率等) ---
    // 辅助函数，用于从字节数组中提取一个16位无符号整数并转换为浮点数
    auto getFloat = [&](int offset, float divisor) {
        unsigned short val = ValidData[offset] | (ValidData[offset + 1] << 8);
        return static_cast<float>(val) / divisor;
    };

    // 偏移量  大小   含义             换算
    // 11      2     U1            / 100
    // 13      2     U2            / 100
    // 15      2     F1            / 1000
    // 17      2     F2            / 1000
    // 19      2     F1/dt         / 100
    // 21      2     F2/dt         / 100
    // 23      2     DU            / 100
    // 25      2     DF            / 1000
    // 27      2     DF/dt         / 100
    // 29      2     DA            / 100
    float u1 = getFloat(11, 100.0f);
    float u2 = getFloat(13, 100.0f);
    float f1 = getFloat(15, 1000.0f);
    float f2 = getFloat(17, 1000.0f);
    float f1dt = getFloat(19, 100.0f);
    float f2dt = getFloat(21, 100.0f);
    float DU = getFloat(23, 100.0f);
    float Df = getFloat(25, 1000.0f);
    float Dfdt = getFloat(27, 100.0f);
    float Da = getFloat(29, 100.0f);

    // --- [重构] 解析日期和时间 ---
    // 偏移量   大小   含义
    // 5       2     天数 (自1984.1.1)
    // 7       4     毫秒数 (自当天0点)
    QDateTime dateTime = parseDateTimeFromData(ValidData, 5, 7);

    // --- 根据记录类型 (ValidData[4]) 更新对应的数据和状态 ---
    int recordType = ValidData[4];
    if (recordType == 1) // 接收(Rec)数据
    {
        m_DateTimeTQML_REC_DATE = dateTime;
        m_nTQML_REC = 0;
        m_nTQML_REC_MS = dateTime.time().msec();
        m_floatTQML_REC_F1 = f1;
        m_floatTQML_REC_F2 = f2;
        m_floatTQML_REC_DF = Df;
        m_floatTQML_REC_DA = Da;
        m_floatTQML_REC_DU = DU;
        m_floatTQML_REC_U1 = u1;
        m_floatTQML_REC_U2 = u2;
        m_floatTQML_REC_F1_DT = f1dt;
        m_floatTQML_REC_F2_DT = f2dt;
        m_floatTQML_REC_DF_DT = Dfdt;

        m_BOOLTQML_UNFINISH = false;
        m_BOOLTQML_RESET = false;
        m_BOOLTQML_REC_OK = true;
    }
    else if (recordType == 2) // 发送(Sent)数据
    {
        m_DateTimeTQML_SENT_DATE = dateTime;
        m_nTQML_REC = 1;
        m_nTQML_SENT_MS = dateTime.time().msec();
        m_floatTQML_SENT_F1 = f1;
        m_floatTQML_SENT_F2 = f2;
        m_floatTQML_SENT_DF = Df;
        m_floatTQML_SENT_DA = Da;
        m_floatTQML_SENT_DU = DU;
        m_floatTQML_SENT_U1 = u1;
        m_floatTQML_SENT_U2 = u2;
        m_floatTQML_SENT_F1_DT = f1dt;
        m_floatTQML_SENT_F2_DT = f2dt;
        m_floatTQML_SENT_DF_DT = Dfdt;

        m_BOOLTQML_UNFINISH = false;
        m_BOOLTQML_RESET = false;
        m_BOOLTQML_SENT_OK = true;
    }
    else if (recordType == 3) // 时间(Time)数据
    {
        m_DateTimeTQML_TIME_DATE = dateTime;
        m_nTQML_REC = 2;
        m_nTQML_TIME_MS = dateTime.time().msec();
        m_floatTQML_TIME_F1 = f1;
        m_floatTQML_TIME_F2 = f2;
        m_floatTQML_TIME_DF = Df;
        m_floatTQML_TIME_DA = Da;
        m_floatTQML_TIME_DU = DU;
        m_floatTQML_TIME_U1 = u1;
        m_floatTQML_TIME_U2 = u2;
        m_floatTQML_TIME_F1_DT = f1dt;
        m_floatTQML_TIME_F2_DT = f2dt;
        m_floatTQML_TIME_DF_DT = Dfdt;

        m_BOOLTQML_UNFINISH = false;
        m_BOOLTQML_RESET = false;
        m_BOOLTQML_TIME_OK = true;
    }
    else if (recordType == 4) // 复位(Reset)
    {
        m_BOOLTQML_RESET = true;
        m_BOOLTQML_UNFINISH = false;
        resetDataFields(); // 调用辅助函数清空数据
    }
    else if (recordType == 5) // 未完成(Unfinish)
    {
        m_BOOLTQML_UNFINISH = true;
        m_BOOLTQML_RESET = false;
        resetDataFields(); // 调用辅助函数清空数据
    }

    updateUI(); // 刷新界面
}

/**
 * @brief “应用”按钮点击槽函数。发送复位命令。
 */
void Dlg_TQML::onButtonApplyClicked()
{
    Function_ID = 1031; // 设置操作ID为 "Apply"

    // --- 获取祖父窗口(new1Dlg)指针，用于调用通信函数 ---
    // 这是一个强依赖关系：Dlg_TQML -> Dlg_ZJM -> new1Dlg
    Dlg_ZJM *pParent_ZJM = qobject_cast<Dlg_ZJM *>(parent());
    if (!pParent_ZJM)
    {
        QMessageBox::warning(this, "错误", "无法获取父窗口 Dlg_ZJM");
        return;
    }
    new1Dlg *pParent = qobject_cast<new1Dlg *>(pParent_ZJM->parent());
    if (!pParent)
    {
        QMessageBox::warning(this, "错误", "无法获取祖父窗口 new1Dlg");
        return;
    }

    // --- 发送命令前，重置本地状态和UI ---
    pParent->Connect_Index = 0;
    pParent->ErrorCounter = 0;
    pParent->Down = 1031; // 设置通信状态标志
    resetDataFields();    // 调用辅助函数清空本地数据

    // --- 准备并发送通信报文 ---
    TeleBlock TQML_block_44;
    TQML_block_44.Station_adr = 0;
    TQML_block_44.Telegram_Nr = 44; // 报文号
    TQML_block_44.F1_field = 0;
    TQML_block_44.F2_field = 0x80;
    pParent->Connect_Telegram.ValidDataToPack[0] = 0x55; // 命令数据
    pParent->Connect_Telegram.ValidDataToPack[1] = 0xaa;
    pParent->Connect_Telegram.ValidDataToPack[2] = 0;
    pParent->Connect_Telegram.PACK_SELECT = true;
    pParent->Fun_Connect(TQML_block_44, 0);

    updateUI();
}

/**
 * @brief “获取”按钮点击槽函数。根据单选按钮选择的类型，向设备请求数据。
 */
void Dlg_TQML::onButtonFetchClicked()
{
    Function_ID = 1032; // 设置操作ID为 "Fetch"

    // --- 获取祖父窗口(new1Dlg)指针 ---
    Dlg_ZJM *pParent_ZJM = qobject_cast<Dlg_ZJM *>(parent());
    if (!pParent_ZJM)
    {
        QMessageBox::warning(this, "错误", "无法获取父窗口 Dlg_ZJM");
        return;
    }
    new1Dlg *pParent = qobject_cast<new1Dlg *>(pParent_ZJM->parent());
    if (!pParent)
    {
        QMessageBox::warning(this, "错误", "无法获取祖父窗口 new1Dlg");
        return;
    }

    // --- 初始化通信状态 ---
    pParent->Connect_Index = 0;
    pParent->ErrorCounter = 0;
    pParent->Down = 1032; // 设置通信状态标志

    // --- 准备并发送通信报文 ---
    TeleBlock TQML_block_45;
    TQML_block_45.Station_adr = 0;
    TQML_block_45.Telegram_Nr = 45; // 报文号
    TQML_block_45.F1_field = 0;
    TQML_block_45.F2_field = 0x80;
    // 请求的数据类型: 1(Rec), 2(Sent), 3(Time)，由单选按钮决定
    pParent->Connect_Telegram.ValidDataToPack[0] = (uchar)(m_nTQML_REC + 1);
    pParent->Connect_Telegram.ValidDataToPack[1] = 0;
    pParent->Connect_Telegram.PACK_SELECT = true;
    pParent->Fun_Connect(TQML_block_45, 0);

    // 启动定时器，延迟一段时间后解析返回的数据
    m_timer->start(666);
}

/**
 * @brief "Rec" 单选按钮被选中。
 */
void Dlg_TQML::onRadioRecChecked()
{
    m_nTQML_REC = 0;
}

/**
 * @brief "Sent" 单选按钮被选中。
 */
void Dlg_TQML::onRadioSentChecked()
{
    m_nTQML_REC = 1;
}

/**
 * @brief "Time" 单选按钮被选中。
 */
void Dlg_TQML::onRadioTimeChecked()
{
    m_nTQML_REC = 2;
}

/**
 * @brief 窗口显示事件处理。
 */
void Dlg_TQML::onShowWindow(bool visible)
{
    if (visible)
    {
        // 每次显示窗口时，都初始化数据
        initData();
    }
}

/**
 * @brief 定时器超时槽函数。
 */
void Dlg_TQML::onTimerTimeout()
{
    m_timer->stop(); // 停止定时器
    parseData();     // 解析数据
    updateUI();      // 更新UI
}

/**
 * @brief “取消”按钮点击槽函数。
 */
void Dlg_TQML::on_pushButton_Cancel_clicked()
{
    this->close(); // 直接关闭窗口
}
