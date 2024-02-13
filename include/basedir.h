#pragma once

#include <QDialog>

namespace Ui
{
    class BaseDir;
} // namespace Ui

class BaseDir : public QDialog
{
        Q_OBJECT

    public:
        BaseDir(const BaseDir&) = delete;
        BaseDir(BaseDir&&) = delete;
        auto operator=(const BaseDir&) -> BaseDir& = delete;
        auto operator=(BaseDir&&) -> BaseDir& = delete;

        explicit BaseDir(QWidget* parent = nullptr);
        ~BaseDir() override;

        [[nodiscard]] auto getBaseDir() const -> QString;

    public Q_SLOTS:
        void browse();
        void proceed();

    private:
        Ui::BaseDir* _ui;
};
