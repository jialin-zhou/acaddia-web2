#include "Dlg_BaseAngle.h"
#include "ui_Dlg_BaseAngle.h"
#include "new1Dlg.h"
#include "qmessagebox.h"
#include <QDebug>
#include <QScreen>
Dlg_BaseAngle::Dlg_BaseAngle(QWidget *parent) : QWidget(parent), ui(new Ui::Dlg_BaseAngle)
{
    ui->setupUi(this);
    // 设置窗口标题
    qreal pixelRatio = qApp->devicePixelRatio();
    // 获取逻辑DPI缩放比（考虑系统字体缩放）
    qreal dpiRatio = QGuiApplication::primaryScreen()->logicalDotsPerInch() / 96.0;
    // 综合缩放因子
    qreal scaleFactor = qMin(pixelRatio, dpiRatio);
    QFont font = this->font();                   // 获取基准字体
    font.setPixelSize(qRound(12 * scaleFactor)); // 基于DPI缩放
    this->setFont(font);
    setWindowTitle(u8"角度矢量");
    BaseAngleConnection();
}

void Dlg_BaseAngle::BaseAngleConnection()
{
    connect(ui->pushButton, &QPushButton::clicked, this, &Dlg_BaseAngle::onButtonFetchClicked);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &Dlg_BaseAngle::onButtonCancelClicked);
}

void Dlg_BaseAngle::onButtonFetchClicked()
{
    // TODO: Add your control notification handler code here
    ANGLE_FUNCTION = 1198;
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
    pParent->Down = 1198;

    pParent->AC_angle_block_47.Station_adr = 0;
    pParent->AC_angle_block_47.Telegram_Nr = 47;
    pParent->AC_angle_block_47.F1_field = 0;
    pParent->AC_angle_block_47.F2_field = 0x80;
    pParent->AC_angle_block_47.Data = (uchar *)"";

    pParent->Fun_Connect(pParent->AC_angle_block_47, 0);

    Dlg_BaseAngle_update();
}

void Dlg_BaseAngle::Dlg_BaseAngle_update()
{
    unsigned short TempData_short = 1;
    uchar TempData_UCHAR = 1;
    uint TempData_UINT = 1;
    bool TempData_BOOL = false;
    float TempData_float = 0;

    TempData_UCHAR = ValidData[4]; // UA1
    TempData_short = ValidData[5];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    TempData_float = TempData_short;
    m_float_U1a = TempData_float / 100;

    TempData_UCHAR = ValidData[6]; // UB1
    TempData_short = ValidData[7];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    TempData_float = TempData_short;
    m_float_U1b = TempData_float / 100;

    TempData_UCHAR = ValidData[8]; // UC1
    TempData_short = ValidData[9];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    TempData_float = TempData_short;
    m_float_U1c = TempData_float / 100;

    TempData_UCHAR = ValidData[10]; // IA1
    TempData_short = ValidData[11];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    TempData_float = TempData_short;
    m_float_I1a = TempData_float / 100;

    TempData_UCHAR = ValidData[12]; // IB1
    TempData_short = ValidData[13];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    TempData_float = TempData_short;
    m_float_I1b = TempData_float / 100;

    TempData_UCHAR = ValidData[14]; // IC1
    TempData_short = ValidData[15];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    TempData_float = TempData_short;
    m_float_I1c = TempData_float / 100;

    TempData_UCHAR = ValidData[16]; // UA2
    TempData_short = ValidData[17];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    TempData_float = TempData_short;
    m_float_U2a = TempData_float / 100;

    TempData_UCHAR = ValidData[18]; // UB2
    TempData_short = ValidData[19];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    TempData_float = TempData_short;
    m_float_U2b = TempData_float / 100;

    TempData_UCHAR = ValidData[20]; // UC2
    TempData_short = ValidData[21];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    TempData_float = TempData_short;
    m_float_U2c = TempData_float / 100;

    TempData_UCHAR = ValidData[22]; // IA2
    TempData_short = ValidData[23];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    TempData_float = TempData_short;
    m_float_I2a = TempData_float / 100;

    TempData_UCHAR = ValidData[24]; // IB2
    TempData_short = ValidData[25];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    TempData_float = TempData_short;
    m_float_I2b = TempData_float / 100;

    TempData_UCHAR = ValidData[26]; // IC2
    TempData_short = ValidData[27];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    TempData_float = TempData_short;
    m_float_I2c = TempData_float / 100;

    TempData_UCHAR = ValidData[28]; // da
    TempData_short = ValidData[29];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    TempData_float = TempData_short;
    m_float_da = TempData_float / 100;

    lineEdit_show();
}

void Dlg_BaseAngle::lineEdit_show()
{
    ui->lineEdit->setText(QString::number(m_float_U1a));
    ui->lineEdit_2->setText(QString::number(m_float_U1b));
    ui->lineEdit_3->setText(QString::number(m_float_U1c));
    ui->lineEdit_4->setText(QString::number(m_float_I1a));
    ui->lineEdit_5->setText(QString::number(m_float_I1b));
    ui->lineEdit_6->setText(QString::number(m_float_I1c));
    ui->lineEdit_7->setText(QString::number(m_float_U2a));
    ui->lineEdit_8->setText(QString::number(m_float_U2b));
    ui->lineEdit_9->setText(QString::number(m_float_U2c));
    ui->lineEdit_10->setText(QString::number(m_float_I2a));
    ui->lineEdit_11->setText(QString::number(m_float_I2b));
    ui->lineEdit_12->setText(QString::number(m_float_I2c));
    ui->lineEdit_13->setText(QString::number(m_float_da));
}

void Dlg_BaseAngle::onButtonCancelClicked()
{
    // 关闭窗口，不保存任何修改
    this->close();
}

Dlg_BaseAngle::~Dlg_BaseAngle()
{
    delete ui;
}
