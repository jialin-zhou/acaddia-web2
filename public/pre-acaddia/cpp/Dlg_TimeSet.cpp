#include "Dlg_TimeSet.h"
#include "ui_Dlg_TimeSet.h"
#include "Dlg_ZJM.h"
#include "new1Dlg.h"
#include <QMessageBox>
#include <QDebug>
#include <QScreen>

Dlg_TimeSet::Dlg_TimeSet(QWidget *parent)
    // clang-format off
    : QWidget(parent)
    , ui(new Ui::Dlg_TimeSet)
    , m_timer(new QTimer(this))
    , m_autoUpdateEnabled(true) // 默认开启自动更新
    , Function_ID(0)
    , m_isUserModified(false)
// clang-format on
{
    ui->setupUi(this);

    // --- UI 字体和DPI缩放设置 ---
    qreal pixelRatio = qApp->devicePixelRatio();
    qreal dpiRatio = QGuiApplication::primaryScreen()->logicalDotsPerInch() / 96.0;
    qreal scaleFactor = qMin(pixelRatio, dpiRatio);
    QFont font = this->font();
    font.setPixelSize(qRound(12 * scaleFactor));
    this->setFont(font);
    this->setWindowTitle(u8"时间设置");

    // --- 初始化控件和成员变量 ---
    // 设置时间控件显示格式，包含秒
    ui->timeEdit->setDisplayFormat("HH:mm:ss");

    // 将当前系统时间作为初始值
    m_currentDateTime = QDateTime::currentDateTime();
    syncUIToDateTime(m_currentDateTime);

    // 设置定时器，每秒触发一次
    m_timer->setInterval(1000);
    m_timer->start(); // 默认启动定时器

    // 设置复选框初始状态为未选中，对应 "自动更新" 状态
    // 根据业务逻辑，未选中 = 自动更新，选中 = 停止自动更新（手动模式）
    ui->checkBox->setChecked(false);

    // 统一管理所有信号槽连接
    setupConnections();
}

Dlg_TimeSet::~Dlg_TimeSet()
{
    // 在析构时确保定时器已停止
    if (m_timer->isActive())
    {
        m_timer->stop();
    }
    delete ui;
}

/**
 * @brief 统一设置所有信号槽连接。
 */
void Dlg_TimeSet::setupConnections()
{
    // 定时器超时，自动更新时间显示
    connect(m_timer, &QTimer::timeout, this, &Dlg_TimeSet::onTimer);

    // 界面按钮点击事件
    connect(ui->SetTimeButton, &QPushButton::clicked, this, &Dlg_TimeSet::onSetTimeButtonClicked);
    connect(ui->GetTimeButton, &QPushButton::clicked, this, &Dlg_TimeSet::onGetTimeButtonClicked);
    connect(ui->ExitButton, &QPushButton::clicked, this, &Dlg_TimeSet::onExitButtonClicked);

    // 复选框状态改变事件
    connect(ui->checkBox, &QCheckBox::stateChanged, this, &Dlg_TimeSet::onAutoUpdateCheckBoxChanged);

    // 日期和时间控件被用户手动修改的事件
    connect(ui->dateEdit, &QDateEdit::dateChanged, this, &Dlg_TimeSet::onDateTimeChanged);
    connect(ui->timeEdit, &QTimeEdit::timeChanged, this, &Dlg_TimeSet::onDateTimeChanged);
}

/**
 * @brief [新增] 将给定的QDateTime对象的值同步到UI的dateEdit和timeEdit控件。
 * @param dt 要显示的时间日期对象。
 */
void Dlg_TimeSet::syncUIToDateTime(const QDateTime &dt)
{
    // 暂时阻塞信号，防止更新UI时触发 onDateTimeChanged 槽函数
    ui->dateEdit->blockSignals(true);
    ui->timeEdit->blockSignals(true);

    ui->dateEdit->setDate(dt.date());
    ui->timeEdit->setTime(dt.time());

    // 恢复信号
    ui->dateEdit->blockSignals(false);
    ui->timeEdit->blockSignals(false);
}

/**
 * @brief 定时器超时处理函数。
 * @details 当定时器触发时（即在自动更新模式下），获取当前系统时间并更新UI。
 */
