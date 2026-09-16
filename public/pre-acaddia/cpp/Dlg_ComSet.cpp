#include "Dlg_ComSet.h"
#include "ui_Dlg_ComSet.h"
#include "Dlg_ZJM.h"
#include "new1Dlg.h"
#include <QMessageBox>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QShowEvent>
#include <QDebug>
#include <QScreen>
Dlg_ComSet::Dlg_ComSet(QWidget *parent)
    : QWidget(parent), ui(new Ui::Dlg_ComSet), m_nComSetBaud(0), m_nComSetCheck(0), m_nComSetData(0), m_nComSetStop(0), m_nComSetCom(0)
{
    ui->setupUi(this);
    qreal pixelRatio = qApp->devicePixelRatio();
    // 获取逻辑DPI缩放比（考虑系统字体缩放）
    qreal dpiRatio = QGuiApplication::primaryScreen()->logicalDotsPerInch() / 96.0;
    // 综合缩放因子
    qreal scaleFactor = qMin(pixelRatio, dpiRatio);
    QFont font = this->font();                   // 获取基准字体
    font.setPixelSize(qRound(12 * scaleFactor)); // 基于DPI缩放
    this->setFont(font);
    // 设置窗口标题
    setWindowTitle(u8"串口设置");

    // 初始化下拉框内容
    initComboBoxes();

    // 连接信号和槽
    connect(ui->pushButton_2, &QPushButton::clicked, this, &Dlg_ComSet::onOkButtonClicked);     // 确定按钮
    connect(ui->pushButton, &QPushButton::clicked, this, &Dlg_ComSet::onCancelButtonClicked);   // 退出按钮
    connect(ui->pushButton_3, &QPushButton::clicked, this, &Dlg_ComSet::onGetComButtonClicked); // 获取串口按钮
}

Dlg_ComSet::~Dlg_ComSet()
{
    delete ui;
}

void Dlg_ComSet::initComboBoxes()
{
    // 清空所有下拉框
    ui->comboBox->clear();   // 波特率
    ui->comboBox_2->clear(); // 校验位
    ui->comboBox_3->clear(); // 数据位
    ui->comboBox_4->clear(); // 停止位
    ui->comboBox_5->clear(); // COM口
    ui->comboBox_6->clear(); // 获取到的COM口列表

    // 添加波特率选项
    ui->comboBox->addItem("4800");
    ui->comboBox->addItem("9600");
    ui->comboBox->addItem("19200");
    ui->comboBox->addItem("38400");

    // 添加校验位选项
    ui->comboBox_2->addItem("NONE");
    ui->comboBox_2->addItem("ODD");
    ui->comboBox_2->addItem("EVEN");

    // 添加数据位选项
    ui->comboBox_3->addItem("8");
    ui->comboBox_3->addItem("7");
    ui->comboBox_3->addItem("6");

    // 添加停止位选项
    ui->comboBox_4->addItem("1");
    ui->comboBox_4->addItem("2");

    // 添加COM口选项
    for (int i = 1; i <= 20; ++i)
    {
        ui->comboBox_5->addItem(QString("COM%1").arg(i));
    }
}

