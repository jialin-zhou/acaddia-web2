#include "Dlg_ADAdjust1.h"
#include "ui_Dlg_ADAdjust1.h"
#include <QMessageBox>
#include "Dlg_ZJM.h"
#include "new1Dlg.h"
#include <QGroupBox>
#include <QScreen>
Dlg_ADAdjust1::Dlg_ADAdjust1(QWidget *parent) : QWidget(parent), ui(new Ui::Dlg_ADAdjust1)
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
    setWindowTitle(u8"通道校准");

    m_boolADAdjust_Ia_OK = false;
    m_boolADAdjust_Ib_OK = false;
    m_boolADAdjust_Ic_OK = false;
    m_boolADAdjust_Ua_OK = false;
    m_boolADAdjust_Ub_OK = false;
    m_boolADAdjust_Uc_OK = false;

    m_nADAdjust_ACChannel = -1;
    m_nADAdjust_Channel = -1;
    m_nADAdjust_DCChannel = -1;

    m_floatADAdjust_Uc_1 = 0.0f;
    m_floatADAdjust_Ub_1 = 0.0f;
    m_floatADAdjust_Ua_1 = 0.0f;
    m_floatADAdjust_Ic_1 = 0.0f;
    m_floatADAdjust_Ib_1 = 0.0f;
    m_floatADAdjust_Ia_1 = 0.0f;

    CHECK_STATUS = 0;
    update_LineEdit();
    // 初始打开界面初始化
    m_nADAdjust_Channel = 0;
    control();

    // Initialize channel data containers
    // Line1.resize(6);
    // Line2.resize(6);
    // DC.resize(16); // Increased to accommodate more channels
    // line1\line2\dc
    connect(ui->radioButton, &QRadioButton::toggled, this, &Dlg_ADAdjust1::onRadioChannelline1Toggled);   // line1
    connect(ui->radioButton_2, &QRadioButton::toggled, this, &Dlg_ADAdjust1::onRadioChannelLine2Toggled); // line2
    connect(ui->radioButton_3, &QRadioButton::toggled, this, &Dlg_ADAdjust1::onRadioChannelDCToggled);    // DC

    // line1 and line2 Ua等radioButton
    connect(ui->radioButton_4, &QRadioButton::toggled, this, &Dlg_ADAdjust1::onRadioACChannelUaToggled); // Ua radioButton控件
    connect(ui->radioButton_5, &QRadioButton::toggled, this, &Dlg_ADAdjust1::onRadioACChannelUbToggled); // Ub radioButton控件
    connect(ui->radioButton_6, &QRadioButton::toggled, this, &Dlg_ADAdjust1::onRadioACChannelUcToggled); // Uc radioButton控件
    connect(ui->radioButton_7, &QRadioButton::toggled, this, &Dlg_ADAdjust1::onRadioACChannelIaToggled); // ia radioButton控件
    connect(ui->radioButton_8, &QRadioButton::toggled, this, &Dlg_ADAdjust1::onRadioACChannelIbToggled); // ia radioButton控件
    connect(ui->radioButton_9, &QRadioButton::toggled, this, &Dlg_ADAdjust1::onRadioACChannelIcToggled); // ia radioButton控件
    // DC
    // 零漂
    connect(ui->radioButton_10, &QRadioButton::toggled, this, &Dlg_ADAdjust1::onRadioDCChannelVDC1Toggled); // DC1
    connect(ui->radioButton_11, &QRadioButton::toggled, this, &Dlg_ADAdjust1::onRadioDCChannelVDC2Toggled);
    connect(ui->radioButton_12, &QRadioButton::toggled, this, &Dlg_ADAdjust1::onRadioDCChannelVDC3Toggled);
    connect(ui->radioButton_13, &QRadioButton::toggled, this, &Dlg_ADAdjust1::onRadioDCChannelVDC4Toggled);
    connect(ui->radioButton_14, &QRadioButton::toggled, this, &Dlg_ADAdjust1::onRadioDCChannelVDC5Toggled);
    connect(ui->radioButton_15, &QRadioButton::toggled, this, &Dlg_ADAdjust1::onRadioDCChannelVDC6Toggled);
    connect(ui->radioButton_16, &QRadioButton::toggled, this, &Dlg_ADAdjust1::onRadioDCChannelVDC7Toggled);
    connect(ui->radioButton_17, &QRadioButton::toggled, this, &Dlg_ADAdjust1::onRadioDCChannelVDC8Toggled);
    // 系数
    connect(ui->radioButton_18, &QRadioButton::toggled, this, &Dlg_ADAdjust1::onRadioDCChannelCDC1Toggled);
    connect(ui->radioButton_19, &QRadioButton::toggled, this, &Dlg_ADAdjust1::onRadioDCChannelCDC2Toggled);
    connect(ui->radioButton_20, &QRadioButton::toggled, this, &Dlg_ADAdjust1::onRadioDCChannelCDC3Toggled);
    connect(ui->radioButton_21, &QRadioButton::toggled, this, &Dlg_ADAdjust1::onRadioDCChannelCDC4Toggled);
    connect(ui->radioButton_22, &QRadioButton::toggled, this, &Dlg_ADAdjust1::onRadioDCChannelCDC5Toggled);
    connect(ui->radioButton_23, &QRadioButton::toggled, this, &Dlg_ADAdjust1::onRadioDCChannelCDC6Toggled);
    connect(ui->radioButton_24, &QRadioButton::toggled, this, &Dlg_ADAdjust1::onRadioDCChannelCDC7Toggled);
    connect(ui->radioButton_25, &QRadioButton::toggled, this, &Dlg_ADAdjust1::onRadioDCChannelCDC8Toggled);

    connect(ui->lineEdit, &QLineEdit::editingFinished, this, &Dlg_ADAdjust1::onEditUa1EditingFinished);
    connect(ui->lineEdit_2, &QLineEdit::editingFinished, this, &Dlg_ADAdjust1::onEditUb1EditingFinished);
    connect(ui->lineEdit_3, &QLineEdit::editingFinished, this, &Dlg_ADAdjust1::onEditUc1EditingFinished);
    connect(ui->lineEdit_4, &QLineEdit::editingFinished, this, &Dlg_ADAdjust1::onEditIa1EditingFinished);
    connect(ui->lineEdit_5, &QLineEdit::editingFinished, this, &Dlg_ADAdjust1::onEditIb1EditingFinished);
    connect(ui->lineEdit_6, &QLineEdit::editingFinished, this, &Dlg_ADAdjust1::onEditIc1EditingFinished);

    connect(ui->pushButton, &QPushButton::clicked, this, &Dlg_ADAdjust1::onButtonApplyClicked);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &Dlg_ADAdjust1::onButtonCancelClicked);
    // connect(ui->buttonADAdjustNewParApply, &QPushButton::clicked, this, &Dlg_ADAdjust1::onButtonNewParApplyClicked);
}
// 数据更新
void Dlg_ADAdjust1::update_LineEdit()
{
    m_floatADAdjust_Ua_1 = ui->lineEdit->text().toFloat();   // Ua
    m_floatADAdjust_Ub_1 = ui->lineEdit_2->text().toFloat(); // Ub
    m_floatADAdjust_Uc_1 = ui->lineEdit_3->text().toFloat(); // Uc
    m_floatADAdjust_Ia_1 = ui->lineEdit_4->text().toFloat(); // Ia
    m_floatADAdjust_Ib_1 = ui->lineEdit_5->text().toFloat(); // Ib
    m_floatADAdjust_Ic_1 = ui->lineEdit_6->text().toFloat(); // Ic

    // m_CStringADAdjust_Ua_2 = ui->lineEdit_7->text().toFloat();//Ua
    // m_CStringADAdjust_Ua_3 = ui->lineEdit_13->text().toFloat();
    // m_CStringADAdjust_Ua_4 = ui->lineEdit_19->text().toFloat();
    // m_CStringADAdjust_Ua_5 = ui->lineEdit_25->text().toFloat();

    // m_CStringADAdjust_Ub_2 = ui->lineEdit_8->text().toFloat();//Ub
    // m_CStringADAdjust_Ub_3 = ui->lineEdit_14->text().toFloat();
    // m_CStringADAdjust_Ub_4 = ui->lineEdit_20->text().toFloat();
    // m_CStringADAdjust_Ub_5 = ui->lineEdit_26->text().toFloat();

    // m_CStringADAdjust_Uc_2 = ui->lineEdit_9->text().toFloat();//Uc
    // m_CStringADAdjust_Uc_3 = ui->lineEdit_15->text().toFloat();
    // m_CStringADAdjust_Uc_4 = ui->lineEdit_21->text().toFloat();
    // m_CStringADAdjust_Uc_5 = ui->lineEdit_27->text().toFloat();

    // m_CStringADAdjust_Ia_2 = ui->lineEdit_10->text().toFloat();//Ia
    // m_CStringADAdjust_Ia_3 = ui->lineEdit_16->text().toFloat();
    // m_CStringADAdjust_Ia_4 = ui->lineEdit_22->text().toFloat();
    // m_CStringADAdjust_Ia_5 = ui->lineEdit_28->text().toFloat();

    // m_CStringADAdjust_Ib_2 = ui->lineEdit_11->text().toFloat();//Ib
    // m_CStringADAdjust_Ib_3 = ui->lineEdit_17->text().toFloat();
    // m_CStringADAdjust_Ib_4 = ui->lineEdit_23->text().toFloat();
    // m_CStringADAdjust_Ib_5 = ui->lineEdit_29->text().toFloat();

    // m_CStringADAdjust_Ic_2 = ui->lineEdit_12->text().toFloat();//Ic
    // m_CStringADAdjust_Ic_3 = ui->lineEdit_18->text().toFloat();
    // m_CStringADAdjust_Ic_4 = ui->lineEdit_24->text().toFloat();
    // m_CStringADAdjust_Ic_5 = ui->lineEdit_30->text().toFloat();

    m_CStringADAdjust_Ua_2 = ui->lineEdit_7->text(); // Ua
    m_CStringADAdjust_Ua_3 = ui->lineEdit_13->text();
    m_CStringADAdjust_Ua_4 = ui->lineEdit_19->text();
    m_CStringADAdjust_Ua_5 = ui->lineEdit_25->text();

    m_CStringADAdjust_Ub_2 = ui->lineEdit_8->text(); // Ub
    m_CStringADAdjust_Ub_3 = ui->lineEdit_14->text();
    m_CStringADAdjust_Ub_4 = ui->lineEdit_20->text();
    m_CStringADAdjust_Ub_5 = ui->lineEdit_26->text();

    m_CStringADAdjust_Uc_2 = ui->lineEdit_9->text(); // Uc
    m_CStringADAdjust_Uc_3 = ui->lineEdit_15->text();
    m_CStringADAdjust_Uc_4 = ui->lineEdit_21->text();
    m_CStringADAdjust_Uc_5 = ui->lineEdit_27->text();

    m_CStringADAdjust_Ia_2 = ui->lineEdit_10->text(); // Ia
    m_CStringADAdjust_Ia_3 = ui->lineEdit_16->text();
    m_CStringADAdjust_Ia_4 = ui->lineEdit_22->text();
    m_CStringADAdjust_Ia_5 = ui->lineEdit_28->text();

    m_CStringADAdjust_Ib_2 = ui->lineEdit_11->text(); // Ib
    m_CStringADAdjust_Ib_3 = ui->lineEdit_17->text();
    m_CStringADAdjust_Ib_4 = ui->lineEdit_23->text();
    m_CStringADAdjust_Ib_5 = ui->lineEdit_29->text();

    m_CStringADAdjust_Ic_2 = ui->lineEdit_12->text(); // Ic
    m_CStringADAdjust_Ic_3 = ui->lineEdit_18->text();
    m_CStringADAdjust_Ic_4 = ui->lineEdit_24->text();
    m_CStringADAdjust_Ic_5 = ui->lineEdit_30->text();
}

