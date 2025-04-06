#ifndef REG_H
#define REG_H

#include <QStackedWidget>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>
#include <QCoreApplication>
#include <QAbstractEventDispatcher>

namespace Ui {
class reg;
}

class reg : public QWidget
{
    Q_OBJECT

public:
    explicit reg(QWidget *parent = nullptr);
    void setUI();
    void closeAndMin(QWidget *shadowWin);
    void changeWin(QStackedWidget* stackWidget , int index);
    ~reg();

private slots:
    void on_pushButton_login_clicked();

private:
    Ui::reg *ui;
    QPushButton* minimizeButton;
    QPushButton* closeButton;
    QVBoxLayout* mainLayout;
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
};

#endif // REG_H
