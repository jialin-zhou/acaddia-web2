#include "Dlg_AD1.h"
#include "ui_Dlg_AD1.h"
#include "Dlg_ZJM.h"
#include "new1Dlg.h"
#include "qmessagebox.h"
#include <QDebug>
#include <QScreen>
uchar Dlg_AD1::ValidData[280] = { 0 };

Dlg_AD1::Dlg_AD1(QWidget *parent) : QWidget(parent), ui(new Ui::Dlg_AD1)
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
    setWindowTitle(u8"AD参数");
    setupConnections();
    // QTimer::singleShot(1000, this, &Dlg_AD1::onpushButton_2Ad1FetchClicked);
}

void Dlg_AD1::setupConnections()
{
    connect(ui->pushButton, &QPushButton::clicked, this, &Dlg_AD1::onpushButtonAd1ApplyClicked);       // 下载参数
    connect(ui->pushButton_2, &QPushButton::clicked, this, &Dlg_AD1::onpushButton_2Ad1FetchClicked);   // 获取参数
    connect(ui->pushButton_3, &QPushButton::clicked, this, &Dlg_AD1::onpushButton_3Ad1DefaultClicked); // 默认参数
    connect(ui->pushButton_4, &QPushButton::clicked, this, &Dlg_AD1::onpushButton_4AdExitClicked);     // 返回
    connect(ui->radioButton, &QRadioButton::toggled, this, &Dlg_AD1::onradioButtonToggled);            // dc1 source select
    connect(ui->radioButton_2, &QRadioButton::toggled, this, &Dlg_AD1::onradioButton_2Toggled);
    connect(ui->radioButton_3, &QRadioButton::toggled, this, &Dlg_AD1::onradioButton_3Toggled); // dc2 source select
    connect(ui->radioButton_4, &QRadioButton::toggled, this, &Dlg_AD1::onradioButton_4Toggled);
    connect(ui->radioButton_5, &QRadioButton::toggled, this, &Dlg_AD1::onradioButton_5Toggled); // dc3 source select
    connect(ui->radioButton_6, &QRadioButton::toggled, this, &Dlg_AD1::onradioButton_6Toggled);
    connect(ui->radioButton_7, &QRadioButton::toggled, this, &Dlg_AD1::onradioButton_7Toggled); // dc4 source select
    connect(ui->radioButton_8, &QRadioButton::toggled, this, &Dlg_AD1::onradioButton_8Toggled);
    connect(ui->radioButton_9, &QRadioButton::toggled, this, &Dlg_AD1::onradioButton_9Toggled); // dc5 source select
    connect(ui->radioButton_10, &QRadioButton::toggled, this, &Dlg_AD1::onradioButton_10Toggled);
    connect(ui->radioButton_11, &QRadioButton::toggled, this, &Dlg_AD1::onradioButton_11Toggled); // dc6 source select
    connect(ui->radioButton_12, &QRadioButton::toggled, this, &Dlg_AD1::onradioButton_12Toggled);
    connect(ui->radioButton_13, &QRadioButton::toggled, this, &Dlg_AD1::onradioButton_13Toggled); // dc7 source select
    connect(ui->radioButton_14, &QRadioButton::toggled, this, &Dlg_AD1::onradioButton_14Toggled);
    connect(ui->radioButton_15, &QRadioButton::toggled, this, &Dlg_AD1::onradioButton_15Toggled); // dc8 source select
    connect(ui->radioButton_16, &QRadioButton::toggled, this, &Dlg_AD1::onradioButton_16Toggled);

    // connect(ui->pushButton,SIGNAL(clicked()),this,SLOT(onpushButtonAd1ApplyClicked()));
}

