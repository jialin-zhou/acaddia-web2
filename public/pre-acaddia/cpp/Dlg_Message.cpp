#include "Dlg_Message.h"
#include "ui_Dlg_Message.h"
#include "Dlg_ZJM.h"
#include "new1Dlg.h"
#include "qmessagebox.h"
#include <QDebug>
#include <QScreen>
Dlg_Message::Dlg_Message(QWidget *parent) : QWidget(parent), ui(new Ui::Dlg_Message)
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
    setWindowTitle(u8"通信报文");
    MessageConnections();
    m_Message_DataInvalid = 1;
    m_Message_Interpolation = 1;
    m_Message_Mode = 0;
    m_Message_OutputType = 0;
    m_Message_PHHealth = 0;
    m_Message_RangeFlag = 0;
    m_Message_SysInvalid = 0;
    if (ui->radioButton_2->isCheckable())
    {
        m_Message_DEC = 1;
        onRadioButton_DEC_Clicked();
    }
}

void Dlg_Message::MessageConnections()
{
    connect(ui->radioButton_2, &QRadioButton::clicked, this, &Dlg_Message::onRadioButton_DEC_Clicked);
    connect(ui->radioButton, &QRadioButton::clicked, this, &Dlg_Message::onRadioButton_HEX_Clicked);
    connect(ui->pushButton, &QPushButton::clicked, this, &Dlg_Message::OnBUTTONMessageFetch);    // 获取参数
    connect(ui->pushButton_2, &QPushButton::clicked, this, &Dlg_Message::onButtonCancelClicked); // 退出
}

void Dlg_Message::OnBUTTONMessageFetch()
{
    Message_FUNCTION = 1277;
    // Dlg_ZJM * pParent_ZJM = (Dlg_ZJM *)GetParent();
    // pParent->
    // CNew1Dlg * pParent =(CNew1Dlg *)pParent_ZJM->GetParent();
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
    pParent->Down = 1277;

    pParent->Message_block_49.Station_adr = 0;
    pParent->Message_block_49.Telegram_Nr = 49;
    pParent->Message_block_49.F1_field = 0;
    pParent->Message_block_49.F2_field = 0x80;
    pParent->Message_block_49.Data = (uchar *)"";

    pParent->Fun_Connect(pParent->Message_block_49, 0);

    // UpdateData(FALSE);
    // ValidData[4]=ValidData[5]=0;ValidData[6]=ValidData[7]=0;ValidData[8]=ValidData[9]=0;
    // SetTimer(1277,666,NULL);
    // pParent->Down =IDC_BUTTON_Message_FETCH;
    Fetch_data();
}