void Dlg_ComSet::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    // 获取父窗口和祖父窗口
    new1Dlg *pParent = getGrandParent();
    if (!pParent)
    {
        QMessageBox::warning(this, u8"错误", u8"无法获取new1Dlg祖父窗口");
        return;
    }
    Dlg_ZJM *pParent_ZJM = getParentZJM();
    if (!pParent_ZJM)
    {
        QMessageBox::warning(this, "错误", "无法获取");
        return;
    }
    // 获取当前串口设置
    if (pParent->m_serialPort && pParent->m_serialPort->isOpen())
    {
        QString para;

        // 获取波特率
        int baudRate = pParent->m_serialPort->baudRate();
        if (baudRate == 4800)
            m_nComSetBaud = 0;
        else if (baudRate == 9600)
            m_nComSetBaud = 1;
        else if (baudRate == 19200)
            m_nComSetBaud = 2;
        else if (baudRate == 38400)
            m_nComSetBaud = 3;

        // 获取校验位
        QSerialPort::Parity parity = pParent->m_serialPort->parity();
        if (parity == QSerialPort::NoParity)
            m_nComSetCheck = 0;
        else if (parity == QSerialPort::OddParity)
            m_nComSetCheck = 1;
        else if (parity == QSerialPort::EvenParity)
            m_nComSetCheck = 2;

        // 获取数据位
        QSerialPort::DataBits dataBits = pParent->m_serialPort->dataBits();
        if (dataBits == QSerialPort::Data8)
            m_nComSetData = 0;
        else if (dataBits == QSerialPort::Data7)
            m_nComSetData = 1;
        else if (dataBits == QSerialPort::Data6)
            m_nComSetData = 2;

        // 获取停止位
        QSerialPort::StopBits stopBits = pParent->m_serialPort->stopBits();
        if (stopBits == QSerialPort::OneStop)
            m_nComSetStop = 0;
        else if (stopBits == QSerialPort::TwoStop)
            m_nComSetStop = 1;

        // 获取COM口
        QString portName = pParent->m_serialPort->portName();
        // qDebug() << "portName:" << portName;
        if (portName.startsWith("COM"))
        {
            bool ok;
            int comNum = portName.mid(3).toInt(&ok);
            if (ok && comNum >= 1 && comNum <= 20)
            {
                m_nComSetCom = comNum - 1;
            }
        }
    }

    // 更新UI显示
    ui->comboBox->setCurrentIndex(m_nComSetBaud);
    ui->comboBox_2->setCurrentIndex(m_nComSetCheck);
    ui->comboBox_3->setCurrentIndex(m_nComSetData);
    ui->comboBox_4->setCurrentIndex(m_nComSetStop);
    ui->comboBox_5->setCurrentIndex(m_nComSetCom);
}

