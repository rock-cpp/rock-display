#include "ConfigurationSelectDialog.hpp"
#include "ui_ConfigurationSelectDialog.h"
#include <QPushButton>
#include <QToolButton>

ConfigurationSelectDialog::ConfigurationSelectDialog(QWidget* parent, Qt::WindowFlags f)
    : QDialog(parent, f), m_ui(new Ui::ConfigurationSelectDialog)
{
    m_ui->setupUi(this);

    connect(m_ui->tbtUp, &QToolButton::clicked,
            this, [this]()
    {
        int currentRow = m_ui->listWidget->currentRow();
        if (currentRow <= 0)
        {
            return;
        }
        auto currentItem = m_ui->listWidget->takeItem(currentRow);
        m_ui->listWidget->insertItem(currentRow - 1, currentItem);
        m_ui->listWidget->setCurrentRow(currentRow-1);
        m_ui->tbtUp->setEnabled(currentRow-1 > 0);
        m_ui->tbtDown->setEnabled(currentRow-1 < m_ui->listWidget->count()-1);
    });
    connect(m_ui->tbtDown, &QToolButton::clicked,
            this, [this]()
    {
        int currentRow = m_ui->listWidget->currentRow();
        if (currentRow >= m_ui->listWidget->count()-1)
        {
            return;
        }
        auto currentItem = m_ui->listWidget->takeItem(currentRow + 1);
        m_ui->listWidget->insertItem(currentRow, currentItem);
        m_ui->listWidget->setCurrentRow(currentRow+1);
        m_ui->tbtUp->setEnabled(currentRow+1 > 0);
        m_ui->tbtDown->setEnabled(currentRow+1 < m_ui->listWidget->count()-1);
    });
    connect(m_ui->listWidget, &QListWidget::itemChanged,
            this, [this]()
    {
        bool anySelected = false;
        for(int i = 0; i < m_ui->listWidget->count(); i++)
        {
            if(m_ui->listWidget->item(i)->checkState() == Qt::CheckState::Checked)
            {
                anySelected = true;
                break;
            }
        }
        m_ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(anySelected);
    });
    connect(m_ui->listWidget, &QListWidget::currentRowChanged,
            this, [this]()
    {
        int currentRow = m_ui->listWidget->currentRow();
        m_ui->tbtUp->setEnabled(currentRow > 0);
        m_ui->tbtDown->setEnabled(currentRow < m_ui->listWidget->count()-1);
    });
    connect(m_ui->listWidget->model(), &QAbstractItemModel::rowsMoved,
            this, [this]()
    {
        int currentRow = m_ui->listWidget->currentRow();
        m_ui->tbtUp->setEnabled(currentRow > 0);
        m_ui->tbtDown->setEnabled(currentRow < m_ui->listWidget->count()-1);
    });
}

ConfigurationSelectDialog::~ConfigurationSelectDialog()
{
}

void ConfigurationSelectDialog::setSelectionOptions(std::vector<std::string> const &options)
{
    m_ui->listWidget->clear();
    bool found_default = false;
    for(auto &o : options)
    {
        QListWidgetItem *item = new QListWidgetItem(QString::fromStdString(o));
        if (o == "default")
        {
            item->setCheckState(Qt::CheckState::Checked);
            found_default = true;
        }
        else
        {
            item->setCheckState(Qt::CheckState::Unchecked);
        }
        m_ui->listWidget->addItem(item);
    }
    m_ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(found_default);
}

std::vector<std::string> ConfigurationSelectDialog::getSelectionOptions()
{
    std::vector<std::string> result;
    for(int i = 0; i < m_ui->listWidget->count(); i++)
    {
        if(m_ui->listWidget->item(i)->checkState() == Qt::CheckState::Checked)
        {
            result.push_back(m_ui->listWidget->item(i)->text().toStdString());
        }
    }
    return result;
}
