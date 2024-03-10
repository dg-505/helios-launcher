#pragma once

#include <QDialog>

namespace Ui
{
    class Help;
} // namespace Ui

class Help : public QDialog
{
        Q_OBJECT // NOLINT

    public:
        Help(const Help&) = delete;
        Help(Help&&) = delete;
        auto operator=(const Help&) -> Help& = delete;
        auto operator=(Help&&) -> Help& = delete;

        explicit Help(const QString& msg, QWidget* parent = nullptr);
        ~Help() override;

    private:
        Ui::Help* _ui;
};
