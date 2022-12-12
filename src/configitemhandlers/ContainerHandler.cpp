#include "ContainerHandler.hpp"
#include "../NameServiceModel.hpp"
#include "../TypedItem.hpp"
#include "../PortItem.hpp"
#include "../Types.hpp"
#include <QMenu>
#include <QObject>
#include <QWidgetAction>
#include <QLabel>
#include <typelib/value_ops.hh>

static Typelib::Value *getItemTypelibValue(QStandardItem *item)
{
    TypedItem *ti = dynamic_cast<TypedItem*>(item);
    if (!ti)
        return nullptr;
    if (ti->type() == ItemType::INPUTPORT)
    {
        auto item = static_cast<PortItem *>(ti->getData())->getItemBase();
        Typelib::Value &val = item->getValueHandle();
        return &val;
    }
    if (ti->type() == ItemType::EDITABLEITEM)
    {
        auto item = static_cast<ItemBase *>(ti->getData());
        Typelib::Value &val = item->getValueHandle();
        return &val;
    }
    return nullptr;
}

bool ContainerHandler::addContextMenuEntries ( QMenu * menu, const QModelIndex & index ) const
{
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
            auto valptr = getItemTypelibValue(item);
            auto pvalptr = getItemTypelibValue(item->parent());
            if(valptr)
            {
                Typelib::Value &val = *valptr;
                const Typelib::Type &type(val.getType());

                if (type.getCategory() == Typelib::Type::Container)
                {
                    QLabel* label = new QLabel(QObject::tr("<b>Container</b>"), menu);
                    label->setAlignment(Qt::AlignCenter);
                    QWidgetAction* labelAction = new QWidgetAction(menu);
                    labelAction->setDefaultWidget(label);
                    menu->addAction(labelAction);

                    QAction *push = menu->addAction(tr("Insert Element at End"));
                    QAction *clear = menu->addAction(tr("Clear"));

                    auto &cnt = static_cast<Typelib::Container const&>(type);

                    QObject::connect(push, &QAction::triggered,
                            menu, [&val,&cnt,itembase,model]()
                    {
                        const Typelib::Type &eltype = cnt.getIndirection();

                        //create new element and its storage
                        std::vector<uint8_t> elbuf;
                        elbuf.resize(eltype.getSize());
                        Typelib::init(elbuf.data(), Typelib::layout_of(eltype) );
                        Typelib::Value el(elbuf.data(), eltype);

                        cnt.push(val.getData(), el);

                        Typelib::destroy(el);

                        itembase->update(val);
                        model->notifyItemDataEdited(itembase->getName()->index());
                    });
                    QObject::connect(clear, &QAction::triggered,
                            menu, [&val,&cnt,model,itembase]()
                    {
                        cnt.clear(val.getData());
                        model->notifyItemDataEdited(itembase->getName()->index());
                    });

                    return true;
                }

                if(item->parent() && pvalptr)
                {
                    Typelib::Value &pval = *pvalptr;
                    const Typelib::Type &ptype(pval.getType());
                    QStandardItem *pitem = item->parent();
                    TypedItem *pti = dynamic_cast<TypedItem*>( pitem );
                    auto pitembase = static_cast<ItemBase *>(pti->getData());

                    if (ptype.getCategory() == Typelib::Type::Container)
                    {
                        QLabel* label = new QLabel(QObject::tr("<b>Container Element</b>"), menu);
                        label->setAlignment(Qt::AlignCenter);
                        QWidgetAction* labelAction = new QWidgetAction(menu);
                        labelAction->setDefaultWidget(label);
                        menu->addAction(labelAction);

                        QAction *erase = menu->addAction(tr("Erase element"));

                        auto &cnt = static_cast<Typelib::Container const&>(ptype);

                        QObject::connect(erase, &QAction::triggered,
                                menu, [&val,&cnt,&pval,pitembase,model]()
                        {
                            cnt.erase(pval.getData(), val);

                            pitembase->update(pval);
                            model->notifyItemDataEdited(pitembase->getName()->index());

                        });

                        return true;
                    }
                }
            }
        }
            break;
        default:
            return false;
    }

    return false;
}

bool ContainerHandler::probe(Typelib::Type const &type, bool editing) const
{
    //cannot decide with just the type or the information available to getItem
    return editing;
}