void Dlg_TimeSet::onTimer()
{
    // 自动更新模式下才执行
    if (m_autoUpdateEnabled)
    {
        m_currentDateTime = QDateTime::currentDateTime();
        syncUIToDateTime(m_currentDateTime);
    }
}

/**
 * @brief “设置时间”按钮点击槽函数。
 * @details 仅当处于手动模式且用户已经修改过时间后，才将当前UI上的时间发送到设备。
 */
void Dlg_TimeSet::onSetTimeButtonClicked()
{
    // 必须是手动模式，且用户确实修改了时间
    if (!m_autoUpdateEnabled && m_isUserModified)
    {
        sendTimeToDevice();
        m_isUserModified = false; // 发送后重置修改标志
    }
}

/**
 * @brief “获取时间”按钮点击槽函数。
 * @details 仅当处于手动模式时，向设备请求其当前时间。
 */
void Dlg_TimeSet::onGetTimeButtonClicked()
{
    // 必须是手动模式
    if (!m_autoUpdateEnabled)
    {
        // --- 获取祖父窗口(new1Dlg)指针，用于调用通信函数 ---
        // 这是一个强依赖关系：Dlg_TimeSet -> Dlg_ZJM -> new1Dlg
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

        // --- 设置通信参数并发送请求 ---
        pParent->Connect_Index = 0;
        pParent->ErrorCounter = 0;
        pParent->Down = 1029; // 操作ID: 获取时间

        pParent->TimeSet_block_29.Station_adr = 0;
        pParent->TimeSet_block_29.Telegram_Nr = 29; // 报文号
        pParent->TimeSet_block_29.F1_field = 0;
        pParent->TimeSet_block_29.F2_field = 0x80;
        pParent->TimeSet_block_29.Data = (unsigned char *)"";
        pParent->Fun_Connect(pParent->TimeSet_block_29, 0);

        Function_ID = 1029; // 设置当前操作的功能ID

        // 使用单次定时器延迟处理返回的数据，给设备响应留出时间
        QTimer::singleShot(1000, this, &Dlg_TimeSet::processDeviceData);
    }
}

/**
 * @brief “退出”按钮点击槽函数。
 */
void Dlg_TimeSet::onExitButtonClicked()
{
    this->close();
}

/**
 * @brief “自动更新”复选框状态改变槽函数。
 * @param state 复选框的状态 (Qt::Checked 或 Qt::Unchecked)。
 * @details 根据业务逻辑，选中复选框表示“停止自动更新”（进入手动模式），
 * 未选中表示“开启自动更新”。
 */
void Dlg_TimeSet::onAutoUpdateCheckBoxChanged(int state)
{
    // 核心逻辑：选中(Checked)时，自动更新关闭(false)；未选中时，自动更新开启(true)。
    m_autoUpdateEnabled = (state != Qt::Checked);

    if (m_autoUpdateEnabled)
    {
        // 切换到自动模式
        m_timer->start();
        m_isUserModified = false; // 自动模式下，重置用户修改状态
    }
    else
    {
        // 切换到手动模式
        m_timer->stop();
    }
}

/**
 * @brief 当日期或时间控件被用户手动修改时触发。
 */
void Dlg_TimeSet::onDateTimeChanged()
{
    // 仅在手动模式下响应
    if (!m_autoUpdateEnabled)
    {
        // 从UI控件获取最新的日期和时间，更新成员变量
        QDate date = ui->dateEdit->date();
        QTime time = ui->timeEdit->time();
        m_currentDateTime = QDateTime(date, time);

        // 标记用户已经修改了时间
        m_isUserModified = true;
    }
}

/**
 * @brief 处理从设备接收到的返回数据。
 */
void Dlg_TimeSet::processDeviceData()
{
    // 确认当前操作是“获取时间”
    if (Function_ID == 1029)
    {
        // 调用重构后的函数从原始数据中解析时间
        calculateDateTimeFromDeviceData();
        // 将解析出的时间更新到UI上
        syncUIToDateTime(m_currentDateTime);
    }
    // 重置功能ID，避免重复处理
    Function_ID = 0;
}

