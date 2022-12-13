#include "EnumHandler.hpp"

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

QWidget *EnumHandler::createEditor(NameServiceItemDelegate const* delegate, QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
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
    if (type.getCategory() != Typelib::Type::Enum)
        return nullptr;

    const Typelib::Enum &enumT = static_cast<const Typelib::Enum &>(type);

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

    for(auto &e : enumT.values())
    {
        if (itembase->codec)
        {
            QTextCodec::ConverterState state;
            const QString text = itembase->codec->toUnicode(e.first.c_str(), e.first.size(), &state);

            if (state.invalidChars > 0)
            {
                delete cob;
                return nullptr;
            }
            cob->addItem(text,e.second);
        }
    }
    //the combobox receives the mouse-up event from the mouse-down leading to its creation, thus
    //automatically hiding any open popups again. delay a bit, then show the popup.
    QTimer::singleShot(200, [cob]
    {
        cob->showPopup();
    });
    return cob;
}

void EnumHandler::setEditorData(QWidget *editor, const QModelIndex &index) const
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
    if (type.getCategory() != Typelib::Type::Enum)
    {
        cob->setCurrentText(tr("Not Enum type"));
        return;
    }

    Typelib::Enum::integral_type *intVal = (static_cast<Typelib::Enum::integral_type *>(value.getData()));
    if (!intVal)
    {
        cob->setCurrentText(tr("Enum data not available"));
        return;
    }

    for (int i = 0; i < cob->count(); i++)
    {
        if (cob->itemData(i) == *intVal)
        {
            cob->setCurrentIndex(i);
            return;
        }
    }

    cob->setCurrentText(tr("Enum Index %1").arg(*intVal));
}

bool EnumHandler::setModelData(QWidget *editor, NameServiceModel *model, const QModelIndex &index) const
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
    if (type.getCategory() != Typelib::Type::Enum)
        return false;

    Typelib::Enum::integral_type *intVal = (static_cast<Typelib::Enum::integral_type *>(value.getData()));
    if (!intVal)
        return false;

    Typelib::Enum::integral_type newval = cob->currentData().toInt();
    if (*intVal == newval)
        return false;
    *intVal = newval;
    return true;
}

void EnumHandler::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

bool EnumHandler::convertFromTypelibValue(QStandardItem *dst, Typelib::Value const &src, QTextCodec *codec) const
{
    const Typelib::Type &type(src.getType());
    std::string valueS = dst->text().toStdString();
    if (codec)
    {
        QByteArray bytes = codec->fromUnicode(dst->text());
        valueS = bytes.toStdString();
    }
    std::string oldValue = valueS;

    if (type.getCategory() == Typelib::Type::Enum)
    {
        const Typelib::Enum &enumT = static_cast<const Typelib::Enum &>(src.getType());
        Typelib::Enum::integral_type *intVal = (static_cast<Typelib::Enum::integral_type *>(src.getData()));
        try {
            valueS = enumT.get(*intVal);
        } catch(Typelib::Enum::ValueNotFound const &e) {
            valueS = "bad value";
        }
    }
    else
    {
        LOG_WARN_S << "got unsupported type..";
        return false;
    }

    if (valueS != oldValue)
    {
        if (codec)
        {
            QTextCodec::ConverterState state;
            const QString text = codec->toUnicode(valueS.c_str(), valueS.size(), &state);

            if (state.invalidChars > 0)
            {
                return false;
            }

            dst->setText(text);
            return true;
        }
    }

    return false;
}

bool EnumHandler::convertToTypelibValue(Typelib::Value &dst, QStandardItem *src, QTextCodec *codec) const
{
    QString data = src->data(Qt::EditRole).toString();

    const Typelib::Type &type( dst.getType());
    std::string valueS = data.toStdString();
    if (codec)
    {
        QByteArray bytes = codec->fromUnicode(data);
        valueS = bytes.toStdString();
    }

    if (type.getCategory() == Typelib::Type::Enum)
    {
        const Typelib::Enum &enumT = static_cast<const Typelib::Enum &>( dst.getType());
        Typelib::Enum::integral_type *intVal = (static_cast<Typelib::Enum::integral_type *>( dst.getData()));
        if (!intVal)
            return false;
        if (*intVal == enumT.get(valueS))
            return false;
        *intVal = enumT.get(valueS);
        return true;
    }
    else
    {
        LOG_WARN_S << "got unsupported type..";
        return false;
    }

    return false;
}

bool EnumHandler::probe(Typelib::Type const &type, bool editing) const
{
    return type.getCategory() == Typelib::Type::Enum;
}

