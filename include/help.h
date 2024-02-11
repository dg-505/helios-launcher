#pragma once

#include <QDialog>

namespace Ui
{
    class HelpDialog;
} // namespace Ui

class HelpDialog : public QDialog
{
        Q_OBJECT

    public:
        HelpDialog(const HelpDialog&) = delete;
        HelpDialog(HelpDialog&&) = delete;
        auto operator=(const HelpDialog&) -> HelpDialog& = delete;
        auto operator=(HelpDialog&&) -> HelpDialog& = delete;

        explicit HelpDialog(const QString& msg, QWidget* parent = nullptr);
        ~HelpDialog() override;

    private:
        Ui::HelpDialog* _ui;
};