void Dlg_ADAdjust1::onRadioChannelline1Toggled() // line1
{
    // TODO: Add your control notification handler code here
    //////////////////////////////////////////////////////////////////////////
    //  Line1 该块完成的工作： 1、保存Line2和 DC 的参数；
    //                         2、装载Line1 的参数；
    //						   3、updatedata；
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //
    //    STEP 1    SAVE PARAMETRER
    //////////////////////////////////////////////////////////////////////////
    Function_ID = 1000;

    //////////////////////////////////////////////////////////////////////////
    //
    //  STEP 2
    //////////////////////////////////////////////////////////////////////////

    m_nADAdjust_Channel = 0;
    m_nADAdjust_DCChannel = -1;
    m_nADAdjust_ACChannel = 0;
    control();
    m_floatADAdjust_Ua_1 = Line1[0].f_para1;
    m_CStringADAdjust_Ua_2 = Line1[0].Cs_para2;
    m_CStringADAdjust_Ua_3 = Line1[0].Cs_para3;
    m_CStringADAdjust_Ua_4 = Line1[0].Cs_para4;
    m_CStringADAdjust_Ua_5 = Line1[0].Cs_para5;
    m_boolADAdjust_Ua_OK = Line1[0].b_paraOK;

    m_floatADAdjust_Ub_1 = Line1[1].f_para1;
    m_CStringADAdjust_Ub_2 = Line1[1].Cs_para2;
    m_CStringADAdjust_Ub_3 = Line1[1].Cs_para3;
    m_CStringADAdjust_Ub_4 = Line1[1].Cs_para4;
    m_CStringADAdjust_Ub_5 = Line1[1].Cs_para5;
    m_boolADAdjust_Ub_OK = Line1[1].b_paraOK;

    m_floatADAdjust_Uc_1 = Line1[2].f_para1;
    m_CStringADAdjust_Uc_2 = Line1[2].Cs_para2;
    m_CStringADAdjust_Uc_3 = Line1[2].Cs_para3;
    m_CStringADAdjust_Uc_4 = Line1[2].Cs_para4;
    m_CStringADAdjust_Uc_5 = Line1[2].Cs_para5;
    m_boolADAdjust_Uc_OK = Line1[2].b_paraOK;

    m_floatADAdjust_Ia_1 = Line1[3].f_para1;
    m_CStringADAdjust_Ia_2 = Line1[3].Cs_para2;
    m_CStringADAdjust_Ia_3 = Line1[3].Cs_para3;
    m_CStringADAdjust_Ia_4 = Line1[3].Cs_para4;
    m_CStringADAdjust_Ia_5 = Line1[3].Cs_para5;
    m_boolADAdjust_Ia_OK = Line1[3].b_paraOK;

    m_floatADAdjust_Ib_1 = Line1[4].f_para1;
    m_CStringADAdjust_Ib_2 = Line1[4].Cs_para2;
    m_CStringADAdjust_Ib_3 = Line1[4].Cs_para3;
    m_CStringADAdjust_Ib_4 = Line1[4].Cs_para4;
    m_CStringADAdjust_Ib_5 = Line1[4].Cs_para5;
    m_boolADAdjust_Ib_OK = Line1[4].b_paraOK;

    m_floatADAdjust_Ic_1 = Line1[5].f_para1;
    m_CStringADAdjust_Ic_2 = Line1[5].Cs_para2;
    m_CStringADAdjust_Ic_3 = Line1[5].Cs_para3;
    m_CStringADAdjust_Ic_4 = Line1[5].Cs_para4;
    m_CStringADAdjust_Ic_5 = Line1[5].Cs_para5;
    m_boolADAdjust_Ic_OK = Line1[5].b_paraOK;
}