void Dlg_AD1::onpushButtonAd1ApplyClicked() // 下载参数
{
    AD1_FUNCTION = 1031; // IDC_BUTTON_AD1_APPLY 2
    unsigned short Temp_short = 0;
    uchar Temp_UCHAR = 0;
    // 安全获取父窗口(Dlg_ZJM)
    Dlg_ZJM *pParent_ZJM = (Dlg_ZJM *)parent();
    if (!pParent_ZJM)
    {
        QMessageBox::warning(this, u8"错误", u8"无法获取Dlg_ZJM父窗口");
        return;
    }

    // 安全获取祖父窗口(new1Dlg)
    new1Dlg *pParent = (new1Dlg *)pParent_ZJM->parent();
    if (!pParent)
    {
        QMessageBox::warning(this, u8"错误", u8"无法获取new1Dlg祖父窗口");
        return;
    }

    // QMessageBox::warning(this,"null","successed1");
    pParent->Connect_Index = 0;
    pParent->ErrorCounter = 0;
    pParent->Down = 1031;
    /*
        m_shortAD1_V_DC1 = ui->lineEdit_29->text().toInt();//DC1
        DATA[1] = static_cast<char>(m_shortAD1_V_DC1 & 0x00FF);
        DATA[2] = static_cast<char>(m_shortAD1_V_DC1 >> 8);

        m_shortAD1_C_DC1 = ui->lineEdit_29->text().toInt();//DC1
        DATA[3] = static_cast<char>(m_shortAD1_C_DC1 & 0x00FF);
        DATA[4] = static_cast<char>(m_shortAD1_C_DC1 >> 8);

        // DC Source Selects
        DATA[5] = static_cast<char>(m_nAD1_DCSS);
        pParent->DC_V_I_select[0] = m_nAD1_DCSS;
        DATA[6] = 0; // bak
        DATA[7] = '\0';
        */
    m_shortAD1_U1a = ui->lineEdit->text().toShort();
    // DATA[0] = static_cast<char>(m_shortAD1_U1a & 0x00FF);
    // DATA[1] = static_cast<char>(m_shortAD1_U1a >> 8);
    Temp_short = m_shortAD1_U1a; // U1a
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[0] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[1] = Temp_UCHAR;

    m_shortAD1_U1b = ui->lineEdit_2->text().toShort();
    DATA[2] = static_cast<uchar>(m_shortAD1_U1b & 0x00FF);
    DATA[3] = static_cast<uchar>(m_shortAD1_U1b >> 8);

    m_shortAD1_U1c = ui->lineEdit_3->text().toShort();
    DATA[4] = static_cast<uchar>(m_shortAD1_U1c & 0x00FF);
    DATA[5] = static_cast<uchar>(m_shortAD1_U1c >> 8);

    m_shortAD1_I1a = ui->lineEdit_4->text().toShort();
    DATA[6] = static_cast<uchar>(m_shortAD1_I1a & 0x00FF);
    DATA[7] = static_cast<uchar>(m_shortAD1_I1a >> 8);

    m_shortAD1_I1b = ui->lineEdit_5->text().toShort();
    DATA[8] = static_cast<uchar>(m_shortAD1_I1b & 0x00FF);
    DATA[9] = static_cast<uchar>(m_shortAD1_I1b >> 8);

    m_shortAD1_I1c = ui->lineEdit_6->text().toShort();
    DATA[10] = static_cast<uchar>(m_shortAD1_I1c & 0x00FF);
    DATA[11] = static_cast<uchar>(m_shortAD1_I1c >> 8);

    m_shortAD1_U2a = ui->lineEdit_7->text().toShort();
    DATA[12] = static_cast<uchar>(m_shortAD1_U2a & 0x00FF);
    DATA[13] = static_cast<uchar>(m_shortAD1_U2a >> 8);

    m_shortAD1_U2b = ui->lineEdit_8->text().toShort();
    DATA[14] = static_cast<uchar>(m_shortAD1_U2b & 0x00FF);
    DATA[15] = static_cast<uchar>(m_shortAD1_U2b >> 8);

    m_shortAD1_U2c = ui->lineEdit_9->text().toShort();
    DATA[16] = static_cast<uchar>(m_shortAD1_U2c & 0x00FF);
    DATA[17] = static_cast<uchar>(m_shortAD1_U2c >> 8);

    m_shortAD1_I2a = ui->lineEdit_10->text().toShort();
    DATA[18] = static_cast<uchar>(m_shortAD1_I2a & 0x00FF);
    DATA[19] = static_cast<uchar>(m_shortAD1_I2a >> 8);

    m_shortAD1_I2b = ui->lineEdit_11->text().toShort();
    DATA[20] = static_cast<uchar>(m_shortAD1_I2b & 0x00FF);
    DATA[21] = static_cast<uchar>(m_shortAD1_I2b >> 8);

    m_shortAD1_I2c = ui->lineEdit_12->text().toShort();
    DATA[22] = static_cast<uchar>(m_shortAD1_I2c & 0x00FF);
    DATA[23] = static_cast<uchar>(m_shortAD1_I2c >> 8);

    // DC voltage and current conversions
    m_shortAD1_V_DC1 = ui->lineEdit_29->text().toShort(); // V DC
    DATA[24] = static_cast<uchar>(m_shortAD1_V_DC1 & 0x00FF);
    DATA[25] = static_cast<uchar>(m_shortAD1_V_DC1 >> 8);

    m_shortAD1_V_DC2 = ui->lineEdit_31->text().toShort();
    DATA[26] = static_cast<uchar>(m_shortAD1_V_DC2 & 0x00FF);
    DATA[27] = static_cast<uchar>(m_shortAD1_V_DC2 >> 8);

    m_shortAD1_V_DC3 = ui->lineEdit_33->text().toShort();
    DATA[28] = static_cast<uchar>(m_shortAD1_V_DC3 & 0x00FF);
    DATA[29] = static_cast<uchar>(m_shortAD1_V_DC3 >> 8);

    m_shortAD1_V_DC4 = ui->lineEdit_35->text().toShort();
    DATA[30] = static_cast<uchar>(m_shortAD1_V_DC4 & 0x00FF);
    DATA[31] = static_cast<uchar>(m_shortAD1_V_DC4 >> 8);

    m_shortAD1_V_DC5 = ui->lineEdit_37->text().toShort();
    DATA[32] = static_cast<uchar>(m_shortAD1_V_DC5 & 0x00FF);
    DATA[33] = static_cast<uchar>(m_shortAD1_V_DC5 >> 8);

    m_shortAD1_V_DC6 = ui->lineEdit_39->text().toShort();
    DATA[34] = static_cast<uchar>(m_shortAD1_V_DC6 & 0x00FF);
    DATA[35] = static_cast<uchar>(m_shortAD1_V_DC6 >> 8);

    m_shortAD1_V_DC7 = ui->lineEdit_41->text().toShort();
    DATA[36] = static_cast<uchar>(m_shortAD1_V_DC7 & 0x00FF);
    DATA[37] = static_cast<uchar>(m_shortAD1_V_DC7 >> 8);

    m_shortAD1_V_DC8 = ui->lineEdit_43->text().toShort();
    DATA[38] = static_cast<uchar>(m_shortAD1_V_DC8 & 0x00FF);
    DATA[39] = static_cast<uchar>(m_shortAD1_V_DC8 >> 8);

    m_shortAD1_C_DC1 = ui->lineEdit_30->text().toInt(); // C DC
    DATA[40] = static_cast<uchar>(m_shortAD1_C_DC1 & 0x00FF);
    DATA[41] = static_cast<uchar>(m_shortAD1_C_DC1 >> 8);

    m_shortAD1_C_DC2 = ui->lineEdit_32->text().toInt();
    DATA[42] = static_cast<uchar>(m_shortAD1_C_DC2 & 0x00FF);
    DATA[43] = static_cast<uchar>(m_shortAD1_C_DC2 >> 8);

    m_shortAD1_C_DC3 = ui->lineEdit_34->text().toInt();
    DATA[44] = static_cast<uchar>(m_shortAD1_C_DC3 & 0x00FF);
    DATA[45] = static_cast<uchar>(m_shortAD1_C_DC3 >> 8);

    m_shortAD1_C_DC4 = ui->lineEdit_36->text().toInt();
    DATA[46] = static_cast<uchar>(m_shortAD1_C_DC4 & 0x00FF);
    DATA[47] = static_cast<uchar>(m_shortAD1_C_DC4 >> 8);

    m_shortAD1_C_DC5 = ui->lineEdit_38->text().toInt();
    DATA[48] = static_cast<uchar>(m_shortAD1_C_DC5 & 0x00FF);
    DATA[49] = static_cast<uchar>(m_shortAD1_C_DC5 >> 8);

    m_shortAD1_C_DC6 = ui->lineEdit_40->text().toInt();
    DATA[50] = static_cast<uchar>(m_shortAD1_C_DC6 & 0x00FF);
    DATA[51] = static_cast<uchar>(m_shortAD1_C_DC6 >> 8);

    m_shortAD1_C_DC7 = ui->lineEdit_42->text().toInt();
    DATA[52] = static_cast<uchar>(m_shortAD1_C_DC7 & 0x00FF);
    DATA[53] = static_cast<uchar>(m_shortAD1_C_DC7 >> 8);

    m_shortAD1_C_DC8 = ui->lineEdit_44->text().toInt();
    DATA[54] = static_cast<uchar>(m_shortAD1_C_DC8 & 0x00FF);
    DATA[55] = static_cast<uchar>(m_shortAD1_C_DC8 >> 8);

    // LINE voltage and current conversions

    m_shortAD1_V_LINE1 = ui->lineEdit_13->text().toShort();
    DATA[56] = static_cast<uchar>(m_shortAD1_V_LINE1 & 0x00FF);
    DATA[57] = static_cast<uchar>(m_shortAD1_V_LINE1 >> 8);
    m_shortAD1_V_LINE2 = ui->lineEdit_14->text().toShort();
    DATA[58] = static_cast<uchar>(m_shortAD1_V_LINE2 & 0x00FF);
    DATA[59] = static_cast<uchar>(m_shortAD1_V_LINE2 >> 8);

    m_shortAD1_C_LINE1 = ui->lineEdit_15->text().toShort();
    DATA[60] = static_cast<uchar>(m_shortAD1_C_LINE1 & 0x00FF);
    DATA[61] = static_cast<uchar>(m_shortAD1_C_LINE1 >> 8);
    m_shortAD1_C_LINE2 = ui->lineEdit_16->text().toShort();
    DATA[62] = static_cast<uchar>(m_shortAD1_C_LINE2 & 0x00FF);
    DATA[63] = static_cast<uchar>(m_shortAD1_C_LINE2 >> 8);

    // Configuration settings
    m_Model_Sec = ui->comboBox_5->currentText().toInt();
    DATA[64] = static_cast<uchar>(m_Model_Sec); // Interpolation Model Select
    m_Delay = ui->lineEdit_49->text().toInt();
    DATA[65] = static_cast<uchar>(m_Delay & 0x00FF); // Ad sample Fix
    DATA[66] = static_cast<uchar>(m_Delay >> 8);

    m_Delay = ui->lineEdit_50->text().toInt();
    DATA[67] = static_cast<uchar>(m_Primary_Current_Scaling / 25); // Primary_Current_Scaling

    DATA[68] = static_cast<uchar>(m_nAD1_SecVNV); // 多余未删除的，默认为0

    // DC Source Selects
    DATA[69] = static_cast<uchar>(m_nAD1_DCSS);
    pParent->DC_V_I_select[0] = m_nAD1_DCSS;

    DATA[70] = static_cast<uchar>(m_nAD1_DCSS2);
    pParent->DC_V_I_select[1] = m_nAD1_DCSS2;

    DATA[71] = static_cast<uchar>(m_nAD1_DCSS3);
    pParent->DC_V_I_select[2] = m_nAD1_DCSS3;

    DATA[72] = static_cast<uchar>(m_nAD1_DCSS4);
    pParent->DC_V_I_select[3] = m_nAD1_DCSS4;

    DATA[73] = static_cast<uchar>(m_nAD1_DCSS5);
    pParent->DC_V_I_select[4] = m_nAD1_DCSS5;

    DATA[74] = static_cast<uchar>(m_nAD1_DCSS6);
    pParent->DC_V_I_select[5] = m_nAD1_DCSS6;

    DATA[75] = static_cast<uchar>(m_nAD1_DCSS7);
    pParent->DC_V_I_select[6] = m_nAD1_DCSS7;

    DATA[76] = static_cast<uchar>(m_nAD1_DCSS8);
    pParent->DC_V_I_select[7] = m_nAD1_DCSS8;

    // Channel Selects
    m_Channel_Sec1 = ui->comboBox->currentText().toInt() - 1;
    DATA[77] = static_cast<uchar>(m_Channel_Sec1); // Channel Select IA
    m_Channel_Sec2 = ui->comboBox_2->currentText().toInt() - 1;
    DATA[78] = static_cast<uchar>(m_Channel_Sec2); // Channel Select IB
    m_Channel_Sec3 = ui->comboBox_3->currentText().toInt() - 1;
    DATA[79] = static_cast<uchar>(m_Channel_Sec3); // Channel Select IC
    m_Channel_SecX = ui->comboBox_4->currentText().toInt() - 1;
    DATA[80] = static_cast<uchar>(m_Channel_SecX); // Channel Select IX

    // Channel Delay Ticks
    m_Delay_A = ui->lineEdit_45->text().toInt();
    DATA[81] = static_cast<uchar>(m_Delay_A & 0x00FF); // Channel Delay IA LSB
    DATA[82] = static_cast<uchar>(m_Delay_A >> 8);     // Channel Delay IA MSB
    m_Delay_B = ui->lineEdit_46->text().toInt();
    DATA[83] = static_cast<uchar>(m_Delay_B & 0x00FF); // Channel Delay IB LSB
    DATA[84] = static_cast<uchar>(m_Delay_B >> 8);     // Channel Delay IB MSB
    m_Delay_C = ui->lineEdit_47->text().toInt();
    DATA[85] = static_cast<uchar>(m_Delay_C & 0x00FF); // Channel Delay IC LSB
    DATA[86] = static_cast<uchar>(m_Delay_C >> 8);     // Channel Delay IC MSB
    m_Delay_A = ui->lineEdit_48->text().toInt();
    DATA[87] = static_cast<uchar>(m_Delay_X & 0x00FF); // Channel Delay IX LSB
    DATA[88] = static_cast<uchar>(m_Delay_X >> 8);     // Channel Delay IX MSB

    // End of data
    DATA[89] = 0; // bak
    DATA[90] = '\0';

    pParent->Connect_Telegram.PACK_SELECT = true;
    pParent->Zero_NR = 0;

    for (int tt = 0; tt < 90; tt++)
    {
        pParent->Connect_Telegram.ValidDataToPack[tt] = DATA[tt];
        if (DATA[tt] == 0)
        {
            pParent->Zero_NR++;
        }
    }
    pParent->Zero_NR--;

    pParent->Msg_block_32.Station_adr = 0;
    pParent->Msg_block_32.Telegram_Nr = 32;
    pParent->Msg_block_32.F1_field = 0;
    pParent->Msg_block_32.F2_field = 0x80;
    pParent->Fun_Connect(pParent->Msg_block_32, pParent->Zero_NR);

    QMessageBox::information(this, u8"下载提示", u8"下载成功");
}