void Dlg_Message::Fetch_data()
{
    unsigned short TempData_short = 1;
    uchar TempData_UCHAR = 1;
    uint TempData_UINT = 1;
    bool TempData_BOOL = false;
    float TempData_float = 0;
    int buffer[1000];

    TempData_UCHAR = ValidData[4]; // Lenth
    TempData_short = ValidData[5];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    char Cnt_TempBuffer[10];
    buffer[0] = TempData_short;
    if (m_Message_DEC == 0)
    {
        _itoa_s(TempData_short, Cnt_TempBuffer, 10);
        m_Message_Lenth = Cnt_TempBuffer;
    }
    else
    {
        int dec = TempData_short;
        _itoa_s(dec, Cnt_TempBuffer, 16);

        int i, n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Lenth = "0x";
        int j;
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Lenth += "0";
            }
        }
        m_Message_Lenth += Cnt_TempBuffer;
    }

    TempData_UCHAR = ValidData[6]; // LNName
    TempData_UINT = (uint)TempData_UCHAR;
    memset(Cnt_TempBuffer, '\0', 10);
    buffer[1] = TempData_UINT;
    if (m_Message_DEC == 0)
    {
        _itoa_s(TempData_UINT, Cnt_TempBuffer, 10);
        m_Message_LNName = Cnt_TempBuffer;
    }
    else
    {
        int dec = TempData_UINT;
        _itoa_s(dec, Cnt_TempBuffer, 16);

        int i, n = 0;
        for (i = 1; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_LNName = "0x";
        int j;
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_LNName += "0";
            }
        }
        m_Message_LNName += Cnt_TempBuffer;
    }

    TempData_UCHAR = ValidData[7]; // DataSetName
    TempData_UINT = (uint)TempData_UCHAR;
    memset(Cnt_TempBuffer, '\0', 10);
    buffer[2] = TempData_UINT;
    if (m_Message_DEC == 0)
    {
        _itoa_s(TempData_UINT, Cnt_TempBuffer, 10);
        m_Message_DataSetName = Cnt_TempBuffer;
    }
    else
    {
        int dec = TempData_UINT;
        _itoa_s(dec, Cnt_TempBuffer, 16);

        int i, n = 0;
        for (i = 1; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_DataSetName = "0x";
        int j;
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_DataSetName += "0";
            }
        }
        m_Message_DataSetName += Cnt_TempBuffer;
    }

    TempData_UCHAR = ValidData[8]; // LDName
    TempData_short = ValidData[9];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    memset(Cnt_TempBuffer, '\0', 10);
    buffer[3] = TempData_short;
    if (m_Message_DEC == 0)
    {
        _itoa_s(TempData_short, Cnt_TempBuffer, 10);
        m_Message_LDName = Cnt_TempBuffer;
    }
    else
    {
        int dec = TempData_short;
        _itoa_s(dec, Cnt_TempBuffer, 16);

        int i, n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_LDName = "0x";
        int j;
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_LDName += "0";
            }
        }
        m_Message_LDName += Cnt_TempBuffer;
    }

    TempData_UCHAR = ValidData[10]; // m_Message_PhsAArtg
    TempData_short = ValidData[11];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    memset(Cnt_TempBuffer, '\0', 10);
    buffer[4] = TempData_short;
    if (m_Message_DEC == 0)
    {
        _itoa_s(TempData_short, Cnt_TempBuffer, 10);
        m_Message_PhsAArtg = Cnt_TempBuffer;
    }
    else
    {
        int dec = TempData_short;
        _itoa_s(dec, Cnt_TempBuffer, 16);

        int i, n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_PhsAArtg = "0x";
        int j;
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_PhsAArtg += "0";
            }
        }
        m_Message_PhsAArtg += Cnt_TempBuffer;
    }

    TempData_UCHAR = ValidData[12]; // m_Message_NeutArtg
    TempData_short = ValidData[13];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    memset(Cnt_TempBuffer, '\0', 10);
    buffer[5] = TempData_short;
    if (m_Message_DEC == 0)
    {
        _itoa_s(TempData_short, Cnt_TempBuffer, 10);
        m_Message_NeutArtg = Cnt_TempBuffer;
    }
    else
    {
        int dec = TempData_short;
        _itoa_s(dec, Cnt_TempBuffer, 16);

        int i, n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_NeutArtg = "0x";
        int j;
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_NeutArtg += "0";
            }
        }
        m_Message_NeutArtg += Cnt_TempBuffer;
    }

    TempData_UCHAR = ValidData[14]; // m_Message_PhsAVrtg
    TempData_short = ValidData[15];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    memset(Cnt_TempBuffer, '\0', 10);
    buffer[6] = TempData_short;
    if (m_Message_DEC == 0)
    {
        _itoa_s(TempData_short, Cnt_TempBuffer, 10);
        m_Message_PhsAVrtg = Cnt_TempBuffer;
    }
    else
    {
        int dec = TempData_short;
        _itoa_s(dec, Cnt_TempBuffer, 16);

        int i, n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_PhsAVrtg = "0x";
        int j;
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_PhsAVrtg += "0";
            }
        }
        m_Message_PhsAVrtg += Cnt_TempBuffer;
    }

    TempData_UCHAR = ValidData[16]; // m_Message_tdr
    TempData_short = ValidData[17];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    memset(Cnt_TempBuffer, '\0', 10);
    buffer[7] = TempData_short;
    if (m_Message_DEC == 0)
    {
        _itoa_s(TempData_short, Cnt_TempBuffer, 10);
        m_Message_tdr = Cnt_TempBuffer;
    }
    else
    {
        int dec = TempData_short;
        _itoa_s(dec, Cnt_TempBuffer, 16);

        int i, n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_tdr = "0x";
        int j;
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_tdr += "0";
            }
        }
        m_Message_tdr += Cnt_TempBuffer;
    }

    TempData_UCHAR = ValidData[18]; // m_Message_SmpCtr
    TempData_short = ValidData[19];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    memset(Cnt_TempBuffer, '\0', 10);
    buffer[8] = TempData_short;
    if (m_Message_DEC == 0)
    {
        _itoa_s(TempData_short, Cnt_TempBuffer, 10);
        m_Message_SmpCtr = Cnt_TempBuffer;
    }
    else
    {
        int dec = TempData_short;
        _itoa_s(dec, Cnt_TempBuffer, 16);

        int i, n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_SmpCtr = "0x";
        int j;
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_SmpCtr += "0";
            }
        }
        m_Message_SmpCtr += Cnt_TempBuffer;
    }

    TempData_UCHAR = ValidData[20]; // Channel1
    TempData_short = ValidData[21];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    memset(Cnt_TempBuffer, '\0', 10);

    buffer[9] = TempData_short;
    if (m_Message_DEC == 0)
    {
        _itoa_s(TempData_short, Cnt_TempBuffer, 10);
        m_Message_Channel1 = Cnt_TempBuffer;
    }
    else
    {
        int dec = TempData_short;
        _itoa_s(dec, Cnt_TempBuffer, 16);

        int i, n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel1 = "0x";
        int j;
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel1 += "0";
            }
        }
        m_Message_Channel1 += Cnt_TempBuffer;
    }

    TempData_UCHAR = ValidData[22]; // Channel2
    TempData_short = ValidData[23];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    memset(Cnt_TempBuffer, '\0', 10);
    buffer[10] = TempData_short;
    if (m_Message_DEC == 0)
    {
        _itoa_s(TempData_short, Cnt_TempBuffer, 10);
        m_Message_Channel2 = Cnt_TempBuffer;
    }
    else
    {
        int dec = TempData_short;
        _itoa_s(dec, Cnt_TempBuffer, 16);

        int i, n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel2 = "0x";
        int j;
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel2 += "0";
            }
        }
        m_Message_Channel2 += Cnt_TempBuffer;
    }

    TempData_UCHAR = ValidData[24]; // Channel3
    TempData_short = ValidData[25];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    memset(Cnt_TempBuffer, '\0', 10);
    buffer[11] = TempData_short;
    if (m_Message_DEC == 0)
    {
        _itoa_s(TempData_short, Cnt_TempBuffer, 10);
        m_Message_Channel3 = Cnt_TempBuffer;
    }
    else
    {
        int dec = TempData_short;
        _itoa_s(dec, Cnt_TempBuffer, 16);

        int i, n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel3 = "0x";
        int j;
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel3 += "0";
            }
        }
        m_Message_Channel3 += Cnt_TempBuffer;
    }

    TempData_UCHAR = ValidData[26]; // Channel4
    TempData_short = ValidData[27];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    buffer[12] = TempData_short;
    if (m_Message_DEC == 0)
    {
        _itoa_s(TempData_short, Cnt_TempBuffer, 10);
        m_Message_Channel4 = Cnt_TempBuffer;
    }
    else
    {
        int dec = TempData_short;
        _itoa_s(dec, Cnt_TempBuffer, 16);

        int i, n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel4 = "0x";
        int j;
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel4 += "0";
            }
        }
        m_Message_Channel4 += Cnt_TempBuffer;
    }

    TempData_UCHAR = ValidData[28]; // Channel5
    TempData_short = ValidData[29];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    memset(Cnt_TempBuffer, '\0', 10);
    buffer[13] = TempData_short;
    if (m_Message_DEC == 0)
    {
        _itoa_s(TempData_short, Cnt_TempBuffer, 10);
        m_Message_Channel5 = Cnt_TempBuffer;
    }
    else
    {
        int dec = TempData_short;
        _itoa_s(dec, Cnt_TempBuffer, 16);

        int i, n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel5 = "0x";
        int j;
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel5 += "0";
            }
        }
        m_Message_Channel5 += Cnt_TempBuffer;
    }

    TempData_UCHAR = ValidData[30]; // Channel6
    TempData_short = ValidData[31];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    memset(Cnt_TempBuffer, '\0', 10);
    buffer[14] = TempData_short;
    if (m_Message_DEC == 0)
    {
        _itoa_s(TempData_short, Cnt_TempBuffer, 10);
        m_Message_Channel6 = Cnt_TempBuffer;
    }
    else
    {
        int dec = TempData_short;
        _itoa_s(dec, Cnt_TempBuffer, 16);

        int i, n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel6 = "0x";
        int j;
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel6 += "0";
            }
        }
        m_Message_Channel6 += Cnt_TempBuffer;
    }

    TempData_UCHAR = ValidData[32]; // Channel7
    TempData_short = ValidData[33];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    memset(Cnt_TempBuffer, '\0', 10);
    buffer[15] = TempData_short;
    if (m_Message_DEC == 0)
    {
        _itoa_s(TempData_short, Cnt_TempBuffer, 10);
        m_Message_Channel7 = Cnt_TempBuffer;
    }
    else
    {
        int dec = TempData_short;
        _itoa_s(dec, Cnt_TempBuffer, 16);

        int i, n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel7 = "0x";
        int j;
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel7 += "0";
            }
        }
        m_Message_Channel7 += Cnt_TempBuffer;
    }

    TempData_UCHAR = ValidData[34]; // Channel8
    TempData_short = ValidData[35];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    memset(Cnt_TempBuffer, '\0', 10);
    buffer[16] = TempData_short;
    if (m_Message_DEC == 0)
    {
        _itoa_s(TempData_short, Cnt_TempBuffer, 10);
        m_Message_Channel8 = Cnt_TempBuffer;
    }
    else
    {
        int dec = TempData_short;
        _itoa_s(dec, Cnt_TempBuffer, 16);

        int i, n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel8 = "0x";
        int j;
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel8 += "0";
            }
        }
        m_Message_Channel8 += Cnt_TempBuffer;
    }

    TempData_UCHAR = ValidData[36]; // Channel9
    TempData_short = ValidData[37];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    memset(Cnt_TempBuffer, '\0', 10);
    buffer[17] = TempData_short;
    if (m_Message_DEC == 0)
    {
        _itoa_s(TempData_short, Cnt_TempBuffer, 10);
        m_Message_Channel9 = Cnt_TempBuffer;
    }
    else
    {
        int dec = TempData_short;
        _itoa_s(dec, Cnt_TempBuffer, 16);

        int i, n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel9 = "0x";
        int j;
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel9 += "0";
            }
        }
        m_Message_Channel9 += Cnt_TempBuffer;
    }

    TempData_UCHAR = ValidData[38]; // Channel10
    TempData_short = ValidData[39];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    memset(Cnt_TempBuffer, '\0', 10);
    buffer[18] = TempData_short;
    if (m_Message_DEC == 0)
    {
        _itoa_s(TempData_short, Cnt_TempBuffer, 10);
        m_Message_Channel10 = Cnt_TempBuffer;
    }
    else
    {
        int dec = TempData_short;
        _itoa_s(dec, Cnt_TempBuffer, 16);

        int i, n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel10 = "0x";
        int j;
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel10 += "0";
            }
        }
        m_Message_Channel10 += Cnt_TempBuffer;
    }

    TempData_UCHAR = ValidData[40]; // Channel11
    TempData_short = ValidData[41];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    memset(Cnt_TempBuffer, '\0', 10);
    buffer[19] = TempData_short;
    if (m_Message_DEC == 0)
    {
        _itoa_s(TempData_short, Cnt_TempBuffer, 10);
        m_Message_Channel11 = Cnt_TempBuffer;
    }
    else
    {
        int dec = TempData_short;
        _itoa_s(dec, Cnt_TempBuffer, 16);

        int i, n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel11 = "0x";
        int j;
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel11 += "0";
            }
        }
        m_Message_Channel11 += Cnt_TempBuffer;
    }

    TempData_UCHAR = ValidData[42]; // Channel12
    TempData_short = ValidData[43];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    memset(Cnt_TempBuffer, '\0', 10);
    buffer[20] = TempData_short;
    if (m_Message_DEC == 0)
    {
        _itoa_s(TempData_short, Cnt_TempBuffer, 10);
        m_Message_Channel12 = Cnt_TempBuffer;
    }
    else
    {
        int dec = TempData_short;
        _itoa_s(dec, Cnt_TempBuffer, 16);

        int i, n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel12 = "0x";
        int j;
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel12 += "0";
            }
        }
        m_Message_Channel12 += Cnt_TempBuffer;
    }

    TempData_UCHAR = ValidData[44]; // Channel13
    TempData_short = ValidData[45];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    memset(Cnt_TempBuffer, '\0', 10);
    buffer[21] = TempData_short;
    if (m_Message_DEC == 0)
    {
        _itoa_s(TempData_short, Cnt_TempBuffer, 10);
        m_Message_Channel13 = Cnt_TempBuffer;
    }
    else
    {
        int dec = TempData_short;
        _itoa_s(dec, Cnt_TempBuffer, 16);

        int i, n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel13 = "0x";
        int j;
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel13 += "0";
            }
        }
        m_Message_Channel13 += Cnt_TempBuffer;
    }

    TempData_UCHAR = ValidData[46]; // Channel14
    TempData_short = ValidData[47];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    memset(Cnt_TempBuffer, '\0', 10);
    buffer[22] = TempData_short;
    if (m_Message_DEC == 0)
    {
        _itoa_s(TempData_short, Cnt_TempBuffer, 10);
        m_Message_Channel14 = Cnt_TempBuffer;
    }
    else
    {
        int dec = TempData_short;
        _itoa_s(dec, Cnt_TempBuffer, 16);

        int i, n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel14 = "0x";
        int j;
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel14 += "0";
            }
        }
        m_Message_Channel14 += Cnt_TempBuffer;
    }

    TempData_UCHAR = ValidData[48]; // Channel15
    TempData_short = ValidData[49];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    memset(Cnt_TempBuffer, '\0', 10);
    buffer[23] = TempData_short;
    if (m_Message_DEC == 0)
    {
        _itoa_s(TempData_short, Cnt_TempBuffer, 10);
        m_Message_Channel15 = Cnt_TempBuffer;
    }
    else
    {
        int dec = TempData_short;
        _itoa_s(dec, Cnt_TempBuffer, 16);

        int i, n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel15 = "0x";
        int j;
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel15 += "0";
            }
        }
        m_Message_Channel15 += Cnt_TempBuffer;
    }

    TempData_UCHAR = ValidData[50]; // Channel16
    TempData_short = ValidData[51];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    memset(Cnt_TempBuffer, '\0', 10);
    buffer[24] = TempData_short;
    if (m_Message_DEC == 0)
    {
        _itoa_s(TempData_short, Cnt_TempBuffer, 10);
        m_Message_Channel16 = Cnt_TempBuffer;
    }
    else
    {
        int dec = TempData_short;
        _itoa_s(dec, Cnt_TempBuffer, 16);

        int i, n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel16 = "0x";
        int j;
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel16 += "0";
            }
        }
        m_Message_Channel16 += Cnt_TempBuffer;
    }

    TempData_UCHAR = ValidData[52]; // Channel17
    TempData_short = ValidData[53];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    memset(Cnt_TempBuffer, '\0', 10);
    buffer[25] = TempData_short;
    if (m_Message_DEC == 0)
    {
        _itoa_s(TempData_short, Cnt_TempBuffer, 10);
        m_Message_Channel17 = Cnt_TempBuffer;
    }
    else
    {
        int dec = TempData_short;
        _itoa_s(dec, Cnt_TempBuffer, 16);

        int i, n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel17 = "0x";
        int j;
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel17 += "0";
            }
        }
        m_Message_Channel17 += Cnt_TempBuffer;
    }

    TempData_UCHAR = ValidData[54]; // Channel18
    TempData_short = ValidData[55];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    memset(Cnt_TempBuffer, '\0', 10);
    buffer[26] = TempData_short;
    if (m_Message_DEC == 0)
    {
        _itoa_s(TempData_short, Cnt_TempBuffer, 10);
        m_Message_Channel18 = Cnt_TempBuffer;
    }
    else
    {
        int dec = TempData_short;
        _itoa_s(dec, Cnt_TempBuffer, 16);

        int i, n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel18 = "0x";
        int j;
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel18 += "0";
            }
        }
        m_Message_Channel18 += Cnt_TempBuffer;
    }

    TempData_UCHAR = ValidData[56]; // Channel19
    TempData_short = ValidData[57];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    memset(Cnt_TempBuffer, '\0', 10);
    buffer[27] = TempData_short;
    if (m_Message_DEC == 0)
    {
        _itoa_s(TempData_short, Cnt_TempBuffer, 10);
        m_Message_Channel19 = Cnt_TempBuffer;
    }
    else
    {
        int dec = TempData_short;
        _itoa_s(dec, Cnt_TempBuffer, 16);

        int i, n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel19 = "0x";
        int j;
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel19 += "0";
            }
        }
        m_Message_Channel19 += Cnt_TempBuffer;
    }

    TempData_UCHAR = ValidData[58]; // Channel20
    TempData_short = ValidData[59];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    memset(Cnt_TempBuffer, '\0', 10);
    buffer[28] = TempData_short;
    if (m_Message_DEC == 0)
    {
        _itoa_s(TempData_short, Cnt_TempBuffer, 10);
        m_Message_Channel20 = Cnt_TempBuffer;
    }
    else
    {
        int dec = TempData_short;
        _itoa_s(dec, Cnt_TempBuffer, 16);

        int i, n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel20 = "0x";
        int j;
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel20 += "0";
            }
        }
        m_Message_Channel20 += Cnt_TempBuffer;
    }

    TempData_UCHAR = ValidData[60]; // Channel21
    TempData_short = ValidData[61];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    memset(Cnt_TempBuffer, '\0', 10);
    buffer[29] = TempData_short;
    if (m_Message_DEC == 0)
    {
        _itoa_s(TempData_short, Cnt_TempBuffer, 10);
        m_Message_Channel21 = Cnt_TempBuffer;
    }
    else
    {
        int dec = TempData_short;
        _itoa_s(dec, Cnt_TempBuffer, 16);

        int i, n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel21 = "0x";
        int j;
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel21 += "0";
            }
        }
        m_Message_Channel21 += Cnt_TempBuffer;
    }

    TempData_UCHAR = ValidData[62]; // Channel22
    TempData_short = ValidData[63];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    memset(Cnt_TempBuffer, '\0', 10);
    buffer[30] = TempData_short;
    if (m_Message_DEC == 0)
    {
        _itoa_s(TempData_short, Cnt_TempBuffer, 10);
        m_Message_Channel22 = Cnt_TempBuffer;
    }
    else
    {
        int dec = TempData_short;
        _itoa_s(dec, Cnt_TempBuffer, 16);

        int i, n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel22 = "0x";
        int j;
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel22 += "0";
            }
        }
        m_Message_Channel22 += Cnt_TempBuffer;
    }

    TempData_UCHAR = ValidData[64]; // m_Message_StatusWord1
    TempData_short = ValidData[65];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    memset(Cnt_TempBuffer, '\0', 10);
    buffer[31] = TempData_short;
    if (m_Message_DEC == 0)
    {
        _itoa_s(TempData_short, Cnt_TempBuffer, 10);
        m_Message_StatusWord1 = Cnt_TempBuffer;
    }
    else
    {
        int dec = TempData_short;
        _itoa_s(dec, Cnt_TempBuffer, 16);

        int i, n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_StatusWord1 = "0x";
        int j;
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_StatusWord1 += "0";
            }
        }
        m_Message_StatusWord1 += Cnt_TempBuffer;
    }

    TempData_UCHAR = ValidData[66]; // m_Message_StatusWord2
    TempData_short = ValidData[67];
    TempData_short = TempData_short << 8;
    TempData_short += TempData_UCHAR;
    memset(Cnt_TempBuffer, '\0', 10);
    buffer[32] = TempData_short;
    if (m_Message_DEC == 0)
    {
        _itoa_s(TempData_short, Cnt_TempBuffer, 10);
        m_Message_StatusWord2 = Cnt_TempBuffer;
    }
    else
    {
        int dec = TempData_short;
        _itoa_s(dec, Cnt_TempBuffer, 16);

        int i, n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_StatusWord2 = "0x";
        int j;
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_StatusWord2 += "0";
            }
        }
        m_Message_StatusWord2 += Cnt_TempBuffer;
    }

    int x, i, a;
    x = buffer[31];
    // for(i=31;i>=0;i--)
    //    x>>i&1;

    a = x & 0x0001;
    m_Message_PHHealth = a;

    a = x & 0x0002;
    m_Message_Mode = a >> 1;

    a = x & 0x0004;
    m_Message_DataInvalid = a >> 2;

    a = x & 0x0008;
    m_Message_Interpolation = a >> 3;

    a = x & 0x0010;
    m_Message_SysInvalid = a >> 4;

    a = x & 0x1000;
    m_Message_OutputType = a >> 12;

    a = x & 0x2000;
    m_Message_RangeFlag = a >> 13;

    message_show_Data();
}