void Dlg_ADAdjust1::onRadioChannelLine2Toggled() // LINE2
{
    // TODO: Add your control notification handler code here
    //////////////////////////////////////////////////////////////////////////
    //  Line2 该块完成的工作： 1、保存Line1和 DC 的参数；
    //                         2、装载Line2 的参数；
    //						   3、updatedata；
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //
    //    STEP 1    SAVE PARAMETRER
    //////////////////////////////////////////////////////////////////////////

    Function_ID = 1160;

    //////////////////////////////////////////////////////////////////////////
    //
    //  STEP 2
    //////////////////////////////////////////////////////////////////////////

    m_nADAdjust_Channel = 1;
    m_nADAdjust_ACChannel = 0;
    m_nADAdjust_DCChannel = -1;
    control();
    m_floatADAdjust_Ua_1 = Line2[0].f_para1;
    m_CStringADAdjust_Ua_2 = Line2[0].Cs_para2;
    m_CStringADAdjust_Ua_3 = Line2[0].Cs_para3;
    m_CStringADAdjust_Ua_4 = Line2[0].Cs_para4;
    m_CStringADAdjust_Ua_5 = Line2[0].Cs_para5;
    m_boolADAdjust_Ua_OK = Line2[0].b_paraOK;

    m_floatADAdjust_Ub_1 = Line2[1].f_para1;
    m_CStringADAdjust_Ub_2 = Line2[1].Cs_para2;
    m_CStringADAdjust_Ub_3 = Line2[1].Cs_para3;
    m_CStringADAdjust_Ub_4 = Line2[1].Cs_para4;
    m_CStringADAdjust_Ub_5 = Line2[1].Cs_para5;
    m_boolADAdjust_Ub_OK = Line2[1].b_paraOK;

    m_floatADAdjust_Uc_1 = Line2[2].f_para1;
    m_CStringADAdjust_Uc_2 = Line2[2].Cs_para2;
    m_CStringADAdjust_Uc_3 = Line2[2].Cs_para3;
    m_CStringADAdjust_Uc_4 = Line2[2].Cs_para4;
    m_CStringADAdjust_Uc_5 = Line2[2].Cs_para5;
    m_boolADAdjust_Uc_OK = Line2[2].b_paraOK;

    m_floatADAdjust_Ia_1 = Line2[3].f_para1;
    m_CStringADAdjust_Ia_2 = Line2[3].Cs_para2;
    m_CStringADAdjust_Ia_3 = Line2[3].Cs_para3;
    m_CStringADAdjust_Ia_4 = Line2[3].Cs_para4;
    m_CStringADAdjust_Ia_5 = Line2[3].Cs_para5;
    m_boolADAdjust_Ia_OK = Line2[3].b_paraOK;

    m_floatADAdjust_Ib_1 = Line2[4].f_para1;
    m_CStringADAdjust_Ib_2 = Line2[4].Cs_para2;
    m_CStringADAdjust_Ib_3 = Line2[4].Cs_para3;
    m_CStringADAdjust_Ib_4 = Line2[4].Cs_para4;
    m_CStringADAdjust_Ib_5 = Line2[4].Cs_para5;
    m_boolADAdjust_Ib_OK = Line2[4].b_paraOK;

    m_floatADAdjust_Ic_1 = Line2[5].f_para1;
    m_CStringADAdjust_Ic_2 = Line2[5].Cs_para2;
    m_CStringADAdjust_Ic_3 = Line2[5].Cs_para3;
    m_CStringADAdjust_Ic_4 = Line2[5].Cs_para4;
    m_CStringADAdjust_Ic_5 = Line2[5].Cs_para5;
    m_boolADAdjust_Ic_OK = Line2[5].b_paraOK;
}

