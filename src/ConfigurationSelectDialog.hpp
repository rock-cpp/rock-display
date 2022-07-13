#pragma once

#include <QDialog>
#include <QScopedPointer>
#include <vector>
#include <string>

namespace Ui
{
class ConfigurationSelectDialog;
}

/**
 * @todo write docs
 */
class ConfigurationSelectDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * Constructor
     *
     * @param parent TODO
     * @param f TODO
     */
    ConfigurationSelectDialog(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    virtual ~ConfigurationSelectDialog() override;

    void setSelectionOptions(std::vector<std::string> const &options);
    std::vector<std::string> getSelectionOptions();

private:
    QScopedPointer<Ui::ConfigurationSelectDialog> m_ui;
};