void Dlg_AD1::onpushButton_2Ad1FetchClicked() // 获取参数
{
    // QMessageBox::warning(this,"下载提示","下载成功");
    AD1_FUNCTION = 1030; // IDC_BUTTON_AD1_Fetch 1

    Dlg_ZJM *pParent_ZJM = (Dlg_ZJM *)parent();
    if (!pParent_ZJM)
    {
        QMessageBox::warning(this, u8"错误", u8"无法获取Dlg_ZJM父窗口");
        return;
    }

    // 安全获取祖父窗口(new1Dlg)
    new1Dlg *pParent = (new1Dlg *)pParent_ZJM->parent();
    if (!pParent)
    {
        QMessageBox::warning(this, u8"错误", u8"无法获取new1Dlg祖父窗口");
        return;
    }

    pParent->Connect_Index = 0;
    pParent->ErrorCounter = 0;
    pParent->Down = 1030;

    pParent->ACAD_block_33.Station_adr = 0;
    pParent->ACAD_block_33.Telegram_Nr = 33;
    pParent->ACAD_block_33.F1_field = 0;
    pParent->ACAD_block_33.F2_field = 0x80;
    pParent->ACAD_block_33.Data = (uchar *)"";
    pParent->Fun_Connect(pParent->ACAD_block_33, 0);
    // ===== 新增的延迟逻辑部分 =====
    // 改进后的消息框显示
    /*QMessageBox msgBox(this);
   msgBox.setWindowTitle(u8"测试提示");
    msgBox.setText(u8"已发送获取参数请求");
    msgBox.setInformativeText(u8"在常规操作中，设备将返回数据。\n如需测试，请在5秒内通过串口工具发送测试数据。");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setMinimumWidth(400); // 设置最小宽度
    msgBox.exec();
*/
    // 使用QTimer延迟处理
    QTimer::singleShot(1000, this, [this]() {
        // 准备处理接收到的设备数据
        processDeviceData();
    });
}