void Dlg_ADAdjust1::onRadioChannelDCToggled() // DC
{
    // TODO: Add your control notification handler code here
    //////////////////////////////////////////////////////////////////////////
    //  DC该块完成的工作： 1、保存Line2和 line1 的参数；
    //                         2、装载dc 的参数；
    //						   3、updatedata；
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //
    //    STEP 1    SAVE PARAMETRER
    //////////////////////////////////////////////////////////////////////////
    Function_ID = 1002;
    // update();

    //////////////////////////////////////////////////////////////////////////
    //
    //  STEP 2
    //////////////////////////////////////////////////////////////////////////

    m_nADAdjust_Channel = 2;
    m_nADAdjust_DCChannel = 0;
    m_nADAdjust_ACChannel = -1;
    control();
}
// 控件互斥、逻辑顺序
void Dlg_ADAdjust1::control()
{
    /* if(m_nADAdjust_Channel==0||m_nADAdjust_Channel==1)
     {
         ui->radioButton_4->setChecked(1);
         ui->radioButton_10->setChecked(0);
         ui->radioButton_11->setChecked(0);
         ui->radioButton_12->setChecked(0);
         ui->radioButton_13->setChecked(0);
         ui->radioButton_14->setChecked(0);
         ui->radioButton_15->setChecked(0);
         ui->radioButton_16->setChecked(0);
         ui->radioButton_17->setChecked(0);
         ui->radioButton_18->setChecked(0);
         ui->radioButton_19->setChecked(0);
         ui->radioButton_20->setChecked(0);
         ui->radioButton_21->setChecked(0);
         ui->radioButton_22->setChecked(0);
         ui->radioButton_23->setChecked(0);
         ui->radioButton_24->setChecked(0);
         ui->radioButton_25->setChecked(0);
         ui->radioButton_10->setEnabled(false);
         ui->radioButton_11->setDisabled(1);
         ui->radioButton_12->setDisabled(1);
         ui->radioButton_13->setDisabled(1);
         ui->radioButton_14->setDisabled(1);
         ui->radioButton_15->setDisabled(1);
         ui->radioButton_16->setDisabled(1);
         ui->radioButton_17->setDisabled(1);
         ui->radioButton_18->setDisabled(1);
         ui->radioButton_19->setDisabled(1);
         ui->radioButton_20->setDisabled(1);
         ui->radioButton_21->setDisabled(1);
         ui->radioButton_22->setDisabled(1);
         ui->radioButton_23->setDisabled(1);
         ui->radioButton_24->setDisabled(1);
         ui->radioButton_25->setDisabled(1);
         ui->radioButton_4->setDisabled(0);
         ui->radioButton_5->setDisabled(0);
         ui->radioButton_6->setDisabled(0);
         ui->radioButton_7->setDisabled(0);
         ui->radioButton_8->setDisabled(0);
         ui->radioButton_9->setDisabled(0);

     }
     else
     {
         ui->radioButton_10->setDisabled(0);
         ui->radioButton_11->setDisabled(0);
         ui->radioButton_12->setDisabled(0);
         ui->radioButton_13->setDisabled(0);
         ui->radioButton_14->setDisabled(0);
         ui->radioButton_15->setDisabled(0);
         ui->radioButton_16->setDisabled(0);
         ui->radioButton_17->setDisabled(0);
         ui->radioButton_18->setDisabled(0);
         ui->radioButton_19->setDisabled(0);
         ui->radioButton_20->setDisabled(0);
         ui->radioButton_21->setDisabled(0);
         ui->radioButton_22->setDisabled(0);
         ui->radioButton_23->setDisabled(0);
         ui->radioButton_24->setDisabled(0);
         ui->radioButton_25->setDisabled(0);
         ui->radioButton_10->setChecked(1);
         ui->radioButton_4->setChecked(0);
         ui->radioButton_5->setChecked(0);
         ui->radioButton_6->setChecked(0);
         ui->radioButton_7->setChecked(0);
         ui->radioButton_8->setChecked(0);
         ui->radioButton_9->setChecked(0);
         ui->radioButton_4->setDisabled(1);
         ui->radioButton_5->setDisabled(1);
         ui->radioButton_6->setDisabled(1);
         ui->radioButton_7->setDisabled(1);
         ui->radioButton_8->setDisabled(1);
         ui->radioButton_9->setDisabled(1);
     }*/
    QList<QRadioButton *> acRadioButtons = { ui->radioButton_4, ui->radioButton_5, ui->radioButton_6, ui->radioButton_7, ui->radioButton_8, ui->radioButton_9 };

    QList<QRadioButton *> dcRadioButtons = { ui->radioButton_10, ui->radioButton_11, ui->radioButton_12, ui->radioButton_13, ui->radioButton_14,
        ui->radioButton_15, ui->radioButton_16, ui->radioButton_17, ui->radioButton_18, ui->radioButton_19, ui->radioButton_20, ui->radioButton_21,
        ui->radioButton_22, ui->radioButton_23, ui->radioButton_24, ui->radioButton_25 };

    if (m_nADAdjust_Channel == 0 || m_nADAdjust_Channel == 1)
    {
        // AC通道模式
        ui->radioButton_4->setChecked(true);
        // 取消互斥
        setRadioButtonsExclusived(dcRadioButtons, false);
        // 取消选中所有DC通道单选按钮
        setRadioButtonsChecked(dcRadioButtons, false);
        // 恢复互斥
        setRadioButtonsExclusived(dcRadioButtons, true);

        // 启用AC通道单选按钮，禁用DC通道
        setRadioButtonsEnabled(acRadioButtons, true);
        setRadioButtonsEnabled(dcRadioButtons, false);
    }
    else
    {
        // DC通道模式
        ui->radioButton_10->setChecked(true);
        // 取消互斥
        setRadioButtonsExclusived(acRadioButtons, false);
        // 取消选中所有AC通道单选按钮
        setRadioButtonsChecked(acRadioButtons, false);
        // 恢复互斥
        setRadioButtonsExclusived(acRadioButtons, true);
        // 启用DC通道单选按钮，禁用AC通道
        setRadioButtonsEnabled(dcRadioButtons, true);
        setRadioButtonsEnabled(acRadioButtons, false);
    }
}

