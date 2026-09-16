#include "Dlg_TQCS1.h"
#include "ui_Dlg_TQCS1.h"
#include "Dlg_ZJM.h"
#include "new1Dlg.h"
#include <QMessageBox>
#include <QDebug>
#include <QScreen>

// 初始化静态成员变量
uchar Dlg_TQCS1::ValidData[280] = { 0 };

/**
 * @brief Dlg_TQCS1 类的构造函数。
 */
Dlg_TQCS1::Dlg_TQCS1(QWidget *parent) : QWidget(parent), ui(new Ui::Dlg_TQCS1), m_fetchDataTimer(new QTimer(this))
{
    ui->setupUi(this);

    // --- UI 字体和DPI缩放设置 ---
    qreal pixelRatio = qApp->devicePixelRatio();
    qreal dpiRatio = QGuiApplication::primaryScreen()->logicalDotsPerInch() / 96.0;
    qreal scaleFactor = qMin(pixelRatio, dpiRatio);
    QFont font = this->font();
    font.setPixelSize(qRound(12 * scaleFactor));
    this->setFont(font);
    this->setWindowTitle(u8"同期参数");

    // --- 初始化 ---
    TQCS_FUNCTION = 0;
    setDefaultValues(); // 设置默认参数并更新UI
    setupConnections(); // 连接所有信号与槽
}

/**
 * @brief Dlg_TQCS1 类的析构函数。
 */
Dlg_TQCS1::~Dlg_TQCS1()
{
    delete ui;
}

/**
 * @brief 初始化所有信号与槽的连接。
 */
void Dlg_TQCS1::setupConnections()
{
    // 按钮连接
    connect(ui->pushButton_Apply, &QPushButton::clicked, this, &Dlg_TQCS1::onButtonApplyClicked);
    connect(ui->pushButton_Fetch, &QPushButton::clicked, this, &Dlg_TQCS1::onButtonFetchClicked);
    connect(ui->pushButton_Default, &QPushButton::clicked, this, &Dlg_TQCS1::onButtonDefaultClicked);
    connect(ui->pushButton_Cancel, &QPushButton::clicked, this, &Dlg_TQCS1::onButtonCancelClicked);

    // 连接所有会影响其他控件状态的单选按钮和复选框到一个统一的槽函数
    connect(ui->checkBox_Func_Enab, &QCheckBox::stateChanged, this, &Dlg_TQCS1::onControlStateChanged);
    connect(ui->radioButton_Routine_Auto_Select, &QRadioButton::toggled, this, &Dlg_TQCS1::onControlStateChanged);

    // 定时器连接
    connect(m_fetchDataTimer, &QTimer::timeout, this, &Dlg_TQCS1::onFetchDataTimeout);
}

/**
 * @brief [核心重构] 从UI控件读取数据并更新到成员变量中。
 */
void Dlg_TQCS1::updateMembersFromUi()
{
    // --- 从 CheckBox 和 RadioButton 读取状态 ---
    m_funcEnabled = ui->checkBox_Func_Enab->isChecked();
    m_l1DeadVoltCheck = ui->checkBox_L1_Dead_Volt->isChecked();
    m_l2DeadVoltCheck = ui->checkBox_L2_Dead_Volt->isChecked();
    m_allDeadVoltCheck = ui->checkBox_ALL_Dead_Volt->isChecked();

    if (ui->radioButton_Phase_A->isChecked())
        m_phaseSelect = 0;
    else if (ui->radioButton_Phase_B->isChecked())
        m_phaseSelect = 1;
    else
        m_phaseSelect = 2;

    m_routineSelect = ui->radioButton_Routine_Auto_Select->isChecked() ? 0 : 1;
    m_syncStyle = ui->radioButton_ASYN_Style->isChecked() ? 0 : 1;

    // --- 从 QLineEdit 读取数值 ---
    bool ok; // 用于检查字符串到数字的转换是否成功
    m_tcb = ui->lineEdit_TCB->text().toShort(&ok);
    m_damax = ui->lineEdit_Damax->text().toFloat(&ok);
    m_duMax = ui->lineEdit_DUmax->text().toFloat(&ok);
    m_dfMax = ui->lineEdit_Dfmax->text().toFloat(&ok);
    m_dfptMax = ui->lineEdit_Dfptmax->text().toFloat(&ok);
    m_uDead = ui->lineEdit_Udead->text().toFloat(&ok);
    m_trU1U2 = ui->lineEdit_TrU1U2->text().toFloat(&ok);
    m_uMin = ui->lineEdit_Umin->text().toFloat(&ok);
    m_uMax = ui->lineEdit_Umax->text().toFloat(&ok);
    m_fMin = ui->lineEdit_fmin->text().toFloat(&ok);
    m_fMax = ui->lineEdit_fmax->text().toFloat(&ok);
    m_tsynDuration = ui->lineEdit_TsynDuration->text().toFloat(&ok);
    m_synchron = ui->lineEdit_Synchron->text().toFloat(&ok);
    m_dFmax2 = ui->lineEdit_D_fmax2->text().toFloat(&ok);
    m_dUmax2 = ui->lineEdit_D_Umax2->text().toFloat(&ok);
    m_synchron2 = ui->lineEdit_Synchron2->text().toFloat(&ok);
}