void Dlg_AD1::processDeviceData() // 获取数据1
{
    // 检查功能状态
    if (AD1_FUNCTION != 1030)
    {
        return;
    }

    // 输出当前ValidData前10个字节用于诊断
    /* qDebug() << "ValidData current state:";
     for (int i = 0; i < 10; i++) {
         qDebug() << "ValidData[" << i << "] = 0x" << QString::number(ValidData[i], 16).rightJustified(2, '0');
     }*/

    // 检查是否仍然是未初始化数据
    // bool hasUninitializedData = false;
    // for (int i = 0; i < 10; i++) {
    //     if (ValidData[i] == 0xcd) {
    //         hasUninitializedData = true;
    //         break;
    //     }
    // }
    /*
        if (true) {
            qDebug() << "Warning: ValidData contains uninitialized data";
            // 获取原始数据失败，尝试从new1Dlg重新获取
            Dlg_ZJM* pParent_ZJM = qobject_cast<Dlg_ZJM*>(parent());
            if (pParent_ZJM) {
                new1Dlg* pParent = qobject_cast<new1Dlg*>(pParent_ZJM->parent());
                if (pParent && pParent->Connect_Telegram.Block_len > 0) {
                    qDebug() << "Try to re-obtain from the original data...";
                    for (int i = 0; i < pParent->Connect_Telegram.Block_len; i++) {
                        ValidData[i] = pParent->Connect_Telegram.Telegarm_Array[i];
                    }
                }
            }
        }*/
    /*
    // 检查数据长度是否足够
    if(ValidData.size() < 93) {
        QMessageBox::warning(this,"ERROR","Received incomplete data, expected 93 bytes, got");

        return;
    }

    // 使用QDataStream简化数据解析
    QDataStream stream(ValidData);
    stream.setByteOrder(QDataStream::LittleEndian); // 根据实际数据端序设置
    stream >> m_shortAD1_V_DC1 >> m_shortAD1_C_DC1;
    quint8 lowByte = static_cast<quint8>(ValidData[4]);  // 低字节 (相当于原TempData_UCHAR)
    quint8 highByte = static_cast<quint8>(ValidData[5]); // 高字节
    m_shortAD1_U1a = static_cast<qint16>((highByte << 8) | lowByte);
    quint8 lowByte_2 = static_cast<quint8>(ValidData[6]);  // 低字节 (相当于原TempData_UCHAR)
    quint8 highByte_2= static_cast<quint8>(ValidData[7]); // 高字节
    m_shortAD1_U1a = static_cast<qint16>((highByte_2 << 8) | lowByte_2);
    m_nAD1_DCSS = static_cast<uint>(ValidData[8]);

    */

    unsigned short TempData_short = 0;
    uchar TempData_UCHAR = 0;
    uint TempData_UINT = 0;
    // bool TempData_BOOL=false;

    TempData_UCHAR = ValidData[4]; // U1a
    TempData_short = ValidData[5];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_shortAD1_U1a = TempData_short;

    TempData_UCHAR = ValidData[6]; // U1b
    TempData_short = ValidData[7];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_shortAD1_U1b = TempData_short;

    TempData_UCHAR = ValidData[8]; // U1c
    TempData_short = ValidData[9];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_shortAD1_U1c = TempData_short;

    TempData_UCHAR = ValidData[10]; // I1a
    TempData_short = ValidData[11];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_shortAD1_I1a = TempData_short;

    TempData_UCHAR = ValidData[12]; // I1b
    TempData_short = ValidData[13];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_shortAD1_I1b = TempData_short;

    TempData_UCHAR = ValidData[14]; // I1c
    TempData_short = ValidData[15];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_shortAD1_I1c = TempData_short;

    TempData_UCHAR = ValidData[16]; // U2a
    TempData_short = ValidData[17];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_shortAD1_U2a = TempData_short;

    TempData_UCHAR = ValidData[18]; // U2b
    TempData_short = ValidData[19];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_shortAD1_U2b = TempData_short;

    TempData_UCHAR = ValidData[20]; // U2c
    TempData_short = ValidData[21];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_shortAD1_U2c = TempData_short;

    TempData_UCHAR = ValidData[22]; // I2a
    TempData_short = ValidData[23];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_shortAD1_I2a = TempData_short;

    TempData_UCHAR = ValidData[24]; // I2b
    TempData_short = ValidData[25];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_shortAD1_I2b = TempData_short;

    TempData_UCHAR = ValidData[26]; // I2c
    TempData_short = ValidData[27];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_shortAD1_I2c = TempData_short;

    TempData_UCHAR = ValidData[28]; // V DC1
    TempData_short = ValidData[29];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_shortAD1_V_DC1 = TempData_short;

    // m_shortAD1_V_DC1 = *(short int*)(&ValidData[28]);
    TempData_UCHAR = ValidData[30]; // V DC2
    TempData_short = ValidData[31];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_shortAD1_V_DC2 = TempData_short;

    TempData_UCHAR = ValidData[32]; // V DC3
    TempData_short = ValidData[33];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_shortAD1_V_DC3 = TempData_short;

    TempData_UCHAR = ValidData[34]; // V DC4
    TempData_short = ValidData[35];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_shortAD1_V_DC4 = TempData_short;

    // m_shortAD1_V_DC4 = *(short*)(&ValidData[34]);

    TempData_UCHAR = ValidData[36]; // V DC5
    TempData_short = ValidData[37];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_shortAD1_V_DC5 = TempData_short;

    TempData_UCHAR = ValidData[38]; // V DC6
    TempData_short = ValidData[39];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_shortAD1_V_DC6 = TempData_short;

    TempData_UCHAR = ValidData[40]; // V DC7
    TempData_short = ValidData[41];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_shortAD1_V_DC7 = TempData_short;

    TempData_UCHAR = ValidData[42]; // V DC8
    TempData_short = ValidData[43];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_shortAD1_V_DC8 = TempData_short;

    TempData_UCHAR = ValidData[44]; // C DC1
    TempData_short = ValidData[45];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_shortAD1_C_DC1 = TempData_short;

    TempData_UCHAR = ValidData[46]; // C DC2
    TempData_short = ValidData[47];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_shortAD1_C_DC2 = TempData_short;

    TempData_UCHAR = ValidData[48]; // C DC3
    TempData_short = ValidData[49];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_shortAD1_C_DC3 = TempData_short;

    TempData_UCHAR = ValidData[50]; // C DC4
    TempData_short = ValidData[51];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_shortAD1_C_DC4 = TempData_short;

    TempData_UCHAR = ValidData[52]; // C DC5
    TempData_short = ValidData[53];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_shortAD1_C_DC5 = TempData_short;

    TempData_UCHAR = ValidData[54]; // C DC6
    TempData_short = ValidData[55];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_shortAD1_C_DC6 = TempData_short;

    TempData_UCHAR = ValidData[56]; // C DC7
    TempData_short = ValidData[57];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_shortAD1_C_DC7 = TempData_short;

    TempData_UCHAR = ValidData[58]; // C DC8
    TempData_short = ValidData[59];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_shortAD1_C_DC8 = TempData_short;

    TempData_UCHAR = ValidData[60]; // V_LINE1
    TempData_short = ValidData[61];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_shortAD1_V_LINE1 = TempData_short;

    TempData_UCHAR = ValidData[62]; // V_LINE2
    TempData_short = ValidData[63];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_shortAD1_V_LINE2 = TempData_short;

    TempData_UCHAR = ValidData[64]; // C_LINE1
    TempData_short = ValidData[65];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_shortAD1_C_LINE1 = TempData_short;

    TempData_UCHAR = ValidData[66]; // C_LINE2
    TempData_short = ValidData[67];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_shortAD1_C_LINE2 = TempData_short;

    TempData_UCHAR = ValidData[68]; // Interpolation Model Select
    TempData_UINT = (uint)TempData_UCHAR;
    m_Model_Sec = TempData_UINT;

    TempData_UCHAR = ValidData[69]; // Common Delay
    TempData_short = ValidData[70];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_Delay = TempData_short;

    TempData_UCHAR = ValidData[71]; // Scaling
    TempData_UINT = (uint)TempData_UCHAR * 25;
    m_Primary_Current_Scaling = TempData_UINT;
    // pParent->Current_Style=m_nAD1_SecCNV;

    TempData_UCHAR = ValidData[72]; // Second Voltage Nom Value
    TempData_UINT = (uint)TempData_UCHAR;
    m_nAD1_SecVNV = TempData_UINT;

    TempData_UCHAR = ValidData[73]; // DC1 source select
    TempData_UINT = (uint)TempData_UCHAR;
    m_nAD1_DCSS = TempData_UINT;

    TempData_UCHAR = ValidData[74]; // DC2 source select
    TempData_UINT = (uint)TempData_UCHAR;
    m_nAD1_DCSS2 = TempData_UINT;

    TempData_UCHAR = ValidData[75]; // DC3 source select
    TempData_UINT = (uint)TempData_UCHAR;
    m_nAD1_DCSS3 = TempData_UINT;

    TempData_UCHAR = ValidData[76]; // DC4 source select
    TempData_UINT = (uint)TempData_UCHAR;
    m_nAD1_DCSS4 = TempData_UINT;

    TempData_UCHAR = ValidData[77]; // DC5 source select
    TempData_UINT = (uint)TempData_UCHAR;
    m_nAD1_DCSS5 = TempData_UINT;

    TempData_UCHAR = ValidData[78]; // DC6 source select
    TempData_UINT = (uint)TempData_UCHAR;
    m_nAD1_DCSS6 = TempData_UINT;

    TempData_UCHAR = ValidData[79]; // DC7 source select
    TempData_UINT = (uint)TempData_UCHAR;
    m_nAD1_DCSS7 = TempData_UINT;

    TempData_UCHAR = ValidData[80]; // DC8 source select
    TempData_UINT = (uint)TempData_UCHAR;
    m_nAD1_DCSS8 = TempData_UINT;

    TempData_UCHAR = ValidData[81]; // Channel Seclect IA
    TempData_UINT = (uint)TempData_UCHAR;
    m_Channel_Sec1 = TempData_UINT;

    TempData_UCHAR = ValidData[82]; // Channel Seclect IB
    TempData_UINT = (uint)TempData_UCHAR;
    m_Channel_Sec2 = TempData_UINT;

    TempData_UCHAR = ValidData[83]; // Channel Seclect IC
    TempData_UINT = (uint)TempData_UCHAR;
    m_Channel_Sec3 = TempData_UINT;

    TempData_UCHAR = ValidData[84]; // Channel Seclect IX
    TempData_UINT = (uint)TempData_UCHAR;
    m_Channel_SecX = TempData_UINT;

    TempData_UCHAR = ValidData[85]; // Channel Delay IA
    TempData_short = ValidData[86];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_Delay_A = TempData_short;

    TempData_UCHAR = ValidData[87]; // Channel Delay IA
    TempData_short = ValidData[88];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_Delay_B = TempData_short;

    TempData_UCHAR = ValidData[89]; // Channel Delay IC
    TempData_short = ValidData[90];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_Delay_C = TempData_short;

    TempData_UCHAR = ValidData[91]; // Channel Delay IX
    TempData_short = ValidData[92];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    m_Delay_X = TempData_short;

    // 更新界面显示
    updateUiFromData();

    // 重置功能状态
    AD1_FUNCTION = NULL;
}