// 辅助函数：设置一组单选按钮的启用/禁用状态
void Dlg_ADAdjust1::setRadioButtonsEnabled(const QList<QRadioButton *> &buttons, bool enabled)
{
    for (QRadioButton *button : buttons)
    {
        button->setEnabled(enabled);
    }
}

// 辅助函数：设置一组单选按钮的选中状态
void Dlg_ADAdjust1::setRadioButtonsChecked(const QList<QRadioButton *> &buttons, bool checked)
{
    for (QRadioButton *button : buttons)
    {
        button->setChecked(checked);
    }
}
// 辅助函数：设置一组单选按钮的启用/禁用状态
void Dlg_ADAdjust1::setRadioButtonsExclusived(const QList<QRadioButton *> &buttons, bool Exclusived)
{
    for (QRadioButton *button : buttons)
    {
        button->setAutoExclusive(Exclusived);
    }
}

void Dlg_ADAdjust1::onRadioACChannelUaToggled() // Ua
{
    if (m_nADAdjust_Channel == 0 || m_nADAdjust_Channel == 1)
    {
        m_nADAdjust_ACChannel = 0;
    }
    else if (m_nADAdjust_Channel == 2)
    {
        m_nADAdjust_ACChannel = -1;
    }
}

void Dlg_ADAdjust1::onRadioACChannelUbToggled() // Ub
{
    if (m_nADAdjust_Channel == 0 || m_nADAdjust_Channel == 1)
        m_nADAdjust_ACChannel = 1;
    else
        m_nADAdjust_ACChannel = -1;
}

void Dlg_ADAdjust1::onRadioACChannelUcToggled() // Uc
{
    if (m_nADAdjust_Channel == 0 || m_nADAdjust_Channel == 1)
        m_nADAdjust_ACChannel = 2;
    else
        m_nADAdjust_ACChannel = -1;
}

void Dlg_ADAdjust1::onRadioACChannelIaToggled() // Ia
{
    if (m_nADAdjust_Channel == 0 || m_nADAdjust_Channel == 1)
        m_nADAdjust_ACChannel = 3;
    else
        m_nADAdjust_ACChannel = -1;
}

void Dlg_ADAdjust1::onRadioACChannelIbToggled() // Ib
{
    if (m_nADAdjust_Channel == 0 || m_nADAdjust_Channel == 1)
        m_nADAdjust_ACChannel = 4;
    else
        m_nADAdjust_ACChannel = -1;
}