/**
 * @brief [核心重构] 根据成员变量的值，更新整个UI界面的显示。
 */
void Dlg_TQCS1::updateUiFromMembers()
{
    // --- 更新 QLineEdit ---
    ui->lineEdit_TCB->setText(QString::number(m_tcb));
    ui->lineEdit_Damax->setText(QString::number(m_damax));
    ui->lineEdit_DUmax->setText(QString::number(m_duMax));
    ui->lineEdit_Dfmax->setText(QString::number(m_dfMax));
    ui->lineEdit_Dfptmax->setText(QString::number(m_dfptMax));
    ui->lineEdit_Udead->setText(QString::number(m_uDead));
    ui->lineEdit_TrU1U2->setText(QString::number(m_trU1U2));
    ui->lineEdit_Umin->setText(QString::number(m_uMin));
    ui->lineEdit_Umax->setText(QString::number(m_uMax));
    ui->lineEdit_fmin->setText(QString::number(m_fMin));
    ui->lineEdit_fmax->setText(QString::number(m_fMax));
    ui->lineEdit_TsynDuration->setText(QString::number(m_tsynDuration));
    ui->lineEdit_Synchron->setText(QString::number(m_synchron));
    ui->lineEdit_D_fmax2->setText(QString::number(m_dFmax2));
    ui->lineEdit_D_Umax2->setText(QString::number(m_dUmax2));
    ui->lineEdit_Synchron2->setText(QString::number(m_synchron2));

    // --- 更新 CheckBox 和 RadioButton ---
    ui->checkBox_Func_Enab->setChecked(m_funcEnabled);
    ui->checkBox_L1_Dead_Volt->setChecked(m_l1DeadVoltCheck);
    ui->checkBox_L2_Dead_Volt->setChecked(m_l2DeadVoltCheck);
    ui->checkBox_ALL_Dead_Volt->setChecked(m_allDeadVoltCheck);

    if (m_phaseSelect == 0)
        ui->radioButton_Phase_A->setChecked(true);
    else if (m_phaseSelect == 1)
        ui->radioButton_Phase_B->setChecked(true);
    else
        ui->radioButton_Phase_C->setChecked(true);

    if (m_routineSelect == 0)
        ui->radioButton_Routine_Auto_Select->setChecked(true);
    else
        ui->radioButton_Manual_Select->setChecked(true);

    if (m_syncStyle == 0)
        ui->radioButton_ASYN_Style->setChecked(true);
    else
        ui->radioButton_SYN_Style->setChecked(true);

    // --- 最后，根据最新的状态更新控件的可用性 ---
    updateControlStates();
}

/**
 * @brief [重构] 更新所有依赖于当前状态的控件的启用/禁用状态。
 */