/**
 * @brief [重构] 从设备返回的ValidData字节数组中解析出日期和时间。
 * @details 此函数替代了原有的复杂循环计算逻辑，采用 Qt 内置函数，代码更简洁、健壮。
 */
void Dlg_TimeSet::calculateDateTimeFromDeviceData()
{
    // --- 1. 解析天数 (从1984年1月1日算起) ---
    // 从 ValidData 数组中提取2个字节(小端模式)组成一个16位的天数
    unsigned short days_since_1984 = ValidData[4] | (ValidData[5] << 8);

    // --- 2. 解析毫秒数 (从当天 00:00:00.000 算起) ---
    // 从 ValidData 数组中提取4个字节(小端模式)组成一个32位的毫秒数
    // 原始代码中是3个字节，这里遵循原始逻辑
    unsigned int msecs_since_midnight = (ValidData[8] << 16) | (ValidData[7] << 8) | ValidData[6];

    // --- 3. 使用 Qt 内置函数计算最终日期和时间 ---
    QDate date = QDate(1984, 1, 1).addDays(days_since_1984 - 1); // 协议中的天数可能是从1开始的，daysTo从0开始，所以减1
    QTime time = QTime::fromMSecsSinceStartOfDay(msecs_since_midnight);

    // 更新成员变量
    m_currentDateTime = QDateTime(date, time);
}

/**
 * @brief [重构] 将UI界面上的时间打包成字节数据并发送到设备。
 * @details 使用 QTime::msecsSinceStartOfDay() 简化了毫秒数的计算。
 */
void Dlg_TimeSet::sendTimeToDevice()
{
    // --- 1. 从UI控件获取当前要设置的日期和时间 ---
    QDateTime dateTime(ui->dateEdit->date(), ui->timeEdit->time());

    // --- 2. 计算从1984年1月1日到目标日期的总天数 ---
    QDate baseDate(1984, 1, 1);
    // daysTo 返回的是间隔天数，如果协议要求当天也算1天，则需要+1
    int days = baseDate.daysTo(dateTime.date()) + 1;

    // --- 3. 计算从当天零点到目标时间的总毫秒数 ---
    // [优化] 使用QTime的内置函数，替代手动乘加计算，更简洁安全
    int msecs = dateTime.time().msecsSinceStartOfDay();

    // --- 4. 将计算结果打包到字节数组中 ---
    unsigned char DATA[6];
    DATA[0] = days & 0xFF;          // 天数低字节
    DATA[1] = (days >> 8) & 0xFF;   // 天数高字节
    DATA[2] = msecs & 0xFF;         // 毫秒数 字节0
    DATA[3] = (msecs >> 8) & 0xFF;  // 毫秒数 字节1
    DATA[4] = (msecs >> 16) & 0xFF; // 毫秒数 字节2
    DATA[5] = (msecs >> 24) & 0xFF; // 毫秒数 字节3

    // --- 5. 获取父窗口指针并发送数据 ---
    Dlg_ZJM *pParent_ZJM = qobject_cast<Dlg_ZJM *>(parent());
    if (pParent_ZJM)
    {
        new1Dlg *pParent = qobject_cast<new1Dlg *>(pParent_ZJM->parent());
        if (pParent)
        {
            pParent->Connect_Index = 0;
            pParent->ErrorCounter = 0;
            pParent->Down = 1028; // 操作ID: 设置时间

            pParent->TimeSet_block_28.Station_adr = 0;
            pParent->TimeSet_block_28.Telegram_Nr = 28; // 报文号
            pParent->TimeSet_block_28.F1_field = 0;
            pParent->TimeSet_block_28.F2_field = 0x80;

            // 复制数据到待发送的缓冲区
            unsigned char zeroCount = 0; // 似乎是用于特定通信协议的字段
            for (int i = 0; i < 6; i++)
            {
                pParent->Connect_Telegram.ValidDataToPack[i] = DATA[i];
                if (DATA[i] == 0)
                    zeroCount++;
            }
            pParent->Connect_Telegram.ValidDataToPack[6] = 0;
            pParent->Connect_Telegram.PACK_SELECT = true;

            pParent->Fun_Connect(pParent->TimeSet_block_28, zeroCount);
        }
    }
}