void Dlg_ADAdjust1::onRadioACChannelIcToggled() // Ic
{
    if (m_nADAdjust_Channel == 0 || m_nADAdjust_Channel == 1)
        m_nADAdjust_ACChannel = 5;
    else
        m_nADAdjust_ACChannel = -1;
}
// DC
// 零漂
void Dlg_ADAdjust1::onRadioDCChannelVDC1Toggled() // DC1
{
    if (m_nADAdjust_Channel == 2)
        m_nADAdjust_DCChannel = 0;
    else
        m_nADAdjust_DCChannel = -1;
}

void Dlg_ADAdjust1::onRadioDCChannelVDC2Toggled() // DC2
{
    if (m_nADAdjust_Channel == 2)
        m_nADAdjust_DCChannel = 1;
    else
        m_nADAdjust_DCChannel = -1;
}

void Dlg_ADAdjust1::onRadioDCChannelVDC3Toggled() // DC3
{
    if (m_nADAdjust_Channel == 2)
        m_nADAdjust_DCChannel = 2;
    else
        m_nADAdjust_DCChannel = -1;
}

void Dlg_ADAdjust1::onRadioDCChannelVDC4Toggled() // DC4
{
    if (m_nADAdjust_Channel == 2)
        m_nADAdjust_DCChannel = 3;
    else
        m_nADAdjust_DCChannel = -1;
}

void Dlg_ADAdjust1::onRadioDCChannelVDC5Toggled() // DC5
{
    if (m_nADAdjust_Channel == 2)
        m_nADAdjust_DCChannel = 8;
    else
        m_nADAdjust_DCChannel = -1;
}

void Dlg_ADAdjust1::onRadioDCChannelVDC6Toggled() // DC6
{
    if (m_nADAdjust_Channel == 2)
        m_nADAdjust_DCChannel = 9;
    else
        m_nADAdjust_DCChannel = -1;
}

void Dlg_ADAdjust1::onRadioDCChannelVDC7Toggled() // DC7
{
    if (m_nADAdjust_Channel == 2)
        m_nADAdjust_DCChannel = 10;
    else
        m_nADAdjust_DCChannel = -1;
}

void Dlg_ADAdjust1::onRadioDCChannelVDC8Toggled() // DC8
{
    if (m_nADAdjust_Channel == 2)
        m_nADAdjust_DCChannel = 11;
    else
        m_nADAdjust_DCChannel = -1;
}

// DC
// 系数
void Dlg_ADAdjust1::onRadioDCChannelCDC1Toggled() // DC1
{
    if (m_nADAdjust_Channel == 2)
        m_nADAdjust_DCChannel = 4;
    else
        m_nADAdjust_DCChannel = -1;
}

void Dlg_ADAdjust1::onRadioDCChannelCDC2Toggled() // DC2
{
    if (m_nADAdjust_Channel == 2)
        m_nADAdjust_DCChannel = 5;
    else
        m_nADAdjust_DCChannel = -1;
}

void Dlg_ADAdjust1::onRadioDCChannelCDC3Toggled() // DC3
{
    if (m_nADAdjust_Channel == 2)
        m_nADAdjust_DCChannel = 6;
    else
        m_nADAdjust_DCChannel = -1;
}

void Dlg_ADAdjust1::onRadioDCChannelCDC4Toggled() // DC4
{
    if (m_nADAdjust_Channel == 2)
        m_nADAdjust_DCChannel = 7;
    else
        m_nADAdjust_DCChannel = -1;
}
void Dlg_ADAdjust1::onRadioDCChannelCDC5Toggled() // DC5
{
    if (m_nADAdjust_Channel == 2)
        m_nADAdjust_DCChannel = 12;
    else
        m_nADAdjust_DCChannel = -1;
}
void Dlg_ADAdjust1::onRadioDCChannelCDC6Toggled() // DC6
{
    if (m_nADAdjust_Channel == 2)
        m_nADAdjust_DCChannel = 13;
    else
        m_nADAdjust_DCChannel = -1;
}
void Dlg_ADAdjust1::onRadioDCChannelCDC7Toggled() // DC7
{
    if (m_nADAdjust_Channel == 2)
        m_nADAdjust_DCChannel = 14;
    else
        m_nADAdjust_DCChannel = -1;
}
void Dlg_ADAdjust1::onRadioDCChannelCDC8Toggled() // DC8
{
    if (m_nADAdjust_Channel == 2)
        m_nADAdjust_DCChannel = 15;
    else
        m_nADAdjust_DCChannel = -1;
}

void Dlg_ADAdjust1::onEditUa1EditingFinished()
{
    if (m_nADAdjust_ACChannel == 0)
    {
        m_CStringADAdjust_Ua_5 = FormChang(m_floatADAdjust_Ua_1);
    }
}

void Dlg_ADAdjust1::onEditUb1EditingFinished()
{
    if (m_nADAdjust_ACChannel == 1)
    {
        m_CStringADAdjust_Ub_5 = FormChang(m_floatADAdjust_Ub_1);
    }
}

void Dlg_ADAdjust1::onEditUc1EditingFinished()
{
    if (m_nADAdjust_ACChannel == 2)
    {
        m_CStringADAdjust_Uc_5 = FormChang(m_floatADAdjust_Uc_1);
    }
}

void Dlg_ADAdjust1::onEditIa1EditingFinished()
{
    if (m_nADAdjust_ACChannel == 3)
    {
        m_CStringADAdjust_Ia_5 = FormChang(m_floatADAdjust_Ia_1);
    }
}

void Dlg_ADAdjust1::onEditIb1EditingFinished()
{
    if (m_nADAdjust_ACChannel == 4)
    {
        m_CStringADAdjust_Ib_5 = FormChang(m_floatADAdjust_Ib_1);
    }
}

void Dlg_ADAdjust1::onEditIc1EditingFinished()
{
    if (m_nADAdjust_ACChannel == 5)
    {
        m_CStringADAdjust_Ic_5 = FormChang(m_floatADAdjust_Ic_1);
    }
}