void Dlg_ComSet::onOkButtonClicked()
{
    // 获取父窗口和祖父窗口
    new1Dlg *pParent = getGrandParent();
    if (!pParent)
    {
        QMessageBox::warning(this, u8"错误", u8"无法获取new1Dlg祖父窗口");
        return;
    }
    // 获取当前选择的COM口和设置参数
    m_nComSetBaud = ui->comboBox->currentIndex();
    m_nComSetCheck = ui->comboBox_2->currentIndex();
    m_nComSetData = ui->comboBox_3->currentIndex();
    m_nComSetStop = ui->comboBox_4->currentIndex();
    m_nComSetCom = ui->comboBox_5->currentIndex();

    // 构建COM口名称
    QString comPort = QString("COM%1").arg(m_nComSetCom + 1);

    // 构建设置参数字符串
    switch (m_nComSetBaud)
    {
    case 0:
        settingpara = "4800,";
        break;
    case 1:
        settingpara = "9600,";
        break;
    case 2:
        settingpara = "19200,";
        break;
    case 3:
        settingpara = "38400,";
        break;
    default:
        settingpara = "9600,";
        break;
    }

    // 校验位
    switch (m_nComSetCheck)
    {
    case 0:
        settingpara += "n,";
        break;
    case 1:
        settingpara += "o,";
        break;
    case 2:
        settingpara += "e,";
        break;
    default:
        settingpara += "n,";
        break;
    }

    // 数据位
    switch (m_nComSetData)
    {
    case 0:
        settingpara += "8,";
        break;
    case 1:
        settingpara += "7,";
        break;
    case 2:
        settingpara += "6,";
        break;
    default:
        settingpara += "8,";
        break;
    }

    // 停止位
    switch (m_nComSetStop)
    {
    case 0:
        settingpara += "1";
        break;
    case 1:
        settingpara += "2";
        break;
    default:
        settingpara += "1";
        break;
    }

    // 关闭已打开的串口
    if (pParent->m_serialPort && pParent->m_serialPort->isOpen())
    {
        pParent->m_serialPort->close();
    }

    // 配置串口
    pParent->m_serialPort->setPortName(comPort);

    // 尝试打开串口
    if (!pParent->m_serialPort->isOpen())
    {
        if (!pParent->m_serialPort->open(QIODevice::ReadWrite))
        {
            QMessageBox::warning(this, "错误", "无法获取");
            return;
        }
    }

    // 配置串口参数
    // 波特率
    QSerialPort::BaudRate baudRate;
    switch (m_nComSetBaud)
    {
    case 0:
        baudRate = QSerialPort::Baud4800;
        break;
    case 1:
        baudRate = QSerialPort::Baud9600;
        break;
    case 2:
        baudRate = QSerialPort::Baud19200;
        break;
    case 3:
        baudRate = QSerialPort::Baud38400;
        break;
    default:
        baudRate = QSerialPort::Baud9600;
        break;
    }
    pParent->m_serialPort->setBaudRate(baudRate);

    // 校验位
    QSerialPort::Parity parity;
    switch (m_nComSetCheck)
    {
    case 0:
        parity = QSerialPort::NoParity;
        break;
    case 1:
        parity = QSerialPort::OddParity;
        break;
    case 2:
        parity = QSerialPort::EvenParity;
        break;
    default:
        parity = QSerialPort::NoParity;
        break;
    }
    pParent->m_serialPort->setParity(parity);

    // 数据位
    QSerialPort::DataBits dataBits;
    switch (m_nComSetData)
    {
    case 0:
        dataBits = QSerialPort::Data8;
        break;
    case 1:
        dataBits = QSerialPort::Data7;
        break;
    case 2:
        dataBits = QSerialPort::Data6;
        break;
    default:
        dataBits = QSerialPort::Data8;
        break;
    }
    pParent->m_serialPort->setDataBits(dataBits);

    // 停止位
    QSerialPort::StopBits stopBits;
    switch (m_nComSetStop)
    {
    case 0:
        stopBits = QSerialPort::OneStop;
        break;
    case 1:
        stopBits = QSerialPort::TwoStop;
        break;
    default:
        stopBits = QSerialPort::OneStop;
        break;
    }
    pParent->m_serialPort->setStopBits(stopBits);

    // connect(pParent->m_serialPort, &QSerialPort::readyRead, this, &pParent->new1Dlg::handleReadyRead);

    // 清空缓冲区
    pParent->m_serialPort->clear();

    // 显示设置信息
    QMessageBox::information(this, "串口设置", comPort + ":" + settingpara);

    // 关闭对话框
    close();
}

void Dlg_ComSet::onCancelButtonClicked()
{
    // 关闭对话框
    close();
}

void Dlg_ComSet::onGetComButtonClicked()
{
    // 清空COM口列表
    ui->comboBox_6->clear();

    // 获取所有可用串口
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

    // 如果没有可用串口，禁用获取串口按钮
    if (ports.isEmpty())
    {
        ui->comboBox_6->setEnabled(false);
        return;
    }

    // 添加可用串口
    for (const QSerialPortInfo &port : ports)
    {
        ui->comboBox_6->addItem(port.portName());
    }

    // 默认选择最后一个
    if (ui->comboBox_6->count() > 0)
    {
        ui->comboBox_6->setCurrentIndex(ui->comboBox_6->count() - 1);
    }
}

Dlg_ZJM *Dlg_ComSet::getParentZJM()
{
    return qobject_cast<Dlg_ZJM *>(parentWidget());
}

new1Dlg *Dlg_ComSet::getGrandParent()
{
    Dlg_ZJM *parent_ZJM = getParentZJM();
    if (parent_ZJM)
    {
        return qobject_cast<new1Dlg *>(parent_ZJM->parentWidget());
    }
    return nullptr;
}
