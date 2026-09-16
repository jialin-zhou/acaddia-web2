#include "new1Dlg.h"
#include "qmessagebox.h"
#include "ui_new1Dlg.h"
#include "Dlg_ZJM.h"
#include "qmessagebox.h"
#include <QtSerialPort>
#include <QByteArray>
#include <QDebug>
#include <QScreen>
#include <QSerialPortInfo>

new1Dlg::new1Dlg(QWidget *parent) : QWidget(parent), ui(new Ui::new1Dlg)
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
    setWindowTitle(u8"ACAD DIA(V1.0.0)");
    initSerialPort();
    initTimers();
    setupConnections_new1Dlg();
    ErrorCounter = 0; // variable of error times
    Connect_Index = 0;
    Zero_NR = 0;
    RXDAT[0] = '\0';
    index = 0;
    Current_Style = 1;
    Msg_Len = 0;
    Down = 1000;
    RX_Msg_End = true;
}

void new1Dlg::initTimers()
{
    // 接收超时定时器 (100ms)
    m_timer11 = new QTimer(this);
    // m_timer11->setInterval(100);
    m_timer11->setSingleShot(true);

    // 通信超时定时器 (3333ms)
    m_timer12 = new QTimer(this);
    // m_timer12->setInterval(3333);
    m_timer12->setSingleShot(true);
}

void new1Dlg::setupConnections_new1Dlg()
{
    connect(ui->pushButton, &QPushButton::clicked, this, &new1Dlg::OnConnect);   // 在线连接
    connect(ui->pushButton_2, &QPushButton::clicked, this, &new1Dlg::OnOffline); // 离线连接
    connect(ui->pushButton_3, &QPushButton::clicked, this, &new1Dlg::OnExit);    // 退出
    connect(m_timer11, &QTimer::timeout, this, &new1Dlg::handleTimer11);
    connect(m_timer12, &QTimer::timeout, this, &new1Dlg::handleTimer12);
    connect(m_serialPort, &QSerialPort::readyRead, this, &new1Dlg::handleReadyRead);
    // connect(m_commTimer, &QTimer::timeout, this, &new1Dlg::OnTimer);
}

