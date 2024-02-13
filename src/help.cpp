#include "help.h"
#include "../ui/ui_help.h"

Help::Help(const QString& msg, QWidget* parent)
    : QDialog(parent),
      _ui(new Ui::Help)
{
    _ui->setupUi(this);
    _ui->helpBrowser->setPlainText(msg);
}

Help::~Help()
{
    delete _ui;
}