void Dlg_AD1::updateUiFromData() // 控件数据显示在界面中
{
    // QMessageBox::warning(this,"ERROR","Received incomplete data, expected 93 bytes, got");

    // 更新电压电流显示
    ui->lineEdit->setText(QString::number(m_shortAD1_U1a));
    ui->lineEdit_2->setText(QString::number(m_shortAD1_U1b));
    ui->lineEdit_3->setText(QString::number(m_shortAD1_U1c));
    ui->lineEdit_4->setText(QString::number(m_shortAD1_I1a));
    ui->lineEdit_5->setText(QString::number(m_shortAD1_I1b));
    ui->lineEdit_6->setText(QString::number(m_shortAD1_I1c));
    ui->lineEdit_7->setText(QString::number(m_shortAD1_U2a));
    ui->lineEdit_8->setText(QString::number(m_shortAD1_U2b));
    ui->lineEdit_9->setText(QString::number(m_shortAD1_U2c));
    ui->lineEdit_10->setText(QString::number(m_shortAD1_I2a));
    ui->lineEdit_11->setText(QString::number(m_shortAD1_I2b));
    ui->lineEdit_12->setText(QString::number(m_shortAD1_I2c));

    // 更新线路数据显示
    ui->lineEdit_13->setText(QString::number(m_shortAD1_V_LINE1));
    ui->lineEdit_14->setText(QString::number(m_shortAD1_V_LINE2));
    ui->lineEdit_15->setText(QString::number(m_shortAD1_C_LINE1));
    ui->lineEdit_16->setText(QString::number(m_shortAD1_C_LINE2));

    // 更新DC电压电流显示
    ui->lineEdit_29->setText(QString::number(m_shortAD1_V_DC1));
    ui->lineEdit_31->setText(QString::number(m_shortAD1_V_DC2));
    ui->lineEdit_33->setText(QString::number(m_shortAD1_V_DC3));
    ui->lineEdit_35->setText(QString::number(m_shortAD1_V_DC4));
    ui->lineEdit_37->setText(QString::number(m_shortAD1_V_DC5));
    ui->lineEdit_39->setText(QString::number(m_shortAD1_V_DC6));
    ui->lineEdit_41->setText(QString::number(m_shortAD1_V_DC7));
    ui->lineEdit_43->setText(QString::number(m_shortAD1_V_DC8));

    ui->lineEdit_30->setText(QString::number(m_shortAD1_C_DC1));
    ui->lineEdit_32->setText(QString::number(m_shortAD1_C_DC2));
    ui->lineEdit_34->setText(QString::number(m_shortAD1_C_DC3));
    ui->lineEdit_36->setText(QString::number(m_shortAD1_C_DC4));
    ui->lineEdit_38->setText(QString::number(m_shortAD1_C_DC5));
    ui->lineEdit_40->setText(QString::number(m_shortAD1_C_DC6));
    ui->lineEdit_42->setText(QString::number(m_shortAD1_C_DC7));
    ui->lineEdit_44->setText(QString::number(m_shortAD1_C_DC8));

    // 更新单选按钮状态
    if (m_nAD1_DCSS == 1) // DC1
    {
        ui->radioButton_2->setChecked(1);
    }
    else
    {
        ui->radioButton->setChecked(1);
    }
    if (m_nAD1_DCSS2 == 1) // DC2
    {
        ui->radioButton_4->setChecked(1);
    }
    else
    {
        ui->radioButton_3->setChecked(1);
    }
    if (m_nAD1_DCSS3 == 1) // DC3
    {
        ui->radioButton_6->setChecked(1);
    }
    else
    {
        ui->radioButton_5->setChecked(1);
    }
    if (m_nAD1_DCSS4 == 1) // DC4
    {
        ui->radioButton_8->setChecked(1);
    }
    else
    {
        ui->radioButton_7->setChecked(1);
    }
    if (m_nAD1_DCSS5 == 1) // DC5
    {
        ui->radioButton_10->setChecked(1);
    }
    else
    {
        ui->radioButton_9->setChecked(1);
    }
    if (m_nAD1_DCSS6 == 1) // DC6
    {
        ui->radioButton_12->setChecked(1);
    }
    else
    {
        ui->radioButton_11->setChecked(1);
    }
    if (m_nAD1_DCSS7 == 1) // DC7
    {
        ui->radioButton_14->setChecked(1);
    }
    else
    {
        ui->radioButton_13->setChecked(1);
    }
    if (m_nAD1_DCSS8 == 1) // DC8
    {
        ui->radioButton_16->setChecked(1);
    }
    else
    {
        ui->radioButton_15->setChecked(1);
    }

    // 更新配置参数
    ui->comboBox_5->setCurrentIndex(m_Model_Sec);
    ui->lineEdit_49->setText(QString::number(m_Delay));
    ui->lineEdit_50->setText(QString::number(m_Primary_Current_Scaling));

    // 更新通道选择
    ui->comboBox->setCurrentIndex(m_Channel_Sec1);
    ui->comboBox_2->setCurrentIndex(m_Channel_Sec2);
    ui->comboBox_3->setCurrentIndex(m_Channel_Sec3);
    ui->comboBox_4->setCurrentIndex(m_Channel_SecX);

    // 更新延迟设置
    ui->lineEdit_45->setText(QString::number(m_Delay_A));
    ui->lineEdit_46->setText(QString::number(m_Delay_B));
    ui->lineEdit_47->setText(QString::number(m_Delay_C));
    ui->lineEdit_48->setText(QString::number(m_Delay_X));
}