void new1Dlg::OnConnect()
{
    /*
      Down
    IDC_CONNECT   == 1001   1
    IDC_OFFLINE  == 1000    2
    IDC_BUTTON_ZJM_FETCH  ==1005
    IDC_BUTTON_ZJM_SAVE   ==1125
    IDC_BUTTON_ZJM_OPEN   ==1126
     */
    Down = 1001;
    Msg_Len = 0;
    uint T1, T2 = 0;
    T1 = 0; // timer
    //	UCHAR TXDat_Temp[280];
    //	UCHAR RXTele_Nr=0;
    // MessageBox("1");
    uchar *pTXDATA1;

    // 初始化TQCS_block_35
    TQCS_block_35.Telegram_Nr = 35;
    TQCS_block_35.Station_adr = 0;
    TQCS_block_35.F1_field = 0;
    TQCS_block_35.F2_field = 0x80;
    TQCS_block_35.Data = (uchar *)"";

    ACAD_block_33.Telegram_Nr = 33; // AD参数 交流AD参数回读
    ACAD_block_33.Station_adr = 0;
    ACAD_block_33.F1_field = 0;
    ACAD_block_33.F2_field = 0x80;
    ACAD_block_33.Data = (uchar *)"";
    //	ACAD_block_33.Data[0]='\0';

    ADData_block_37.Telegram_Nr = 37; // 主页面 AD计算值回读
    ADData_block_37.Station_adr = 0;
    ADData_block_37.F1_field = 0;
    ADData_block_37.F2_field = 0x80;
    ADData_block_37.Data = (uchar *)"";

    Message_block_49.Telegram_Nr = 49; // 通信报文  message回读
    Message_block_49.Station_adr = 0;
    Message_block_49.F1_field = 0;
    Message_block_49.F2_field = 0x80;
    Message_block_49.Data = (uchar *)"";

    AC_angle_block_47.Telegram_Nr = 47; // 角度矢量 交流AD角度回读
    AC_angle_block_47.Station_adr = 0;
    AC_angle_block_47.F1_field = 0;
    AC_angle_block_47.F2_field = 0x80;
    AC_angle_block_47.Data = (uchar *)"";

    Connect_Telegram.ValidDataToPack[0] = 0;
    Connect_Telegram.ValidDataToPack[1] = 0;
    Connect_Telegram.ValidDataToPack[2] = 0;
    Connect_Telegram.ValidDataToPack[3] = 0;
    Connect_Telegram.ValidDataToPack[4] = 0;
    Connect_Telegram.ValidDataToPack[5] = 0;
    Connect_Telegram.ValidDataToPack[6] = '\0';

    if (ErrorCounter < 5 && Connect_Index == 0)
    {
        // MessageBox("发送同期参数命令");
        // Fun_Connect(ACAD_block_33,0);
        Fun_Connect(TQCS_block_35, 0); // 发送同期参数命令
        // yyp
    }

    m_CStringStatus = "Connecting...... Please waitting!";

    // 阻塞式等待，cpu空转程序卡死
    for (int ii = 0; ii < 2500; ii++)
    {
        for (int jj = 0; jj < 65535; jj++)
            ;
    };

    QMessageBox::warning(this, "Loading", "Loading Data Please Waitting!");
    // index=0;
    if (Connect_Telegram.ACK1 == false && Connect_Telegram.ACK2 == false) // 有参数返回;
    {
        // 带有重试机制的循环，最多重试5次
        for (; ErrorCounter < 5; ErrorCounter++)
        {
            RXTele_Nr = (uchar)Connect_Telegram.Telegarm_Array[3];

            if (RXTele_Nr == 34) // 同期参数     回读命令 相应
            {
                for (int len = 0; len < (uchar)(Connect_Telegram.Block_len); len++)
                {
                    ZJM_dlg.TQCS_dlg.ValidData[len] = (uchar)Connect_Telegram.Telegarm_Array[len]; // 将有效数据传递给 同期参数回读数组ValidData[]
                }
                TXDATA1[0] = 0xa2;
                SentData(TXDATA1); // master -> slave:     A2
                //	MessageBox("同期参数回读命令");

                //	m_CStringStatus="Loading...syn parameter";
                //	UpdateData(FALSE);
                ErrorCounter = 0;
                //	ReceiveOK=FALSE;
                //	Connect_Index=0;
                continue;
            }
            else if (RXTele_Nr == 32) // AD 参数 交流AD参数回读命令  相应
            {
                for (int len = 0; len < ((uchar)Connect_Telegram.Block_len); len++)
                {
                    ZJM_dlg.AD1_dlg.ValidData[len] = (uchar)Connect_Telegram.Telegarm_Array[len]; // 将有效数据传递给 AD参数回读数组ValidData[]
                }
                TXDATA1[0] = 0xa2;
                SentData(TXDATA1); // master -> slave:     A2
                // MessageBox("交流AD参数回读命令");
                ErrorCounter = 0;

                continue;
            }
            else if (RXTele_Nr == 48) // 角度矢量  AD angle命令  相应
            {
                for (int len = 0; len < (uchar)(Connect_Telegram.Block_len); len++)
                {
                    ZJM_dlg.BaseAngle_dlg.ValidData[len] = (uchar)Connect_Telegram.Telegarm_Array[len]; // 将有效数据传递给 AD参数回读数组ValidData[]
                }
                TXDATA1[0] = 0xa2;
                SentData(TXDATA1); // master -> slave:     A2
                // MessageBox("Loading...Base angle");
                ErrorCounter = 0;
                // m_CStringStatus="All data has been loaded!";
                QMessageBox::information(this, u8"提示", "All data has been loaded!");
                break;
                // ErrorCounter=0;
                //    Connect_Index++;
            }
            else if (RXTele_Nr == 50) // 通信报文 Message命令  相应
            {
                for (int len = 0; len < (uchar)(Connect_Telegram.Block_len); len++)
                {
                    ZJM_dlg.Message_dlg.ValidData[len] = (uchar)Connect_Telegram.Telegarm_Array[len]; // 将有效数据传递给 AD参数回读数组ValidData[]
                }
                TXDATA1[0] = 0xa2;
                SentData(TXDATA1); // master -> slave:     A2
                // MessageBox("Loading...Base angle");
                // QMessageBox::information(this,u8"提示","Loading...Base angle!");
                ErrorCounter = 0;
                // m_CStringStatus="All data has been loaded!";

                // QMessageBox::warning(this,"error","All data has been loaded!");
                //	ReceiveOK=TRUE;
                break;
                // ErrorCounter=0;
                //    Connect_Index++;
            }
            else if (RXTele_Nr == 36) // ZJM_dlg主界面  ADCacul_ValidData  AD计算值回读命令  相应
            {
                for (int len = 0; len < (uchar)(Connect_Telegram.Block_len); len++)
                {
                    ZJM_dlg.ADCacul_ValidData[len] = (uchar)Connect_Telegram.Telegarm_Array[len]; // 将有效数据传递给 AD参数回读数组ValidData[]
                }
                TXDATA1[0] = 0xa2;
                SentData(TXDATA1); // master -> slave:     A2
                ErrorCounter = 0;
                continue;
            }
            else
            {
                //	ErrorCounter++;//返回命令错误时
                continue;
            }
        }
    }

    if (ErrorCounter == 5)
    {
        QMessageBox::warning(this, "error", "communication error");
        return;
    }
    else
    {
        DIM_public = 16384;
        DC_V_I_select[0] = 1;
        DC_V_I_select[1] = 1;
        DC_V_I_select[2] = 1;
        DC_V_I_select[3] = 1;
        DC_V_I_select[4] = 1;
        DC_V_I_select[5] = 1;
        DC_V_I_select[6] = 1;
        DC_V_I_select[7] = 1;
        Current_Style = (uint)(ZJM_dlg.AD1_dlg.ValidData[55]);

        showZjmDialog(); // 显示 Dlg_ZJM 窗口
    }
}

