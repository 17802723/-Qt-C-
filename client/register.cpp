#include "register.h"
#include "ui_register.h"

register ::register(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::register)
{
    ui->setupUi(this);
}

register ::~register()
{
    delete ui;
}