void Dlg_TQCS1::updateControlStates()
{
    // 首先从UI读取最新的状态，确保逻辑基于当前界面
    bool isFuncEnabled = ui->checkBox_Func_Enab->isChecked();
    bool isManualSelect = ui->radioButton_Manual_Select->isChecked();
    // 遍历所有需要根据Func_Enab状态切换的控件
    // [修正] 将错误的控件名称替换为.ui文件中定义的正确名称
    const QList<QWidget *> widgetsToToggle = { ui->groupBox, ui->groupBox_3, ui->groupBox_4, ui->lineEdit_TCB, ui->lineEdit_Damax, ui->lineEdit_DUmax,
        ui->lineEdit_Dfmax, ui->lineEdit_Dfptmax, ui->lineEdit_Udead, ui->lineEdit_TrU1U2, ui->lineEdit_Umin, ui->lineEdit_Umax, ui->lineEdit_fmin,
        ui->lineEdit_fmax, ui->lineEdit_TsynDuration, ui->lineEdit_Synchron, ui->lineEdit_D_fmax2, ui->lineEdit_D_Umax2, ui->lineEdit_Synchron2 };

    for (QWidget *widget : widgetsToToggle)
    {
        widget->setEnabled(isFuncEnabled);
    }
    // 单独处理有额外依赖条件的控件
    // “同步方式”只有在“功能启用”且“手动选择”时才可用
    ui->groupBox_2->setEnabled(isFuncEnabled && isManualSelect);
}

/**
 * @brief [重构] 从 ValidData 字节数组中解析数据，填充到成员变量。
 */
void Dlg_TQCS1::parseDataFromBuffer()
{
    // 从字节数组解析数据并填充到成员变量
    m_routineSelect = !ValidData[4]; // 自动(0)和手动(1)模式选择
    m_syncStyle = ValidData[5];      // 同频(1)非同频(0)选择
    m_l1DeadVoltCheck = ValidData[6];
    m_l2DeadVoltCheck = ValidData[7];
    m_allDeadVoltCheck = ValidData[8];
    m_phaseSelect = ValidData[9];
    m_tcb = readShort(ValidData, 10);
    m_damax = static_cast<float>(readShort(ValidData, 12)) / 100.0f;
    m_duMax = static_cast<float>(readShort(ValidData, 14)) / 100.0f;
    m_dfMax = static_cast<float>(readShort(ValidData, 16));
    m_dUmax2 = static_cast<float>(readShort(ValidData, 18)) / 100.0f;
    m_dFmax2 = static_cast<float>(readShort(ValidData, 20)) / 100.0f;
    m_uDead = static_cast<float>(readShort(ValidData, 22)) / 100.0f;
    m_trU1U2 = static_cast<float>(readShort(ValidData, 24)) / 100.0f;
    m_dfptMax = static_cast<float>(readShort(ValidData, 26)) / 100.0f;
    m_synchron2 = m_dfptMax; // 根据原逻辑，这两个值是相同的
    m_uMin = static_cast<float>(readShort(ValidData, 28)) / 100.0f;
    m_uMax = static_cast<float>(readShort(ValidData, 30)) / 100.0f;
    m_fMin = static_cast<float>(readShort(ValidData, 32)) / 100.0f;
    m_fMax = static_cast<float>(readShort(ValidData, 34)) / 100.0f;
    m_tsynDuration = static_cast<float>(readShort(ValidData, 36)) / 10.0f;
    m_synchron = static_cast<float>(readShort(ValidData, 38)) / 10.0f;
    m_funcEnabled = ValidData[40];
}

/**
 * @brief [重构] 将成员变量中的数据打包成字节流，存入指定的缓冲区。
 * @param dataBuffer 用于存放打包后字节数据的缓冲区。
 */