void new1Dlg::OnOffline()
{
    Down = 1000;
    if (m_serialPort != nullptr && m_serialPort->isOpen())
    {
        m_serialPort->close();
        qDebug() << u8"离线模式已激活，串口已关闭";
    }

    if (Down == 1000)
    {
        m_CStringStatus = "Loading......";
        // UpdateData(false);
        // ShowWindow(SW_HIDE);
        for (int kk = 0; kk < 255; kk++)
        {
            ZJM_dlg.AD1_dlg.ValidData[kk] = 0;
            // ZJM_dlg.TQCS_dlg.ValidData[kk]=0;
        }

        // this->close();
        DIM_public = 16384;

        showZjmDialog(); // 显示 Dlg_ZJM 窗口
    }
}

/*
 * 显示 Dlg_ZJM 窗口
 */
void new1Dlg::showZjmDialog()
{
    // 创建 Dlg_ZJM 对话框实例，并将其父对象设置为当前窗口 (this)
    Dlg_ZJM *dialog_Dlg_ZJM = new Dlg_ZJM(this);

    // 关联子窗口的关闭信号到父窗口的 OnExit 槽函数
    connect(dialog_Dlg_ZJM, &Dlg_ZJM::windowClosed, this, &new1Dlg::OnExit);

    // 设置为模态对话框，打开此窗口时，阻止用户操作父窗口
    dialog_Dlg_ZJM->setWindowModality(Qt::ApplicationModal);

    // 设置窗口在关闭时自动删除，避免内存泄漏
    dialog_Dlg_ZJM->setAttribute(Qt::WA_DeleteOnClose);

    // --- 将窗口在屏幕上居中显示 ---
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    int x = (screenGeometry.width() - dialog_Dlg_ZJM->width()) / 2;
    int y = (screenGeometry.height() - dialog_Dlg_ZJM->height()) / 2;
    dialog_Dlg_ZJM->move(x, y);

    dialog_Dlg_ZJM->show();
}

void new1Dlg::OnExit()
{
    this->close();
}

bool new1Dlg::Fun_Connect(TeleBlock TeleBlock1, int zero_NR) // function: generate the telegram ,and then sent them.
{
    uchar *pTXDATA1; // Pointer of the sent data
    int tt;
    pTXDATA1 = Connect_Telegram.TelePack(TeleBlock1.Station_adr, TeleBlock1.Telegram_Nr, TeleBlock1, zero_NR);
    Connect_Telegram.Block_len = pTXDATA1[1]; // block len
    for (tt = 0; tt < (Connect_Telegram.Block_len + 6); tt++)
        TXDATA1[tt] = *pTXDATA1++;
    TXDATA1[tt] = '\0';
    SentData(TXDATA1);
    // QMessageBox::warning(this,"null",u8"下载成功");
    return true;
}