void Dlg_Message::onRadioButton_DEC_Clicked()
{
    if (m_Message_DEC == 1)
    {
        int buffer[1000];
        m_Message_DEC = 0;
        char Cnt_TempBuffer[100];
        bool ok;

        buffer[0] = ui->lineEdit->text().toInt(&ok, 16);
        _itoa_s(buffer[0], Cnt_TempBuffer, 10);
        m_Message_Lenth = Cnt_TempBuffer;

        // memset(Cnt_TempBuffer,'\0',10);
        buffer[1] = ui->lineEdit_2->text().toInt(&ok, 16);
        _itoa_s(buffer[1], Cnt_TempBuffer, 10);
        m_Message_LNName = Cnt_TempBuffer;

        // memset(Cnt_TempBuffer,'\0',10);
        buffer[2] = ui->lineEdit_3->text().toInt(&ok, 16);
        _itoa_s(buffer[2], Cnt_TempBuffer, 10);
        m_Message_DataSetName = Cnt_TempBuffer;

        // memset(Cnt_TempBuffer,'\0',10);
        buffer[3] = ui->lineEdit_4->text().toInt(&ok, 16);
        _itoa_s(buffer[3], Cnt_TempBuffer, 10);
        m_Message_LDName = Cnt_TempBuffer;

        // memset(Cnt_TempBuffer,'\0',10);
        buffer[4] = ui->lineEdit_8->text().toInt(&ok, 16);
        _itoa_s(buffer[4], Cnt_TempBuffer, 10);
        m_Message_PhsAArtg = Cnt_TempBuffer;

        // memset(Cnt_TempBuffer,'\0',10);
        buffer[5] = ui->lineEdit_10->text().toInt(&ok, 16);
        _itoa_s(buffer[5], Cnt_TempBuffer, 10);
        m_Message_NeutArtg = Cnt_TempBuffer;

        // memset(Cnt_TempBuffer,'\0',10);
        buffer[6] = ui->lineEdit_9->text().toInt(&ok, 16);
        _itoa_s(buffer[6], Cnt_TempBuffer, 10);
        m_Message_PhsAVrtg = Cnt_TempBuffer;

        // memset(Cnt_TempBuffer,'\0',10);
        buffer[7] = ui->lineEdit_11->text().toInt(&ok, 16);
        _itoa_s(buffer[7], Cnt_TempBuffer, 10);
        m_Message_tdr = Cnt_TempBuffer;

        // memset(Cnt_TempBuffer,'\0',10);
        buffer[8] = ui->lineEdit_5->text().toInt(&ok, 16);
        _itoa_s(buffer[8], Cnt_TempBuffer, 10);
        m_Message_SmpCtr = Cnt_TempBuffer;

        // memset(Cnt_TempBuffer,'\0',10);
        buffer[9] = ui->lineEdit_12->text().toInt(&ok, 16);
        _itoa_s(buffer[9], Cnt_TempBuffer, 10);
        m_Message_Channel1 = Cnt_TempBuffer;

        // memset(Cnt_TempBuffer,'\0',10);
        buffer[10] = ui->lineEdit_13->text().toInt(&ok, 16);
        _itoa_s(buffer[10], Cnt_TempBuffer, 10);
        m_Message_Channel2 = Cnt_TempBuffer;

        // memset(Cnt_TempBuffer,'\0',10);
        buffer[11] = ui->lineEdit_14->text().toInt(&ok, 16);
        _itoa_s(buffer[11], Cnt_TempBuffer, 10);
        m_Message_Channel3 = Cnt_TempBuffer;

        // memset(Cnt_TempBuffer,'\0',10);
        buffer[12] = ui->lineEdit_15->text().toInt(&ok, 16);
        _itoa_s(buffer[12], Cnt_TempBuffer, 10);
        m_Message_Channel4 = Cnt_TempBuffer;

        // memset(Cnt_TempBuffer,'\0',10);
        buffer[13] = ui->lineEdit_16->text().toInt(&ok, 16);
        _itoa_s(buffer[13], Cnt_TempBuffer, 10);
        m_Message_Channel5 = Cnt_TempBuffer;

        // memset(Cnt_TempBuffer,'\0',10);
        buffer[14] = ui->lineEdit_17->text().toInt(&ok, 16);
        _itoa_s(buffer[14], Cnt_TempBuffer, 10);
        m_Message_Channel6 = Cnt_TempBuffer;

        // memset(Cnt_TempBuffer,'\0',10);
        buffer[15] = ui->lineEdit_18->text().toInt(&ok, 16);
        _itoa_s(buffer[15], Cnt_TempBuffer, 10);
        m_Message_Channel7 = Cnt_TempBuffer;

        // memset(Cnt_TempBuffer,'\0',10);
        buffer[16] = ui->lineEdit_19->text().toInt(&ok, 16);
        _itoa_s(buffer[16], Cnt_TempBuffer, 10);
        m_Message_Channel8 = Cnt_TempBuffer;

        // memset(Cnt_TempBuffer,'\0',10);
        buffer[17] = ui->lineEdit_20->text().toInt(&ok, 16);
        _itoa_s(buffer[17], Cnt_TempBuffer, 10);
        m_Message_Channel9 = Cnt_TempBuffer;

        // memset(Cnt_TempBuffer,'\0',10);
        buffer[18] = ui->lineEdit_21->text().toInt(&ok, 16);
        _itoa_s(buffer[18], Cnt_TempBuffer, 10);
        m_Message_Channel10 = Cnt_TempBuffer;

        // memset(Cnt_TempBuffer,'\0',10);
        buffer[19] = ui->lineEdit_22->text().toInt(&ok, 16);
        _itoa_s(buffer[19], Cnt_TempBuffer, 10);
        m_Message_Channel11 = Cnt_TempBuffer;

        // memset(Cnt_TempBuffer,'\0',10);
        buffer[20] = ui->lineEdit_23->text().toInt(&ok, 16);
        _itoa_s(buffer[20], Cnt_TempBuffer, 10);
        m_Message_Channel12 = Cnt_TempBuffer;

        // memset(Cnt_TempBuffer,'\0',10);
        buffer[21] = ui->lineEdit_24->text().toInt(&ok, 16);
        _itoa_s(buffer[21], Cnt_TempBuffer, 10);
        m_Message_Channel13 = Cnt_TempBuffer;

        // memset(Cnt_TempBuffer,'\0',10);
        buffer[22] = ui->lineEdit_25->text().toInt(&ok, 16);
        _itoa_s(buffer[22], Cnt_TempBuffer, 10);
        m_Message_Channel14 = Cnt_TempBuffer;

        // memset(Cnt_TempBuffer,'\0',10);
        buffer[23] = ui->lineEdit_26->text().toInt(&ok, 16);
        _itoa_s(buffer[23], Cnt_TempBuffer, 10);
        m_Message_Channel15 = Cnt_TempBuffer;

        // memset(Cnt_TempBuffer,'\0',10);
        buffer[24] = ui->lineEdit_27->text().toInt(&ok, 16);
        _itoa_s(buffer[24], Cnt_TempBuffer, 10);
        m_Message_Channel16 = Cnt_TempBuffer;

        // memset(Cnt_TempBuffer,'\0',10);
        buffer[25] = ui->lineEdit_28->text().toInt(&ok, 16);
        _itoa_s(buffer[25], Cnt_TempBuffer, 10);
        m_Message_Channel17 = Cnt_TempBuffer;

        // memset(Cnt_TempBuffer,'\0',10);
        buffer[26] = ui->lineEdit_29->text().toInt(&ok, 16);
        _itoa_s(buffer[26], Cnt_TempBuffer, 10);
        m_Message_Channel18 = Cnt_TempBuffer;

        // memset(Cnt_TempBuffer,'\0',10);
        buffer[27] = ui->lineEdit_30->text().toInt(&ok, 16);
        _itoa_s(buffer[27], Cnt_TempBuffer, 10);
        m_Message_Channel19 = Cnt_TempBuffer;

        // memset(Cnt_TempBuffer,'\0',10);
        buffer[28] = ui->lineEdit_31->text().toInt(&ok, 16);
        _itoa_s(buffer[28], Cnt_TempBuffer, 10);
        m_Message_Channel20 = Cnt_TempBuffer;

        // memset(Cnt_TempBuffer,'\0',10);
        buffer[29] = ui->lineEdit_32->text().toInt(&ok, 16);
        _itoa_s(buffer[29], Cnt_TempBuffer, 10);
        m_Message_Channel21 = Cnt_TempBuffer;

        // memset(Cnt_TempBuffer,'\0',10);
        buffer[30] = ui->lineEdit_33->text().toInt(&ok, 16);
        _itoa_s(buffer[30], Cnt_TempBuffer, 10);
        m_Message_Channel22 = Cnt_TempBuffer;

        // memset(Cnt_TempBuffer,'\0',10);
        buffer[31] = ui->lineEdit_6->text().toInt(&ok, 16);
        _itoa_s(buffer[31], Cnt_TempBuffer, 10);
        m_Message_StatusWord1 = Cnt_TempBuffer;

        // memset(Cnt_TempBuffer,'\0',10);
        buffer[32] = ui->lineEdit_7->text().toInt(&ok, 16);
        _itoa_s(buffer[32], Cnt_TempBuffer, 10);
        m_Message_StatusWord2 = Cnt_TempBuffer;
        if (ok)
        {
            // 成功转换
            qDebug() << "成功转换 ";
        }
        else
        {
            // 转换失败
            qDebug() << "转换失败";
            // 可以显示错误提示或设置默认值
        }

        message_show_Data();
    }
}

