#include "DLg_DIMset.h"
#include "ui_DLg_DIMset.h"
#include "Dlg_ZJM.h"
#include "new1Dlg.h"
#include "qmessagebox.h"
#include <QDebug>
#include <QScreen>

DLg_DIMset::DLg_DIMset(QWidget *parent) : QWidget(parent), ui(new Ui::DLg_DIMset)
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
    setWindowTitle(u8"标幺设置");
    DIMsetConnections();
}

void DLg_DIMset::DIMsetConnections()
{
    connect(ui->radioButton, &QRadioButton::toggled, this, &DLg_DIMset::onRadio4000HToggled);
    connect(ui->radioButton_2, &QRadioButton::toggled, this, &DLg_DIMset::onRadio2000HToggled);
    connect(ui->radioButton_3, &QRadioButton::toggled, this, &DLg_DIMset::onRadio3Toggled);     // 相电压和线电压的标幺值
    connect(ui->radioButton_4, &QRadioButton::toggled, this, &DLg_DIMset::onRadio4Toggled);     // 相电压标幺设置
    connect(ui->pushButton, &QPushButton::clicked, this, &DLg_DIMset::onButtonOKClicked);       // 默认按钮
    connect(ui->pushButton_2, &QPushButton::clicked, this, &DLg_DIMset::onButtonCancelClicked); // 确认
}

void DLg_DIMset::onRadio4000HToggled()
{
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
    pParent->DIM_public = 16384;
    m_public_dim_select = 0;
}

void DLg_DIMset::onRadio2000HToggled()
{
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
    pParent->DIM_public = 8192;
    m_public_dim_select = 1;
}

void DLg_DIMset::onRadio3Toggled()
{
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
    pParent->DIM_Line_Vot_style = 0;
    m_line_volt_view_select = 0;
}

void DLg_DIMset::onRadio4Toggled()
{
    // TODO: Add your control notification handler code here
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
    pParent->DIM_Line_Vot_style = 1;
    m_line_volt_view_select = 1;
}

void DLg_DIMset::onButtonOKClicked()
{
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
    pParent->DIM_public = 16384;
    pParent->DIM_Line_Vot_style = 0;

    m_public_dim_select = 0;
    m_line_volt_view_select = 0;
    ui->radioButton->setChecked(1);
    ui->radioButton_3->setChecked(1);
}

void DLg_DIMset::onButtonCancelClicked()
{
    DIMsetConnections();
    this->close();
}

DLg_DIMset::~DLg_DIMset()
{
    delete ui;
}
