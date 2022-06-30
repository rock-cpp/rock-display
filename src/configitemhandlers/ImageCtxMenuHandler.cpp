#include "ImageCtxMenuHandler.hpp"
#include "../NameServiceModel.hpp"
#include "../TypedItem.hpp"
#include "../PortItem.hpp"
#include "../Types.hpp"
#include <QMenu>
#include <QObject>
#include <QWidgetAction>
#include <QLabel>
#include <QFileDialog>
#include <base/samples/Frame.hpp>
#include <frame_helper/FrameHelper.h>

static QString imageFilter = "jpg";

bool ImageCtxMenuHandler::addContextMenuEntries ( QMenu * menu, const QModelIndex & index ) const
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
            QLabel* label = new QLabel(QObject::tr("<b>Image</b>"), menu);
            label->setAlignment(Qt::AlignCenter);
            QWidgetAction* labelAction = new QWidgetAction(menu);
            labelAction->setDefaultWidget(label);
            menu->addAction(labelAction);

            QAction *load = menu->addAction(tr("Load image from..."));

            QObject::connect(load, &QAction::triggered,
                    menu, [&val,this,model,itembase]()
            {
                QString filename = QFileDialog::getOpenFileName(nullptr, tr("Load image from"), QString(), imageFilter);
                base::samples::frame::Frame* frame = static_cast<base::samples::frame::Frame*>(val.getData());
                frame_helper::FrameHelper fh;
                fh.loadFrame(filename.toLocal8Bit().toStdString(), *frame);

                model->notifyItemDataEdited(itembase->getName()->index());
            });

            return true;
        }
            break;
        default:
            return false;
    }

    return false;
}

bool ImageCtxMenuHandler::probe(Typelib::Type const &type, bool editing) const
{
    return editing && type.getName() == "/base/samples/frame/Frame";
}