void Dlg_ADAdjust1::onButtonApplyClicked() // 通道校准
{
    uchar DATA[5];
    uchar Temp_UCHAR;
    unsigned short Temp_short;
    float Temp_float;

    Function_ID = 1090; // IDC_BUTTON_ADAdjust_Apply 3;

    update_LineEdit();

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
    pParent->Down = 1090;

    //////////////////////////////////////////////////////////////////////////
    //
    Line1[0].Index = 0;
    Line2[0].Index = 0;
    //	pParent->ADAdjust_block_38.Data
    if (m_nADAdjust_Channel == 0 || m_nADAdjust_Channel == 1)
    {
        Temp_UCHAR = (m_nADAdjust_Channel * 6) + m_nADAdjust_ACChannel;
    }
    else
    {
        Temp_UCHAR = 12 + m_nADAdjust_DCChannel;
    }
    DATA[0] = Temp_UCHAR + 1;

    if (m_nADAdjust_ACChannel == 0)
    {
        m_boolADAdjust_Ua_OK = false; // CLEAR THE CHECK SYMBOL   2008.1.23
        Temp_float = m_floatADAdjust_Ua_1;
        Temp_short = (Temp_float * pParent->DIM_public) / 57.74;
    }
    if (m_nADAdjust_ACChannel == 1)
    {
        m_boolADAdjust_Ub_OK = false; // CLEAR THE CHECK SYMBOL   2008.1.23
        Temp_float = m_floatADAdjust_Ub_1;
        Temp_short = (Temp_float * pParent->DIM_public) / 57.74;
    }
    if (m_nADAdjust_ACChannel == 2)
    {
        m_boolADAdjust_Uc_OK = false; // CLEAR THE CHECK SYMBOL   2008.1.23
        Temp_float = m_floatADAdjust_Uc_1;
        Temp_short = (Temp_float * pParent->DIM_public) / 57.74;
    }
    if (m_nADAdjust_ACChannel == 3)
    {
        m_boolADAdjust_Ia_OK = false; // CLEAR THE CHECK SYMBOL   2008.1.23
        Temp_float = m_floatADAdjust_Ia_1;
        /*		Temp_short = (Temp_float/5)*8192;*/
        if (pParent->Current_Style == 0)
        {
            Temp_short = (Temp_float * pParent->DIM_public) / 5;
        }
        else if (pParent->Current_Style == 1)
        {
            Temp_short = (Temp_float * pParent->DIM_public) / 1;
        }
    }
    if (m_nADAdjust_ACChannel == 4)
    {
        m_boolADAdjust_Ib_OK = false; // CLEAR THE CHECK SYMBOL   2008.1.23
        Temp_float = m_floatADAdjust_Ib_1;
        if (pParent->Current_Style == 0)
        {
            Temp_short = (Temp_float * pParent->DIM_public) / 5;
        }
        else if (pParent->Current_Style == 1)
        {
            Temp_short = (Temp_float * pParent->DIM_public) / 1;
        }
    }
    if (m_nADAdjust_ACChannel == 5)
    {
        m_boolADAdjust_Ic_OK = false; // CLEAR THE CHECK SYMBOL   2008.1.23
        Temp_float = m_floatADAdjust_Ic_1;
        if (pParent->Current_Style == 0)
        {
            Temp_short = (Temp_float * pParent->DIM_public) / 5;
        }
        else if (pParent->Current_Style == 1)
        {
            Temp_short = (Temp_float * pParent->DIM_public) / 1;
        }
    }
    if (m_nADAdjust_DCChannel == 0)
    {
        Temp_short = (Temp_float * pParent->DIM_public) / 2.5;
    }
    if (m_nADAdjust_DCChannel == 1)
    {
        Temp_short = (Temp_float * pParent->DIM_public) / 2.5;
    }
    if (m_nADAdjust_DCChannel == 2)
    {
        Temp_short = (Temp_float * pParent->DIM_public) / 2.5;
    }
    if (m_nADAdjust_DCChannel == 3)
    {
        Temp_short = (Temp_float * pParent->DIM_public) / 2.5;
    }
    if (m_nADAdjust_DCChannel == 4)
    {
        Temp_short = (Temp_float * pParent->DIM_public) / 10;
    }
    if (m_nADAdjust_DCChannel == 5)
    {
        Temp_short = (Temp_float * pParent->DIM_public) / 10;
    }
    if (m_nADAdjust_DCChannel == 6)
    {
        Temp_short = (Temp_float * pParent->DIM_public) / 10;
    }
    if (m_nADAdjust_DCChannel == 7)
    {
        Temp_short = (Temp_float * pParent->DIM_public) / 10;
    }

    if (m_nADAdjust_DCChannel == 8)
    {
        Temp_short = (Temp_float * pParent->DIM_public) / 2.5;
    }
    if (m_nADAdjust_DCChannel == 9)
    {
        Temp_short = (Temp_float * pParent->DIM_public) / 2.5;
    }
    if (m_nADAdjust_DCChannel == 10)
    {
        Temp_short = (Temp_float * pParent->DIM_public) / 2.5;
    }
    if (m_nADAdjust_DCChannel == 11)
    {
        Temp_short = (Temp_float * pParent->DIM_public) / 2.5;
    }
    if (m_nADAdjust_DCChannel == 12)
    {
        Temp_short = (Temp_float * pParent->DIM_public) / 10;
    }
    if (m_nADAdjust_DCChannel == 13)
    {
        Temp_short = (Temp_float * pParent->DIM_public) / 10;
    }
    if (m_nADAdjust_DCChannel == 14)
    {
        Temp_short = (Temp_float * pParent->DIM_public) / 10;
    }
    if (m_nADAdjust_DCChannel == 15)
    {
        Temp_short = (Temp_float * pParent->DIM_public) / 10;
    }

    DATA[1] = Temp_short & 0xff;
    DATA[2] = Temp_short >> 8;
    DATA[3] = 0;
    DATA[4] = '\0';

    pParent->Connect_Telegram.PACK_SELECT = true;
    pParent->Zero_NR = 0;
    for (int tt = 0; tt < 4; tt++)
    {
        pParent->Connect_Telegram.ValidDataToPack[tt] = DATA[tt];
        if (DATA[tt] == 0)
            pParent->Zero_NR++;
    }
    //	Zero_NR--;
    pParent->Zero_NR--;
    pParent->ADAdjust_block_38.Station_adr = 0;
    pParent->ADAdjust_block_38.Telegram_Nr = 38;
    pParent->ADAdjust_block_38.F1_field = 0;
    pParent->ADAdjust_block_38.F2_field = 0x80;
    pParent->Fun_Connect(pParent->ADAdjust_block_38, pParent->Zero_NR);
}
// Line 转化成16进制
QString Dlg_ADAdjust1::FormChang(float INPUT_float)
{
    unsigned short Temp_short = 0;
    uint Temp_UINT;
    uchar Temp_UCHAR;
    char Temp_Buffer[10];
    bool Temp_BOOL;
    float Temp_float;
    QString Temp_CString;

    // 安全获取父窗口(Dlg_ZJM)
    Dlg_ZJM *pParent_ZJM = (Dlg_ZJM *)parent();
    if (!pParent_ZJM)
    {
        QMessageBox::warning(this, u8"错误", u8"无法获取Dlg_ZJM父窗口");
    }

    // 安全获取祖父窗口(new1Dlg)
    new1Dlg *pParent = (new1Dlg *)pParent_ZJM->parent();
    if (!pParent)
    {
        QMessageBox::warning(this, u8"错误", u8"无法获取new1Dlg祖父窗口");
    }

    if (m_nADAdjust_ACChannel == 0 || m_nADAdjust_ACChannel == 1 || m_nADAdjust_ACChannel == 2)
    {
        Temp_short = (INPUT_float * pParent->DIM_public) / 57.74;
        /* _itoa(Temp_short,Temp_Buffer,16);
         Temp_CString =Temp_Buffer;
         Temp_UINT = Temp_CString.length();
         Temp_Buffer[Temp_UINT]='H';
         Temp_Buffer[Temp_UINT+1]='\0';
         Temp_CString =Temp_Buffer;
         Temp_CString.toUpper();*/
        return QString::number(Temp_short, 16).toUpper() + "H";
    }
    else if (m_nADAdjust_DCChannel == 0 || m_nADAdjust_DCChannel == 1 || m_nADAdjust_DCChannel == 2 || m_nADAdjust_DCChannel == 3 ||
             m_nADAdjust_DCChannel == 8 || m_nADAdjust_DCChannel == 9 || m_nADAdjust_DCChannel == 10 || m_nADAdjust_DCChannel == 11) // Voltage
    {
        Temp_short = (INPUT_float * pParent->DIM_public) / 2.5;
        _itoa_s(Temp_short, Temp_Buffer, 16);
        Temp_CString = Temp_Buffer;
        Temp_UINT = Temp_CString.length();
        Temp_Buffer[Temp_UINT] = 'H';
        Temp_Buffer[Temp_UINT + 1] = '\0';
        Temp_CString = Temp_Buffer;
        Temp_CString.toUpper();
    }
    else if (m_nADAdjust_ACChannel == 3 || m_nADAdjust_ACChannel == 4 || m_nADAdjust_ACChannel == 5) // Current
    {
        if (pParent->Current_Style == 0)
        {
            Temp_short = (INPUT_float * pParent->DIM_public) / 5;
        }
        else if (pParent->Current_Style == 1)
        {
            Temp_short = (INPUT_float * pParent->DIM_public) / 1;
        }
        _itoa_s(Temp_short, Temp_Buffer, 16);
        Temp_CString = Temp_Buffer;
        //		Temp_CString.MakeUpper();
        Temp_UINT = Temp_CString.length();
        Temp_Buffer[Temp_UINT] = 'H';
        Temp_Buffer[Temp_UINT + 1] = '\0';
        Temp_CString = Temp_Buffer;
        Temp_CString.toUpper();
    }
    else if (m_nADAdjust_DCChannel == 4 || m_nADAdjust_DCChannel == 5 || m_nADAdjust_DCChannel == 6 || m_nADAdjust_DCChannel == 7 ||
             m_nADAdjust_DCChannel == 12 || m_nADAdjust_DCChannel == 13 || m_nADAdjust_DCChannel == 14 || m_nADAdjust_DCChannel == 15) // Current
    {
        Temp_short = (INPUT_float * pParent->DIM_public) / 10;
        _itoa_s(Temp_short, Temp_Buffer, 16);
        Temp_CString = Temp_Buffer;
        //		Temp_CString.MakeUpper();
        Temp_UINT = Temp_CString.length();
        Temp_Buffer[Temp_UINT] = 'H';
        Temp_Buffer[Temp_UINT + 1] = '\0';
        Temp_CString = Temp_Buffer;
        Temp_CString.toUpper();
    }
    return Temp_CString;
}
// 返回按钮
void Dlg_ADAdjust1::onButtonCancelClicked()
{
    // 关闭窗口，不保存任何修改
    this->close();
}

Dlg_ADAdjust1::~Dlg_ADAdjust1()
{
    delete ui;
}