void Dlg_Message::onRadioButton_HEX_Clicked()
{
    if (m_Message_DEC == 0)
    {
        m_Message_DEC = 1;

        bool ok;
        m_Message_Channel1 = ui->lineEdit_12->text();
        int dec = m_Message_Channel1.toInt(&ok, 10); // Channel1
        char Cnt_TempBuffer[100];
        _itoa_s(dec, Cnt_TempBuffer, 16);

        int i, n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel1 = "0x";
        int j;
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel1 += "0";
            }
        }
        m_Message_Channel1 += Cnt_TempBuffer;

        m_Message_LDName = ui->lineEdit_4->text();
        dec = m_Message_LDName.toInt(&ok, 10); // LDName
        memset(Cnt_TempBuffer, '\0', 10);
        _itoa_s(dec, Cnt_TempBuffer, 16);
        n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_LDName = "0x";
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_LDName += "0";
            }
        }
        m_Message_LDName += Cnt_TempBuffer;

        m_Message_Channel10 = ui->lineEdit_21->text();
        dec = m_Message_Channel10.toInt(&ok, 10); // Channel10
        memset(Cnt_TempBuffer, '\0', 10);
        _itoa_s(dec, Cnt_TempBuffer, 16);
        n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel10 = "0x";
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel10 += "0";
            }
        }
        m_Message_Channel10 += Cnt_TempBuffer;

        m_Message_Channel11 = ui->lineEdit_22->text();
        dec = m_Message_Channel11.toInt(&ok, 10); // Channel11
        memset(Cnt_TempBuffer, '\0', 10);
        _itoa_s(dec, Cnt_TempBuffer, 16);
        n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel11 = "0x";
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel11 += "0";
            }
        }
        m_Message_Channel11 += Cnt_TempBuffer;

        m_Message_Channel12 = ui->lineEdit_23->text();
        dec = m_Message_Channel12.toInt(&ok, 10); // Channel12
        memset(Cnt_TempBuffer, '\0', 10);
        _itoa_s(dec, Cnt_TempBuffer, 16);
        n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel12 = "0x";
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel12 += "0";
            }
        }
        m_Message_Channel12 += Cnt_TempBuffer;

        m_Message_Channel13 = ui->lineEdit_24->text();
        dec = m_Message_Channel13.toInt(&ok, 10);
        memset(Cnt_TempBuffer, '\0', 10);
        _itoa_s(dec, Cnt_TempBuffer, 16);
        n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel13 = "0x";
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel13 += "0";
            }
        }
        m_Message_Channel13 += Cnt_TempBuffer;

        m_Message_Channel14 = ui->lineEdit_25->text();
        dec = m_Message_Channel14.toInt(&ok, 10);
        memset(Cnt_TempBuffer, '\0', 10);
        _itoa_s(dec, Cnt_TempBuffer, 16);
        n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel14 = "0x";
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel14 += "0";
            }
        }
        m_Message_Channel14 += Cnt_TempBuffer;

        m_Message_Channel15 = ui->lineEdit_26->text();
        dec = m_Message_Channel15.toInt(&ok, 10);
        memset(Cnt_TempBuffer, '\0', 10);
        _itoa_s(dec, Cnt_TempBuffer, 16);
        n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel15 = "0x";
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel15 += "0";
            }
        }
        m_Message_Channel15 += Cnt_TempBuffer;

        m_Message_Channel16 = ui->lineEdit_27->text();
        dec = m_Message_Channel16.toInt(&ok, 10);
        memset(Cnt_TempBuffer, '\0', 10);
        _itoa_s(dec, Cnt_TempBuffer, 16);
        n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel16 = "0x";
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel16 += "0";
            }
        }
        m_Message_Channel16 += Cnt_TempBuffer;

        m_Message_Channel17 = ui->lineEdit_28->text();
        dec = m_Message_Channel17.toInt(&ok, 10);
        memset(Cnt_TempBuffer, '\0', 10);
        _itoa_s(dec, Cnt_TempBuffer, 16);
        n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel17 = "0x";
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel17 += "0";
            }
        }
        m_Message_Channel17 += Cnt_TempBuffer;

        m_Message_Channel18 = ui->lineEdit_29->text();
        dec = m_Message_Channel18.toInt(&ok, 10);
        memset(Cnt_TempBuffer, '\0', 10);
        _itoa_s(dec, Cnt_TempBuffer, 16);
        n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel18 = "0x";
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel18 += "0";
            }
        }
        m_Message_Channel18 += Cnt_TempBuffer;

        m_Message_Channel19 = ui->lineEdit_30->text();
        dec = m_Message_Channel19.toInt(&ok, 10);
        memset(Cnt_TempBuffer, '\0', 10);
        _itoa_s(dec, Cnt_TempBuffer, 16);

        n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel19 = "0x";
        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel19 += "0";
            }
        }
        m_Message_Channel19 += Cnt_TempBuffer;

        m_Message_Channel2 = ui->lineEdit_13->text();
        dec = m_Message_Channel2.toInt(&ok, 10);
        memset(Cnt_TempBuffer, '\0', 10);
        _itoa_s(dec, Cnt_TempBuffer, 16);
        n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel2 = "0x";

        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel2 += "0";
            }
        }
        m_Message_Channel2 += Cnt_TempBuffer;

        m_Message_Channel20 = ui->lineEdit_31->text();
        dec = m_Message_Channel20.toInt(&ok, 10);
        memset(Cnt_TempBuffer, '\0', 10);
        _itoa_s(dec, Cnt_TempBuffer, 16);
        n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel20 = "0x";

        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel20 += "0";
            }
        }
        m_Message_Channel20 += Cnt_TempBuffer;

        m_Message_Channel21 = ui->lineEdit_32->text();
        dec = m_Message_Channel21.toInt(&ok, 10);
        memset(Cnt_TempBuffer, '\0', 10);
        _itoa_s(dec, Cnt_TempBuffer, 16);
        n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel21 = "0x";

        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel21 += "0";
            }
        }
        m_Message_Channel21 += Cnt_TempBuffer;

        m_Message_Channel22 = ui->lineEdit_33->text();
        dec = m_Message_Channel22.toInt(&ok, 10);
        memset(Cnt_TempBuffer, '\0', 10);
        _itoa_s(dec, Cnt_TempBuffer, 16);
        n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel22 = "0x";

        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel22 += "0";
            }
        }
        m_Message_Channel22 += Cnt_TempBuffer;

        m_Message_Channel3 = ui->lineEdit_14->text();
        dec = m_Message_Channel3.toInt(&ok, 10);
        memset(Cnt_TempBuffer, '\0', 10);
        _itoa_s(dec, Cnt_TempBuffer, 16);
        n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel3 = "0x";

        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel3 += "0";
            }
        }
        m_Message_Channel3 += Cnt_TempBuffer;

        m_Message_Channel4 = ui->lineEdit_15->text();
        dec = m_Message_Channel4.toInt(&ok, 10);
        memset(Cnt_TempBuffer, '\0', 10);
        _itoa_s(dec, Cnt_TempBuffer, 16);
        n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel4 = "0x";

        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel4 += "0";
            }
        }
        m_Message_Channel4 += Cnt_TempBuffer;

        m_Message_Channel5 = ui->lineEdit_16->text();
        dec = m_Message_Channel5.toInt(&ok, 10);
        memset(Cnt_TempBuffer, '\0', 10);
        _itoa_s(dec, Cnt_TempBuffer, 16);
        n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel5 = "0x";

        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel5 += "0";
            }
        }
        m_Message_Channel5 += Cnt_TempBuffer;

        m_Message_Channel6 = ui->lineEdit_17->text();
        dec = m_Message_Channel6.toInt(&ok, 10);
        memset(Cnt_TempBuffer, '\0', 10);
        _itoa_s(dec, Cnt_TempBuffer, 16);
        n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel6 = "0x";

        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel6 += "0";
            }
        }
        m_Message_Channel6 += Cnt_TempBuffer;

        m_Message_Channel7 = ui->lineEdit_18->text();
        dec = m_Message_Channel7.toInt(&ok, 10);
        memset(Cnt_TempBuffer, '\0', 10);
        _itoa_s(dec, Cnt_TempBuffer, 16);
        n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel7 = "0x";

        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel7 += "0";
            }
        }
        m_Message_Channel7 += Cnt_TempBuffer;

        m_Message_Channel8 = ui->lineEdit_19->text();
        dec = m_Message_Channel8.toInt(&ok);
        memset(Cnt_TempBuffer, '\0', 10);
        _itoa_s(dec, Cnt_TempBuffer, 16);
        n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel8 = "0x";

        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel8 += "0";
            }
        }
        m_Message_Channel8 += Cnt_TempBuffer;

        m_Message_Channel9 = ui->lineEdit_20->text();
        dec = m_Message_Channel9.toInt(&ok, 10);
        memset(Cnt_TempBuffer, '\0', 10);
        _itoa_s(dec, Cnt_TempBuffer, 16);
        n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Channel9 = "0x";

        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Channel9 += "0";
            }
        }
        m_Message_Channel9 += Cnt_TempBuffer;

        m_Message_DataSetName = ui->lineEdit_3->text();
        dec = m_Message_DataSetName.toInt(&ok, 10); // DataSetName
        memset(Cnt_TempBuffer, '\0', 10);
        _itoa_s(dec, Cnt_TempBuffer, 16);
        n = 0;
        for (i = 1; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_DataSetName = "0x";

        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_DataSetName += "0";
            }
        }
        m_Message_DataSetName += Cnt_TempBuffer;

        m_Message_Lenth = ui->lineEdit->text();
        dec = m_Message_Lenth.toInt(&ok, 10); // Lenth
        memset(Cnt_TempBuffer, '\0', 10);
        _itoa_s(dec, Cnt_TempBuffer, 16);
        n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_Lenth = "0x";

        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_Lenth += "0";
            }
        }
        m_Message_Lenth += Cnt_TempBuffer;

        m_Message_LNName = ui->lineEdit_2->text();
        dec = m_Message_LNName.toInt(&ok, 10); // LNName
        memset(Cnt_TempBuffer, '\0', 10);
        _itoa_s(dec, Cnt_TempBuffer, 16);
        n = 0;
        for (i = 1; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_LNName = "0x";

        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_LNName += "0";
            }
        }
        m_Message_LNName += Cnt_TempBuffer;

        m_Message_NeutArtg = ui->lineEdit_10->text();
        dec = m_Message_NeutArtg.toInt(&ok, 10); // NeutArtg
        memset(Cnt_TempBuffer, '\0', 10);
        _itoa_s(dec, Cnt_TempBuffer, 16);
        n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_NeutArtg = "0x";

        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_NeutArtg += "0";
            }
        }
        m_Message_NeutArtg += Cnt_TempBuffer;

        m_Message_PhsAArtg = ui->lineEdit_8->text();
        dec = m_Message_PhsAArtg.toInt(&ok, 10); // PhsAArtg
        memset(Cnt_TempBuffer, '\0', 10);
        _itoa_s(dec, Cnt_TempBuffer, 16);
        n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_PhsAArtg = "0x";

        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_PhsAArtg += "0";
            }
        }
        m_Message_PhsAArtg += Cnt_TempBuffer;

        m_Message_PhsAVrtg = ui->lineEdit_9->text();
        dec = m_Message_PhsAVrtg.toInt(&ok, 10); // PhsAVrtg
        memset(Cnt_TempBuffer, '\0', 10);
        _itoa_s(dec, Cnt_TempBuffer, 16);
        n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_PhsAVrtg = "0x";

        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_PhsAVrtg += "0";
            }
        }
        m_Message_PhsAVrtg += Cnt_TempBuffer;

        m_Message_SmpCtr = ui->lineEdit_5->text();
        dec = m_Message_SmpCtr.toInt(&ok, 10); // SmpCtr
        memset(Cnt_TempBuffer, '\0', 10);
        _itoa_s(dec, Cnt_TempBuffer, 16);
        n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_SmpCtr = "0x";

        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_SmpCtr += "0";
            }
        }
        m_Message_SmpCtr += Cnt_TempBuffer;

        m_Message_StatusWord1 = ui->lineEdit_6->text();
        dec = m_Message_StatusWord1.toInt(&ok, 10); // StatusWord1
        memset(Cnt_TempBuffer, '\0', 10);
        _itoa_s(dec, Cnt_TempBuffer, 16);
        n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_StatusWord1 = "0x";

        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_StatusWord1 += "0";
            }
        }
        m_Message_StatusWord1 += Cnt_TempBuffer;

        m_Message_StatusWord2 = ui->lineEdit_7->text();
        dec = m_Message_StatusWord2.toInt(&ok, 10); // StatusWord2
        memset(Cnt_TempBuffer, '\0', 10);
        _itoa_s(dec, Cnt_TempBuffer, 16);
        n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_StatusWord2 = "0x";

        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_StatusWord2 += "0";
            }
        }
        m_Message_StatusWord2 += Cnt_TempBuffer;

        m_Message_tdr = ui->lineEdit_11->text();
        dec = m_Message_tdr.toInt(&ok, 10); // tdr
        memset(Cnt_TempBuffer, '\0', 10);
        _itoa_s(dec, Cnt_TempBuffer, 16);
        n = 0;
        for (i = 3; Cnt_TempBuffer[i] == 0 && i >= 0; i--)
        {
            n++;
        }
        m_Message_tdr = "0x";

        if (n > 0)
        {
            for (j = 1; j <= n; j++)
            {
                m_Message_tdr += "0";
            }
        }
        m_Message_tdr += Cnt_TempBuffer;

        if (ok)
        {
            // 成功转换
            qDebug() << "成功转换 ";
        }
        else
        {
            // 转换失败
            qDebug() << "转换失败";
            // 可以显示错误提示或设置默认值
        }
        message_show_Data();
    }
}