void Dlg_TQCS1::packDataToBuffer(uchar *dataBuffer)
{
    dataBuffer[0] = !m_routineSelect; // 自动手动选择
    dataBuffer[1] = m_syncStyle;      // 同频非同频选择
    dataBuffer[2] = m_l1DeadVoltCheck;
    dataBuffer[3] = m_l2DeadVoltCheck;
    dataBuffer[4] = m_allDeadVoltCheck;
    dataBuffer[5] = m_phaseSelect; // 相序选择

    // 使用辅助函数写入数值
    writeShort(dataBuffer, 6, m_tcb);
    writeShort(dataBuffer, 8, static_cast<ushort>(m_damax * 100));
    writeShort(dataBuffer, 10, static_cast<ushort>(m_duMax * 100));
    writeShort(dataBuffer, 12, static_cast<ushort>(m_dfMax));
    writeShort(dataBuffer, 14, static_cast<ushort>(m_dUmax2 * 100));
    writeShort(dataBuffer, 16, static_cast<ushort>(m_dFmax2 * 100));
    writeShort(dataBuffer, 18, static_cast<ushort>(m_uDead * 100));
    writeShort(dataBuffer, 20, static_cast<ushort>(m_trU1U2 * 100));
    writeShort(dataBuffer, 22, static_cast<ushort>(m_dfptMax * 100));
    writeShort(dataBuffer, 24, static_cast<ushort>(m_uMin * 100));
    writeShort(dataBuffer, 26, static_cast<ushort>(m_uMax * 100));
    writeShort(dataBuffer, 28, static_cast<ushort>(m_fMin * 100));
    writeShort(dataBuffer, 30, static_cast<ushort>(m_fMax * 100));
    writeShort(dataBuffer, 32, static_cast<ushort>(m_tsynDuration * 10));
    writeShort(dataBuffer, 34, static_cast<ushort>(m_synchron * 10));

    dataBuffer[36] = m_funcEnabled;

    // 其余部分归零
    memset(dataBuffer + 37, 0, 4); // 37, 38, 39, 40
}

/**
 * @brief 将所有参数恢复为出厂默认值，并更新UI。
 */
void Dlg_TQCS1::setDefaultValues()
{
    m_routineSelect = 1; // 手动
    m_syncStyle = 0;     // 非同步
    m_phaseSelect = 0;   // A相
    m_l1DeadVoltCheck = false;
    m_l2DeadVoltCheck = false;
    m_allDeadVoltCheck = false;
    m_tcb = 80;
    m_damax = 10.0f;
    m_duMax = 2.0f;
    m_dUmax2 = 2.0f;
    m_dfMax = 20.0f;
    m_dFmax2 = 0.10f;
    m_uDead = 2.0f;
    m_trU1U2 = 0.0f;
    m_dfptMax = 0.2f;
    m_synchron2 = 0.2f;
    m_uMin = 52.0f;
    m_uMax = 63.5f;
    m_fMin = 47.50f;
    m_fMax = 52.50f;
    m_tsynDuration = 30.0f;
    m_synchron = 0.5f;
    m_funcEnabled = true;

    updateUiFromMembers(); // 将设置好的默认值更新到UI
}

// --- 按钮点击事件处理函数 ---

void Dlg_TQCS1::onButtonApplyClicked()
{
    TQCS_FUNCTION = 1028; // 设置功能码为 "Apply"

    // 1. 从UI获取最新的用户输入
    updateMembersFromUi();

    // 2. 将数据打包到字节数组
    uchar dataToSend[41];
    packDataToBuffer(dataToSend);

    // 3. 准备通信
    // 获取顶层窗口指针以调用通信函数，这是项目特定结构
    Dlg_ZJM *pParent_ZJM = qobject_cast<Dlg_ZJM *>(parent());
    new1Dlg *pTopLevelWindow = pParent_ZJM ? qobject_cast<new1Dlg *>(pParent_ZJM->parent()) : nullptr;
    if (!pTopLevelWindow)
    {
        QMessageBox::warning(this, "错误", "无法获取顶层通信窗口");
        return;
    }

    // 4. 配置通信参数
    pTopLevelWindow->Connect_Index = 0;
    pTopLevelWindow->ErrorCounter = 0;
    pTopLevelWindow->Down = 1028; // "Apply" 功能码
    pTopLevelWindow->Connect_Telegram.PACK_SELECT = true;

    int zero_NR = 0; // 计算需要压缩的0字节数量 (项目特定逻辑)
    for (int i = 0; i < 41; i++)
    {
        pTopLevelWindow->Connect_Telegram.ValidDataToPack[i] = dataToSend[i];
        if (dataToSend[i] == 0)
        {
            zero_NR++;
        }
    }
    zero_NR--; // 根据原逻辑减一

    // 5. 构造报文并发送
    pTopLevelWindow->Msg_block_34.Station_adr = 0;
    pTopLevelWindow->Msg_block_34.Telegram_Nr = 34; // 报文号
    pTopLevelWindow->Msg_block_34.F1_field = 0;
    pTopLevelWindow->Msg_block_34.F2_field = 0x80;
    pTopLevelWindow->Fun_Connect(pTopLevelWindow->Msg_block_34, zero_NR);
}