void Dlg_AD1::onpushButton_3Ad1DefaultClicked() // 默认参数
{
    AD1_FUNCTION = 1027; // IDC_BUTTON_AD1_Default 3
    // 安全获取父窗口(Dlg_ZJM)
    Dlg_ZJM *pParent_ZJM = (Dlg_ZJM *)parent();
    if (!pParent_ZJM)
    {
        QMessageBox::warning(this, u8"错误", u8"无法获取Dlg_ZJM父窗口");
        return;
    }

    // 安全获取祖父窗口(new1Dlg)
    new1Dlg *pParent = (new1Dlg *)pParent_ZJM->parent();
    if (!pParent)
    {
        QMessageBox::warning(this, u8"错误", u8"无法获取new1Dlg祖父窗口");
        return;
    }
    /*
        Dlg_ZJM *pParent_ZJM = new Dlg_ZJM(this);
        if (!pParent_ZJM)
            return;

        new1Dlg *pParent = new new1Dlg(pParent_ZJM);
        if (!pParent)
            return;
    */
    // m_nAD1_SecVNV = 0;

    // DC源选择默认值
    m_nAD1_DCSS = 1;
    m_nAD1_DCSS2 = 1;
    m_nAD1_DCSS3 = 1;
    m_nAD1_DCSS4 = 1;
    m_nAD1_DCSS5 = 1;
    m_nAD1_DCSS6 = 1;
    m_nAD1_DCSS7 = 1;
    m_nAD1_DCSS8 = 1;

    // 通道和模型设置
    m_Channel_Sec1 = 0;
    m_Channel_Sec2 = 1;
    m_Channel_Sec3 = 2;
    m_Channel_SecX = 3;
    m_Model_Sec = 0;

    // 延迟设置
    m_Delay_A = 0;
    m_Delay_B = 0;
    m_Delay_C = 0;
    m_Delay_X = 0;
    m_Delay = 0;

    // 电流比例
    m_Primary_Current_Scaling = 3000;

    if (pParent->DIM_public == 8192)
    {
        m_shortAD1_C_DC1 = DIM_DCS;
        m_shortAD1_C_DC2 = DIM_DCS;
        m_shortAD1_C_DC4 = DIM_DCS;

        m_shortAD1_C_DC5 = DIM_DCS;
        m_shortAD1_C_DC6 = DIM_DCS;
        m_shortAD1_C_DC7 = DIM_DCS;
        m_shortAD1_C_DC8 = DIM_DCS;

        m_shortAD1_V_DC1 = DIM_DCS;
        m_shortAD1_V_DC2 = DIM_DCS;
        m_shortAD1_V_DC3 = DIM_DCS;
        m_shortAD1_V_DC4 = DIM_DCS;

        m_shortAD1_V_DC5 = DIM_DCS;

        m_shortAD1_V_DC6 = DIM_DCS;
        m_shortAD1_V_DC7 = DIM_DCS;
        m_shortAD1_V_DC8 = DIM_DCS;
        m_shortAD1_I1a = DIM_IS;
        m_shortAD1_I1b = DIM_IS;
        m_shortAD1_I1c = DIM_IS;
        m_shortAD1_I2a = DIM_IS;
        m_shortAD1_I2b = DIM_IS;
        m_shortAD1_I2c = DIM_IS;
        m_shortAD1_U1a = DIM_US;
        m_shortAD1_U1b = DIM_US;
        m_shortAD1_U1c = DIM_US;
        m_shortAD1_U2a = DIM_US;
        m_shortAD1_U2b = DIM_US;
        m_shortAD1_U2c = DIM_US;
    }
    else if (pParent->DIM_public == 16384)
    {
        m_shortAD1_C_DC1 = 50360;
        m_shortAD1_C_DC2 = 50360;
        m_shortAD1_C_DC3 = 50360;
        m_shortAD1_C_DC4 = 50360;
        m_shortAD1_C_DC5 = 50360;
        m_shortAD1_C_DC6 = 50360;
        m_shortAD1_C_DC7 = 50360;
        m_shortAD1_C_DC8 = 50360;

        m_shortAD1_V_DC1 = 0;
        m_shortAD1_V_DC2 = 0;
        m_shortAD1_V_DC3 = 0;
        m_shortAD1_V_DC4 = 0;

        m_shortAD1_V_DC5 = 0;

        m_shortAD1_V_DC6 = 0;

        m_shortAD1_V_DC7 = 0;

        m_shortAD1_V_DC8 = 0;

        m_shortAD1_I1a = DIM_I;
        m_shortAD1_I1b = DIM_I;
        m_shortAD1_I1c = DIM_I;
        m_shortAD1_I2a = DIM_I;
        m_shortAD1_I2b = DIM_I;
        m_shortAD1_I2c = DIM_I;
        m_shortAD1_U1a = DIM_U;
        m_shortAD1_U1b = DIM_U;
        m_shortAD1_U1c = DIM_U;
        m_shortAD1_U2a = DIM_U;
        m_shortAD1_U2b = DIM_U;
        m_shortAD1_U2c = DIM_U;
    }
    m_shortAD1_V_LINE1 = 500;
    m_shortAD1_V_LINE2 = 500;
    m_shortAD1_C_LINE1 = 3000;
    m_shortAD1_C_LINE2 = 3000;
    // 更新界面显示
    updateUiFromData();
}

