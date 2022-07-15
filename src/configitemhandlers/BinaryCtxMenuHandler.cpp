
#include "BinaryCtxMenuHandler.hpp"
#include "../NameServiceModel.hpp"
#include "../TypedItem.hpp"
#include "../PortItem.hpp"
#include "../Types.hpp"
#include <QMenu>
#include <QObject>
#include <QWidgetAction>
#include <QLabel>
#include <QFileDialog>
#include <typelib/value_ops.hh>

bool BinaryCtxMenuHandler::addContextMenuEntries ( QMenu * menu, const QModelIndex & index ) const
{
    //i would love to add save-image-to option, but we don't store incoming data. that would needed to be
    //done by the ImageView Plugin etc.
    NameServiceModel const *model = static_cast<NameServiceModel const *>(index.model());

    QStandardItem *item = model->itemFromIndex(index);

    TypedItem *ti = dynamic_cast<TypedItem*>(item);
    if(!ti)
        return false;
    switch (ti->type()) {
        case ItemType::TASK:
            return false;

        case ItemType::INPUTPORT:
        case ItemType::EDITABLEITEM:
        {
            ItemBase *itembase = nullptr;
            if (ti->type() == ItemType::INPUTPORT)
            {
                itembase = static_cast<PortItem *>(ti->getData())->getItemBase().get();
            }
            if (ti->type() == ItemType::EDITABLEITEM)
            {
                itembase = static_cast<ItemBase *>(ti->getData());
            }
            if (!itembase)
                return false;
            Typelib::Value &val = itembase->getValueHandle();
            QLabel* label = new QLabel(QObject::tr("<b>Binary data</b>"), menu);
            label->setAlignment(Qt::AlignCenter);
            QWidgetAction* labelAction = new QWidgetAction(menu);
            labelAction->setDefaultWidget(label);
            menu->addAction(labelAction);

            QAction *load = menu->addAction(tr("Load binary data from..."));

            QObject::connect(load, &QAction::triggered,
                    menu, [&val,model,itembase]()
            {
                QString filename = QFileDialog::getOpenFileName(nullptr, tr("Load binary data from"), QString(), tr("Binary (*.bin);;All files (*.*)"));
                if (!filename.isEmpty())
                {
                    QFile file(filename);
                    file.open(QIODevice::ReadOnly);
                    QByteArray data = file.readAll();
                    file.close();

                    Typelib::load(val, reinterpret_cast<uint8_t const *>(data.data()), data.size());
                }

                model->notifyItemDataEdited(itembase->getName()->index());
            });

            return true;
        }
        case ItemType::OUTPUTPORT:
        case ItemType::CONFIGITEM:
        {
            ItemBase *itembase = nullptr;
            if (ti->type() == ItemType::OUTPUTPORT)
            {
                itembase = static_cast<PortItem *>(ti->getData())->getItemBase().get();
            }
            if (ti->type() == ItemType::CONFIGITEM)
            {
                itembase = static_cast<ItemBase *>(ti->getData());
            }
            if (!itembase)
                return false;
            Typelib::Value &val = itembase->getValueHandle();
            QLabel* label = new QLabel(QObject::tr("<b>Binary data</b>"), menu);
            label->setAlignment(Qt::AlignCenter);
            QWidgetAction* labelAction = new QWidgetAction(menu);
            labelAction->setDefaultWidget(label);
            menu->addAction(labelAction);

            QAction *load = menu->addAction(tr("Save binary data to..."));

            QObject::connect(load, &QAction::triggered,
                    menu, [&val]()
            {
                if (!val.getData() )
                    return; //TODO notify user
                std::vector< boost::uint8_t > data = Typelib::dump(val);

                QString filename = QFileDialog::getSaveFileName(nullptr, tr("Save binary data to"), QString(), tr("Binary (*.bin);;All files (*.*)"));

                if (!filename.isEmpty())
                {
                    QFile file(filename);
                    file.open(QIODevice::WriteOnly | QIODevice::Truncate);
                    file.write(reinterpret_cast<char*>(data.data()), data.size());
                    file.close();
                }
            });

            return true;
        }
            break;
        default:
            return false;
    }

    return false;
}

bool BinaryCtxMenuHandler::probe(Typelib::Type const &type, bool editing) const
{
    return true;
}