void Dlg_TQCS1::onButtonFetchClicked()
{
    TQCS_FUNCTION = 1029; // 设置功能码为 "Fetch"

    // 获取顶层窗口指针
    Dlg_ZJM *pParent_ZJM = qobject_cast<Dlg_ZJM *>(parent());
    new1Dlg *pTopLevelWindow = pParent_ZJM ? qobject_cast<new1Dlg *>(pParent_ZJM->parent()) : nullptr;
    if (!pTopLevelWindow)
    {
        QMessageBox::warning(this, "错误", "无法获取顶层通信窗口");
        return;
    }

    // 配置通信参数
    pTopLevelWindow->Connect_Index = 0;
    pTopLevelWindow->ErrorCounter = 0;
    pTopLevelWindow->Down = 1029; // "Fetch" 功能码

    // 构造报文并发送
    pTopLevelWindow->TQCS_block_35.Station_adr = 0;
    pTopLevelWindow->TQCS_block_35.Telegram_Nr = 35; // 报文号
    pTopLevelWindow->TQCS_block_35.F1_field = 0;
    pTopLevelWindow->TQCS_block_35.F2_field = 0x80;
    pTopLevelWindow->TQCS_block_35.Data = (uchar *)"";
    pTopLevelWindow->Fun_Connect(pTopLevelWindow->TQCS_block_35, 0);

    // 启动一次性定时器，延时666ms后处理返回的数据
    m_fetchDataTimer->setSingleShot(true);
    m_fetchDataTimer->start(666);
}

void Dlg_TQCS1::onButtonDefaultClicked()
{
    TQCS_FUNCTION = 1030; // 设置功能码为 "Default"
    setDefaultValues();   // 调用函数恢复默认值并更新UI
}

void Dlg_TQCS1::onButtonCancelClicked()
{
    this->close(); // 关闭当前窗口
}

/**
 * @brief [重构] 统一处理所有影响控件启用/禁用状态的事件。
 */
void Dlg_TQCS1::onControlStateChanged()
{
    // 每当相关控件状态改变时，都调用此函数来更新所有依赖控件的可用性
    updateControlStates();
}

/**
 * @brief "获取"数据后的延时处理槽函数。
 */
void Dlg_TQCS1::onFetchDataTimeout()
{
    // 1. 从静态缓冲区 `ValidData` 解析数据到成员变量
    parseDataFromBuffer();
    // 2. 将成员变量的新值更新到UI界面
    updateUiFromMembers();
}

// --- 辅助函数 ---

/**
 * @brief [辅助函数] 从指定的字节缓冲区和偏移量读取一个ushort (16位无符号整数)。
 * @details 采用低字节在前，高字节在后的方式（Little-Endian）。
 */
ushort Dlg_TQCS1::readShort(const uchar *buffer, int offset)
{
    return (static_cast<ushort>(buffer[offset + 1]) << 8) | buffer[offset];
}

/**
 * @brief [辅助函数] 将一个ushort值写入指定的字节缓冲区和偏移量。
 * @details 同样采用低字节在前，高字节在后的方式。
 */
void Dlg_TQCS1::writeShort(uchar *buffer, int offset, ushort value)
{
    buffer[offset] = value & 0xFF;            // 写入低字节
    buffer[offset + 1] = (value >> 8) & 0xFF; // 写入高字节
}