void Dlg_AD1::onpushButton_4AdExitClicked() // 退出
{
    // Get parent widgets

    // 安全获取父窗口(Dlg_ZJM)
    Dlg_ZJM *pParent_ZJM = (Dlg_ZJM *)parent();
    if (!pParent_ZJM)
    {
        QMessageBox::warning(this, u8"错误", u8"无法获取Dlg_ZJM父窗口");
        return;
    }

    // 安全获取祖父窗口(new1Dlg)
    new1Dlg *pParent = (new1Dlg *)pParent_ZJM->parent();
    if (!pParent)
    {
        QMessageBox::warning(this, u8"错误", u8"无法获取new1Dlg祖父窗口");
        return;
    }
    // TODO: Add extra cleanup here

    //  uint Temp_UINT=0;
    unsigned short Temp_short = 0;
    uchar Temp_UCHAR = 0;
    //  BOOL  Temp_BOOL=FALSE;

    Temp_short = m_shortAD1_U1a; // U1a
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[0] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[1] = Temp_UCHAR;

    Temp_short = m_shortAD1_U1b; // U1b
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[2] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[3] = Temp_UCHAR;

    Temp_short = m_shortAD1_U1c; // U1c
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[4] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[5] = Temp_UCHAR;

    Temp_short = m_shortAD1_I1a; // I1a
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[6] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[7] = Temp_UCHAR;

    Temp_short = m_shortAD1_I1b; // I1b
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[8] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[9] = Temp_UCHAR;

    Temp_short = m_shortAD1_I1c; // I1c
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[10] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[11] = Temp_UCHAR;

    Temp_short = m_shortAD1_U2a; // U2a
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[12] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[13] = Temp_UCHAR;

    Temp_short = m_shortAD1_U2b; // U2b
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[14] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[15] = Temp_UCHAR;

    Temp_short = m_shortAD1_U2c; // U2c
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[16] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[17] = Temp_UCHAR;

    Temp_short = m_shortAD1_I2a; // I2a
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[18] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[19] = Temp_UCHAR;

    Temp_short = m_shortAD1_I2b; // I2b
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[20] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[21] = Temp_UCHAR;

    Temp_short = m_shortAD1_I2c; // I2c
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[22] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[23] = Temp_UCHAR;

    Temp_short = m_shortAD1_V_DC1; // V DC1
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[24] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[25] = Temp_UCHAR;

    Temp_short = m_shortAD1_V_DC2; // V DC2
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[26] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[27] = Temp_UCHAR;

    Temp_short = m_shortAD1_V_DC3; // V DC3
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[28] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[29] = Temp_UCHAR;

    Temp_short = m_shortAD1_V_DC4; // V DC4
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[30] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[31] = Temp_UCHAR;

    Temp_short = m_shortAD1_V_DC5; // V DC5
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[32] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[33] = Temp_UCHAR;

    Temp_short = m_shortAD1_V_DC6; // V DC6
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[34] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[35] = Temp_UCHAR;

    Temp_short = m_shortAD1_V_DC7; // V DC7
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[36] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[37] = Temp_UCHAR;

    Temp_short = m_shortAD1_V_DC8; // V DC8
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[38] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[39] = Temp_UCHAR;

    Temp_short = m_shortAD1_C_DC1; // C DC1
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[40] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[41] = Temp_UCHAR;

    Temp_short = m_shortAD1_C_DC2; // C DC2
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[42] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[43] = Temp_UCHAR;

    Temp_short = m_shortAD1_C_DC3; // C DC3
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[44] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[45] = Temp_UCHAR;

    Temp_short = m_shortAD1_C_DC4; // C DC4
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[46] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[47] = Temp_UCHAR;

    Temp_short = m_shortAD1_C_DC5; // C DC5
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[48] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[49] = Temp_UCHAR;

    Temp_short = m_shortAD1_C_DC6; // C DC6
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[50] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[51] = Temp_UCHAR;

    Temp_short = m_shortAD1_C_DC7; // C DC7
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[52] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[53] = Temp_UCHAR;

    Temp_short = m_shortAD1_C_DC8; // C DC8
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[54] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[55] = Temp_UCHAR;

    Temp_short = m_shortAD1_V_LINE1; // V LINE1
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[56] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[57] = Temp_UCHAR;

    Temp_short = m_shortAD1_V_LINE2; // V LINE2
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[58] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[59] = Temp_UCHAR;

    Temp_short = m_shortAD1_C_LINE1; // C LINE1
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[60] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[61] = Temp_UCHAR;

    Temp_short = m_shortAD1_C_LINE2; // C LINE2
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[62] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[63] = Temp_UCHAR;

    Temp_UCHAR = (uchar)m_Model_Sec; // Interpolation Model Select
    DATA[64] = Temp_UCHAR;

    Temp_short = m_Delay; // Common Delay
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[65] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[66] = Temp_UCHAR;

    Temp_UCHAR = (uchar)(m_Primary_Current_Scaling / 25); // Scaling
    DATA[67] = Temp_UCHAR;
    pParent->Current_Style = m_nAD1_SecVNV;

    Temp_UCHAR = (uchar)m_nAD1_SecVNV; // Second Voltage Nom Value
    DATA[68] = Temp_UCHAR;

    Temp_UCHAR = (uchar)m_nAD1_DCSS; // DC1 Source Select
    DATA[69] = Temp_UCHAR;
    pParent->DC_V_I_select[0] = m_nAD1_DCSS;

    Temp_UCHAR = (uchar)m_nAD1_DCSS2; // DC2 Source Select
    DATA[70] = Temp_UCHAR;
    pParent->DC_V_I_select[1] = m_nAD1_DCSS2;

    Temp_UCHAR = (uchar)m_nAD1_DCSS3; // DC3 Source Select
    DATA[71] = Temp_UCHAR;
    pParent->DC_V_I_select[2] = m_nAD1_DCSS3;

    Temp_UCHAR = (uchar)m_nAD1_DCSS4; // DC4 Source Select
    DATA[72] = Temp_UCHAR;
    pParent->DC_V_I_select[3] = m_nAD1_DCSS4;

    Temp_UCHAR = (uchar)m_nAD1_DCSS5; // DC5 Source Select
    DATA[73] = Temp_UCHAR;
    pParent->DC_V_I_select[4] = m_nAD1_DCSS5;

    Temp_UCHAR = (uchar)m_nAD1_DCSS6; // DC6 Source Select
    DATA[74] = Temp_UCHAR;
    pParent->DC_V_I_select[5] = m_nAD1_DCSS6;

    Temp_UCHAR = (uchar)m_nAD1_DCSS7; // DC7 Source Select
    DATA[75] = Temp_UCHAR;
    pParent->DC_V_I_select[6] = m_nAD1_DCSS7;

    Temp_UCHAR = (uchar)m_nAD1_DCSS8; // DC8 Source Select
    DATA[76] = Temp_UCHAR;
    pParent->DC_V_I_select[7] = m_nAD1_DCSS8;

    Temp_UCHAR = (uchar)m_Channel_Sec1; // Channel Seclect IA
    DATA[77] = Temp_UCHAR;

    Temp_UCHAR = (uchar)m_Channel_Sec2; // Channel Seclect IB
    DATA[78] = Temp_UCHAR;

    Temp_UCHAR = (uchar)m_Channel_Sec3; // Channel Seclect IC
    DATA[79] = Temp_UCHAR;

    Temp_UCHAR = (uchar)m_Channel_SecX; // Channel Seclect IX
    DATA[80] = Temp_UCHAR;

    Temp_short = m_Delay_A; // Channel Delay IA
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[81] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[82] = Temp_UCHAR;

    Temp_short = m_Delay_B; // Channel Delay IB
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[83] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[84] = Temp_UCHAR;

    Temp_short = m_Delay_C; // Channel Delay IC
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[85] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[86] = Temp_UCHAR;

    Temp_short = m_Delay_X; // Channel Delay IX
    Temp_UCHAR = Temp_short & 0x00ff;
    DATA[87] = Temp_UCHAR;
    Temp_UCHAR = Temp_short >> 8;
    DATA[88] = Temp_UCHAR;

    DATA[89] = 0; // bak
    DATA[90] = '\0';
    this->close();
}

