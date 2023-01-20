#include "BoolHandler.hpp"

#include <typelib/value.hh>
#include <QStandardItem>
#include <QTextCodec>
#include <QComboBox>
#include <QTimer>
#include <base-logging/Logging.hpp>
#include "../NameServiceModel.hpp"
#include "../TypedItem.hpp"
#include "../ConfigItem.hpp"
#include "../Types.hpp"
#include "../PortItem.hpp"
#include "../NameServiceItemDelegate.hpp"

QWidget *BoolHandler::createEditor(NameServiceItemDelegate const* delegate, QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    NameServiceModel const *model = static_cast<NameServiceModel const *>(index.model());
    QStandardItem *item = model->itemFromIndex(index);

    TypedItem *ti = dynamic_cast<TypedItem*>(item);
    if (!ti)
        return nullptr;

    ItemBase* itembase = nullptr;
    if (ti->type() == ItemType::INPUTPORT)
        itembase = static_cast<PortItem *>(ti->getData())->getItemBase().get();
    else if (ti->type() == ItemType::EDITABLEITEM)
        itembase = static_cast<ItemBase *>(ti->getData());
    else
        return nullptr;

    Typelib::Value &value = itembase->getValueHandle();
    Typelib::Type const &type = value.getType();
    if (type.getCategory() != Typelib::Type::Numeric || type.getName() != "/bool")
        return nullptr;

    QComboBox *cob = new QComboBox(parent);
    cob->setAutoFillBackground(true);
    cob->setGeometry(option.rect);
    cob->setFocusPolicy(Qt::StrongFocus);

    QObject::connect(cob, QOverload<int>::of(&QComboBox::activated),
                     delegate, [cob,delegate]()
    {
        auto d = const_cast<NameServiceItemDelegate*>(delegate);
        emit d->commitData(cob);
        emit d->closeEditor(cob);
    });

    cob->addItem("false", QVariant(false));
    cob->addItem("true", QVariant(true));
    //the combobox receives the mouse-up event from the mouse-down leading to its creation, thus
    //automatically hiding any open popups again. delay a bit, then show the popup.
    QTimer::singleShot(200, [cob]
    {
        cob->showPopup();
    });
    return cob;
}

void BoolHandler::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *cob = static_cast<QComboBox*>(editor);

    NameServiceModel const *model = static_cast<NameServiceModel const *>(index.model());
    QStandardItem *item = model->itemFromIndex(index);

    TypedItem *ti = dynamic_cast<TypedItem*>(item);
    if (!ti)
    {
        cob->setCurrentText(tr("item not typeditem"));
        return;
    }

    ItemBase* itembase = nullptr;
    if (ti->type() == ItemType::INPUTPORT)
        itembase = static_cast<PortItem *>(ti->getData())->getItemBase().get();
    else if (ti->type() == ItemType::EDITABLEITEM)
        itembase = static_cast<ItemBase *>(ti->getData());
    else
    {
        cob->setCurrentText(tr("item not inputport or editable"));
        return;
    }

    Typelib::Value &value = itembase->getValueHandle();
    Typelib::Type const &type = value.getType();
    if (type.getCategory() != Typelib::Type::Numeric || type.getName() != "/bool")
    {
        cob->setCurrentText(tr("Not bool type"));
        return;
    }

    bool *boolVal = static_cast<bool *>(value.getData());
    if (!boolVal)
    {
        cob->setCurrentText(tr("Bool data not available"));
        return;
    }

    cob->setCurrentIndex(*boolVal ? 1 : 0);
}

bool BoolHandler::setModelData(QWidget *editor, NameServiceModel *model, const QModelIndex &index) const
{
    QComboBox *cob = static_cast<QComboBox*>(editor);

    QStandardItem *item = model->itemFromIndex(index);

    TypedItem *ti = dynamic_cast<TypedItem*>(item);
    if (!ti)
        return false;

    ItemBase* itembase = nullptr;
    if (ti->type() == ItemType::INPUTPORT)
        itembase = static_cast<PortItem *>(ti->getData())->getItemBase().get();
    else if (ti->type() == ItemType::EDITABLEITEM)
        itembase = static_cast<ItemBase *>(ti->getData());
    else
        return false;

    Typelib::Value &value = itembase->getValueHandle();
    Typelib::Type const &type = value.getType();
    if (type.getCategory() != Typelib::Type::Numeric || type.getName() != "/bool")
        return false;

    bool *boolVal = static_cast<bool *>(value.getData());
    if (!boolVal)
        return false;

    bool newval = cob->currentData().toBool();
    if (*boolVal == newval)
        return false;
    *boolVal = newval;
    return true;
}

void BoolHandler::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

bool BoolHandler::convertFromTypelibValue(QStandardItem *dst, Typelib::Value const &src, QTextCodec *codec) const
{
    const Typelib::Type &type(src.getType());
    QString valueS = dst->text();
    QString oldValue = valueS;

    if (type.getCategory() == Typelib::Type::Numeric && type.getName() == "/bool")
    {
        bool *boolVal = static_cast<bool *>(src.getData());
        if(!boolVal)
        {
            return false;
        }
        if(*boolVal)
            valueS = "true";
        else
            valueS = "false";
    }
    else
    {
        LOG_WARN_S << "got unsupported type..";
        return false;
    }

    if (valueS != oldValue)
    {
        dst->setText(valueS);
        return true;
    }

    return false;
}

bool BoolHandler::convertToTypelibValue(Typelib::Value &dst, QStandardItem *src, QTextCodec *codec) const
{
    QString data = src->data(Qt::EditRole).toString();

    const Typelib::Type &type( dst.getType());
    bool dstval = false;
    if(data.compare(QString("true"), Qt::CaseInsensitive))
        dstval = true;
    else if(data.compare(tr("true"), Qt::CaseInsensitive))
        dstval = true;
    else if(data.compare(QString("on"), Qt::CaseInsensitive))
        dstval = true;
    else if(data.compare(tr("on"), Qt::CaseInsensitive))
        dstval = true;
    else if(data.compare("1", Qt::CaseInsensitive))
        dstval = true;


    if (type.getCategory() == Typelib::Type::Numeric && type.getName() == "/bool")
    {
        bool *boolVal = static_cast<bool *>(dst.getData());
        if(!boolVal)
        {
            return false;
        }
        if(*boolVal == dstval)
        {
            return false;
        }
        *boolVal = dstval;
        return true;
    }
    else
    {
        LOG_WARN_S << "got unsupported type..";
        return false;
    }

    return false;
}

bool BoolHandler::probe(Typelib::Type const &type, bool editing) const
{
    return type.getCategory() == Typelib::Type::Numeric && type.getName() == "/bool";
}

