#ifndef LOGGIN_H
#define LOGGIN_H

#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>
#include <QStackedWidget>
#include <QCoreApplication>
#include <QAbstractEventDispatcher>

QT_BEGIN_NAMESPACE
namespace Ui {
class loggin;
}
QT_END_NAMESPACE

class loggin : public QWidget
{
    Q_OBJECT

public:
    loggin(QWidget *parent = nullptr);
    ~loggin();
    void ini();
    void createBttonn();
    void setWidgetShaddow();
    void closeAndMin(QWidget* shadowWin);
    void setupUI();
    void changeWin(QStackedWidget* stackWidget , int index);

private:
    Ui::loggin *ui;
    QPushButton* minimizeButton;
    QPushButton* closeButton;
    QVBoxLayout* mainLayout;
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;


protected:

signals:
    void successLoggin();

private slots:
    void on_pushButton_login_clicked();
};
#endif // LOGGIN_H
