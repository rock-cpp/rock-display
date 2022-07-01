
#pragma once

#include <QObject>
#include <vector>

QT_BEGIN_NAMESPACE
class QWidget;
class QModelIndex;
class QPainter;
class QStyleOptionViewItem;
class QMenu;
class QAbstractItemModel;
class QStandardItem;
class QTextCodec;
QT_END_NAMESPACE

namespace Typelib
{
    class Value;
    class Type;
}

class NameServiceModel;
class NameServiceItemDelegate;

class ConfigItemHandler
{
public:
    enum Flags {
        CustomPaint              = 1, ///< uses a custom painter instead of Text display of item->setText().
        CustomEditor             = 2, ///< uses a custom editor instead of QLineEdit.
        ContextMenuItems         = 4, ///< adds context menu entries.
        ConvertsFromTypelibValue = 8, ///< has custom text display
        ConvertsToTypelibValue   = 16,///< can use the default text editor to fill the field
        HideFields               = 32,///< do not show sub entries of this field
                                      ///< generic handlers of containers/array/compounds need to watch out for this
                                      ///< from other handlers if they affect fields
    };


    virtual ~ConfigItemHandler();

    virtual int flags() const = 0;

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {}

    virtual QWidget *createEditor(NameServiceItemDelegate const* delegate, QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const { return nullptr; }
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const {}
    /**
     *
     * @returns true if data changed
     */
    virtual bool setModelData(QWidget *editor, NameServiceModel *model, const QModelIndex &index) const { return false; }
    virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const {}

    /** Add entries to the given menu
     *
     * @return true if entries were added
     */
    virtual bool addContextMenuEntries(QMenu *menu, const QModelIndex &index) const { return false; }

    /** Converts to string from typelib value
     */
    virtual bool convertFromTypelibValue(QStandardItem *dst, Typelib::Value const &src, QTextCodec *codec) const { return false; }

    /** Converts from typelib value
     */
    virtual bool convertToTypelibValue(Typelib::Value &dst, QStandardItem *src, QTextCodec *codec) const { return false; }

    /** Tests if this handler can handle the given type
     */
    virtual bool probe(Typelib::Type const &type, bool editing = true ) const = 0;
};