void Dlg_Message::message_show_Data()
{
    ui->lineEdit->setText(m_Message_Lenth);
    ui->lineEdit_2->setText(m_Message_LNName);
    ui->lineEdit_3->setText(m_Message_DataSetName);
    ui->lineEdit_4->setText(m_Message_LDName);
    ui->lineEdit_5->setText(m_Message_SmpCtr);
    ui->lineEdit_6->setText(m_Message_StatusWord1);
    ui->lineEdit_7->setText(m_Message_StatusWord2);
    ui->lineEdit_8->setText(m_Message_PhsAArtg);
    ui->lineEdit_9->setText(m_Message_PhsAVrtg);
    ui->lineEdit_10->setText(m_Message_NeutArtg);
    ui->lineEdit_11->setText(m_Message_tdr);

    ui->lineEdit_12->setText(m_Message_Channel1);
    ui->lineEdit_13->setText(m_Message_Channel2);
    ui->lineEdit_14->setText(m_Message_Channel3);
    ui->lineEdit_15->setText(m_Message_Channel4);
    ui->lineEdit_16->setText(m_Message_Channel5);
    ui->lineEdit_17->setText(m_Message_Channel6);
    ui->lineEdit_18->setText(m_Message_Channel7);
    ui->lineEdit_19->setText(m_Message_Channel8);
    ui->lineEdit_20->setText(m_Message_Channel9);
    ui->lineEdit_21->setText(m_Message_Channel10);
    ui->lineEdit_22->setText(m_Message_Channel11);
    ui->lineEdit_23->setText(m_Message_Channel12);

    ui->lineEdit_24->setText(m_Message_Channel13);
    ui->lineEdit_25->setText(m_Message_Channel14);
    ui->lineEdit_26->setText(m_Message_Channel15);
    ui->lineEdit_27->setText(m_Message_Channel16);
    ui->lineEdit_28->setText(m_Message_Channel17);
    ui->lineEdit_29->setText(m_Message_Channel18);
    ui->lineEdit_30->setText(m_Message_Channel19);
    ui->lineEdit_31->setText(m_Message_Channel20);
    ui->lineEdit_32->setText(m_Message_Channel21);
    ui->lineEdit_33->setText(m_Message_Channel22);

    ui->checkBox->setChecked(m_Message_PHHealth);
    ui->checkBox_2->setChecked(m_Message_Mode);
    ui->checkBox_3->setChecked(m_Message_DataInvalid);
    ui->checkBox_4->setChecked(m_Message_Interpolation);
    ui->checkBox_5->setChecked(m_Message_SysInvalid);
    ui->checkBox_6->setChecked(m_Message_OutputType);
    ui->checkBox_7->setChecked(m_Message_RangeFlag);
}

void Dlg_Message::onButtonCancelClicked()
{
    // 关闭窗口，不保存任何修改
    this->close();
}
Dlg_Message::~Dlg_Message()
{
    delete ui;
}