void new1Dlg::initSerialPort()
{
    if (m_serialPort && m_serialPort->isOpen())
    {
        m_serialPort->close();
    }

    // 创建并配置串口
    // m_serialPort = new QSerialPort(this);

    // 配置串口参数

    // 获取所有可用串口
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    if (ports.isEmpty())
    {
        QMessageBox::critical(this, tr("Error"), tr("Cannot open serial port!"));
        return;
    }

    // 尝试每个可用端口
    foreach (const QSerialPortInfo &portInfo, ports)
    {
        m_serialPort = new QSerialPort();
        m_serialPort->setPort(portInfo);
    }
    // m_serialPort->setPortName("COM1"); // 设置端口为COM1

    m_serialPort->setBaudRate(QSerialPort::Baud9600);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);

    connect(m_serialPort, &QSerialPort::readyRead, this, &new1Dlg::handleReadyRead);

    // 尝试打开端口
    if (!m_serialPort->isOpen())
    {
        m_serialPort->open(QIODevice::ReadWrite);
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), tr("Cannot open serial port1!"));
    }
}
void new1Dlg::handleReadyRead()
{
    // qDebug() << u8"==> 3. 成功触发 handleReadyRead！收到数据！";
    // 处理接收到的数据
    QByteArray receivedData = m_serialPort->readAll();
    const int len = receivedData.size();

    // 安全检查 - 防止索引越界
    if (index + receivedData.size() >= sizeof(RXDAT) / sizeof(RXDAT[0]))
    {
        qDebug() << "Warning: Buffer overflow prevented. Resetting index.";
        index = 0; // 重置索引
    }

    if (len > 0)
    {
        // 处理接收到的数据
        for (int k = 0; k < len; k++)
        {
            quint8 bt = static_cast<quint8>(receivedData.at(k));
            RXDAT[index++] = bt; // 存储接收到的数据

            // 调试用：可以输出每个字节的16进制值
            // qDebug() << QString("Received: 0x%1").arg(bt, 2, 16, QLatin1Char('0'));
        }

        // 启动定时器等待数据接收完成（100ms后超时）
        // m_timer11 = new QTimer(this);
        m_timer11->start(100);
    }
}

void new1Dlg::SentData(uchar TXData[])
{
    // MessageBox("sentdata ture");
    QByteArray array;
    // array.RemoveAll();

    if (TXData[1] != '\0')

    {
        // array.SetSize((TXData[2]+6));
        int size = static_cast<uchar>(TXData[2]) + 6;
        array.resize(size);

        for (int i = 0; i < size; i++)
        {
            array[i] = TXData[i];
        }
        // Send data via Qt's serial port
        if (m_serialPort != nullptr && m_serialPort->isOpen())
        {
            m_serialPort->write(array); // 发送数据
            if (!m_serialPort->waitForBytesWritten(1000))
            {
                QMessageBox::warning(this, "Failed", u8"Failed to write data to serial port");
            }
        }
    }
    else if (TXData[1] == '\0') // master -> slave
    {
        array.resize(1);
        array[0] = TXData[0];
        m_serialPort->write(array); // 发送数据
    }

    if (Connect_Index == 0)
    {
        // m_timer12 = new QTimer(this);
        m_timer12->start(3333);
    }
    // QMessageBox::warning(this,"null",u8"发送数据成功1");
}

