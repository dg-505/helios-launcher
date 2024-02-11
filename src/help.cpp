#include "help.h"
#include "../ui/ui_help.h"

HelpDialog::HelpDialog(const QString& msg, QWidget* parent)
    : QDialog(parent),
      _ui(new Ui::HelpDialog)
{
    _ui->setupUi(this);
    _ui->helpBrowser->setPlainText(msg);
}

HelpDialog::~HelpDialog()
{
    delete _ui;
}
