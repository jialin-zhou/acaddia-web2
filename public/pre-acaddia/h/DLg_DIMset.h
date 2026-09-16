#ifndef DLG_DIMSET_H
#define DLG_DIMSET_H

#include <QWidget>

namespace Ui {
class DLg_DIMset;
}

class DLg_DIMset : public QWidget
{
    Q_OBJECT

public:
    explicit DLg_DIMset(QWidget *parent = nullptr);
    ~DLg_DIMset();

    int	m_public_dim_select;
    int	m_line_volt_view_select;

private:
    Ui::DLg_DIMset *ui;


private slots:
    void DIMsetConnections();
    void onRadio4000HToggled();
    void onRadio2000HToggled();
    void onRadio3Toggled();//相电压和线电压的标幺值
    void onRadio4Toggled();//相电压标幺设置
    void onButtonOKClicked();//默认设置
    void onButtonCancelClicked();
};

#endif // DLG_DIMSET_H