void Dlg_AD1::onradioButtonToggled(bool checked) // dc1 source select
{
    if (checked)
        m_nAD1_DCSS = 0;
}

void Dlg_AD1::onradioButton_2Toggled(bool checked)
{
    if (checked)
        m_nAD1_DCSS = 1;
}

void Dlg_AD1::onradioButton_3Toggled(bool checked) // dc2 source select
{
    if (checked)
        m_nAD1_DCSS2 = 0;
}

void Dlg_AD1::onradioButton_4Toggled(bool checked)
{
    if (checked)
        m_nAD1_DCSS2 = 1;
}

void Dlg_AD1::onradioButton_5Toggled(bool checked) // dc3 source select
{
    if (checked)
        m_nAD1_DCSS3 = 0;
}

void Dlg_AD1::onradioButton_6Toggled(bool checked)
{
    if (checked)
        m_nAD1_DCSS3 = 1;
}

void Dlg_AD1::onradioButton_7Toggled(bool checked) // dc4 source select
{
    if (checked)
        m_nAD1_DCSS4 = 0;
}

void Dlg_AD1::onradioButton_8Toggled(bool checked)
{
    if (checked)
        m_nAD1_DCSS4 = 1;
}

void Dlg_AD1::onradioButton_9Toggled(bool checked) // dc5 source select
{
    if (checked)
        m_nAD1_DCSS5 = 0;
}

void Dlg_AD1::onradioButton_10Toggled(bool checked)
{
    if (checked)
        m_nAD1_DCSS5 = 1;
}

void Dlg_AD1::onradioButton_11Toggled(bool checked) // dc6 source select
{
    if (checked)
        m_nAD1_DCSS6 = 0;
}

void Dlg_AD1::onradioButton_12Toggled(bool checked)
{
    if (checked)
        m_nAD1_DCSS6 = 1;
}
void Dlg_AD1::onradioButton_13Toggled(bool checked) // dc7 source select
{
    if (checked)
        m_nAD1_DCSS7 = 0;
}

void Dlg_AD1::onradioButton_14Toggled(bool checked)
{
    if (checked)
        m_nAD1_DCSS7 = 1;
}

void Dlg_AD1::onradioButton_15Toggled(bool checked) // dc8 source select
{
    if (checked)
        m_nAD1_DCSS8 = 0;
}

void Dlg_AD1::onradioButton_16Toggled(bool checked)
{
    if (checked)
        m_nAD1_DCSS8 = 1;
}

Dlg_AD1::~Dlg_AD1()
{
    delete ui;
}