void new1Dlg::handleTimer11()
{
    qDebug() << u8"==> 4. 成功触发 handleTimer11 开始处理数据帧。";
    uchar TXDATA1[280];
    RXDAT[index] = '\0';
    m_timer11->stop();

    if (Connect_Index == 0)
    {
        m_timer12->stop();
    }

    if (Connect_Telegram.TeleUnpack(RXDAT))
    {
        Connect_Index++;
        index = 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //               save  the  valid  data  into  the array
    //////////////////////////////////////////////////////////////////////////

    if (Connect_Telegram.ACK1 == false && Connect_Telegram.ACK2 == false) // 有参数返回;
    {
        RXTele_Nr = (uchar)Connect_Telegram.Telegarm_Array[3];

        if (RXTele_Nr == (uchar)0x22) // 同期参数   回读命令 相应
        {
            // MessageBox("TQCS ");
            for (int len = 0; len < (uchar)(Connect_Telegram.Block_len); len++)
            {
                ZJM_dlg.TQCS_dlg.ValidData[len] = (uchar)Connect_Telegram.Telegarm_Array[len]; // 将有效数据传递给 同期参数回读数组ValidData[]
            }
            TXDATA1[0] = 0xa2;
            TXDATA1[1] = '\0';
            // MessageBox("34:  A2");
            SentData(TXDATA1); // master -> slave:     A2
        }

        else if (RXTele_Nr == (uchar)0x20) // 交流AD参数回读命令  相应
        {
            for (int len = 0; len < ((uchar)Connect_Telegram.Block_len); len++)
            {
                ZJM_dlg.AD1_dlg.ValidData[len] = (uchar)Connect_Telegram.Telegarm_Array[len]; // 将有效数据传递给 AD参数回读数组ValidData[]
            }
            TXDATA1[0] = 0xa2;
            TXDATA1[1] = '\0';
            // MessageBox("32:  A2");
            SentData(TXDATA1); // master -> slave:     A2
        }

        else if (RXTele_Nr == (uchar)0x24) // ZJM_dlg主界面  AD计算值  回读命令  相应
        {
            for (int len = 0; len < (uchar)(Connect_Telegram.Block_len); len++)
            {
                ZJM_dlg.ADCacul_ValidData[len] = (uchar)Connect_Telegram.Telegarm_Array[len]; // 将有效数据传递给 AD参数回读数组ValidData[]
            }
            TXDATA1[0] = 0xa2;
            TXDATA1[1] = '\0';
            // MessageBox("36:  A2");
            SentData(TXDATA1); // master -> slave:     A2
        }

        else if (RXTele_Nr == (uchar)0x2) // ZJM_dlg主界面 MsgList_h_ValidData   PC请求告警信息(有Head)命令  相应
        {
            for (int len = 0; len < (uchar)(Connect_Telegram.Block_len); len++)
            {
                ZJM_dlg.MsgList_h_ValidData[len] = (uchar)Connect_Telegram.Telegarm_Array[len]; // 将有效数据传递给 AD参数回读数组ValidData[]
            }
            if (Connect_Telegram.Telegarm_Array[13] == 0) //// //判断Head时是否读完信息//////
                RX_Msg_End = true;
            else
                RX_Msg_End = true;
            TXDATA1[0] = 0xa2;
            TXDATA1[1] = '\0';
            // MessageBox("2:  A2");
            SentData(TXDATA1); // master -> slave:     A2
        }
        else if (RXTele_Nr == (uchar)0x3) // ZJM_dlg主界面  ZJM_dlg.MsgList_ValidData PC请求告警信息(无head)命令   相应
        {
            uint len = Msg_Len;
            for (; len < (Connect_Telegram.Block_len + Msg_Len); len++)
            {
                ZJM_dlg.MsgList_ValidData[len] = (uchar)Connect_Telegram.Telegarm_Array[len]; // 将有效数据传递给 AD参数回读数组ValidData[]
            }
            Msg_Len = len;
            if (Connect_Telegram.Telegarm_Array[4] == 0) // 判断是否读完信息
            {
                RX_Msg_End = true; // UpdateData(FALSE);
                // MessageBox("receive end!!!");
            }
            else
            {
                RX_Msg_End = false;
                if (Down == 1001)
                    Connect_Index = 4;
                else if (Down == 1005)
                    Connect_Index = 2;
            }
            TXDATA1[0] = 0xa2;
            TXDATA1[1] = '\0';
            // MessageBox("3:  A2");
            SentData(TXDATA1);
        }
        else if (RXTele_Nr == 41) // AD 通道校准    回读   相应
        {
            for (int len = 0; len < (uchar)(Connect_Telegram.Block_len); len++)
            {
                ZJM_dlg.ADAdjust_dlg.ValidData[len] = (uchar)Connect_Telegram.Telegarm_Array[len]; // 将有效数据传递给 AD参数回读数组ValidData[]
            }
            TXDATA1[0] = 0xa2;
            TXDATA1[1] = '\0';
            // MessageBox("41:  A2");
            SentData(TXDATA1); // master -> slave:     A2
        }

        else if (RXTele_Nr == 46) // 同期命令  同期合闸 Fetch   相应
        {
            for (int len = 0; len < (uchar)(Connect_Telegram.Block_len); len++)
            {
                ZJM_dlg.TQML_dlg.ValidData[len] = (uchar)Connect_Telegram.Telegarm_Array[len];
            }
            TXDATA1[0] = 0xa2;
            TXDATA1[1] = '\0';
            // MessageBox("46:  A2");
            SentData(TXDATA1); // master -> slave:     A2
        }

        else if (RXTele_Nr == 28) // 时间设置    测试 Fetch   相应
        {
            for (int len = 0; len < (uchar)(Connect_Telegram.Block_len); len++)
            {
                ZJM_dlg.TimeSet_dlg.ValidData[len] = (uchar)Connect_Telegram.Telegarm_Array[len];
            }
            TXDATA1[0] = 0xa2;
            TXDATA1[1] = '\0';
            // MessageBox("28:  A2");
            SentData(TXDATA1); // master -> slave:     A2
        }

        ///////////////////////////////////////////////////////////////////////////////////////////////
        else if (RXTele_Nr == 48) // 角度尺量   Base_angle回应
        {
            for (int len = 0; len < (uchar)(Connect_Telegram.Block_len); len++)
            {
                ZJM_dlg.BaseAngle_dlg.ValidData[len] = (uchar)Connect_Telegram.Telegarm_Array[len];
            }
            TXDATA1[0] = 0xa2;
            TXDATA1[1] = '\0';
            // MessageBox("46:  A2");
            SentData(TXDATA1); // master -> slave:     A2
        }

        else if (RXTele_Nr == 50) // 通信报文  Message回应
        {
            for (int len = 0; len < (uchar)(Connect_Telegram.Block_len); len++)
            {
                ZJM_dlg.Message_dlg.ValidData[len] = (uchar)Connect_Telegram.Telegarm_Array[len];
            }
            TXDATA1[0] = 0xa2;
            TXDATA1[1] = '\0';
            // MessageBox("46:  A2");
            SentData(TXDATA1); // master -> slave:     A2
        }

        ///////////////////////////////////////
        else // 返回命令错误时
        {
            ErrorCounter++;
            Connect_Index = 0;
        }
    }
    else if (Connect_Telegram.ACK1 == true || Connect_Telegram.ACK2 == true)
    {
        if (Down == 1001)
        {
            ErrorCounter++;
            Connect_Index = 0;
        }
    }
    if ((Down == 1001 && Connect_Index == 0) && ErrorCounter < 5) // 同期参数
    {
        Fun_Connect(TQCS_block_35, 0);
    }

    if ((Down == 1001 && Connect_Index == 1) && ErrorCounter < 5) // AD参数
    {
        Fun_Connect(ACAD_block_33, 0);
    }

    if (((Down == 1001 && Connect_Index == 2) || (Down == 1005 && Connect_Index == 0)) && ErrorCounter < 5) // 主页面 AD计算值回读
    {
        // MessageBox("ADData_block_37");
        Fun_Connect(ADData_block_37, 0);
    }

    if (((Down == 1001 && Connect_Index == 3) || (Down == 1005 && Connect_Index == 1)) && ErrorCounter < 5) // 角度矢量
    {
        Fun_Connect(AC_angle_block_47, 0);
    }
    if (((Down == 1001 && Connect_Index == 4) || (Down == 1005 && Connect_Index == 2)) && ErrorCounter < 5 && RX_Msg_End == false) // 角度矢量
    {
        Fun_Connect(AC_angle_block_47, 0);
    }
    if (((Down == 1001 && Connect_Index == 6) || (Down == 1005 && Connect_Index == 3)) && ErrorCounter < 5 && RX_Msg_End == false) // 通信报文
    {
        Fun_Connect(Message_block_49, 0);
    }

    if (Down == 1001 && ErrorCounter < 5 && Connect_Index == 5) //
    {
        Down = 1000;
        QMessageBox msgBox;
        int ret = msgBox.exec();
        if (ret == QMessageBox::Cancel)
        {
            this->close();
        }
        Connect_Index = 0;
    }
    if (ErrorCounter > 4)
    {
        QMessageBox::warning(this, "error", u8"communication error11");
        if (Down == 1001)
            this->deleteLater();
    }

    // Rest of your timer 11 handling code...
}

void new1Dlg::handleTimer12()
{
    m_timer12->stop();
    QMessageBox::information(this, "error", u8"communication error12");

    if (Down == 1001)
    {
        // this->deleteLater();
        this->close();
    }
}

new1Dlg::~new1Dlg()
{
    delete ui;
}
