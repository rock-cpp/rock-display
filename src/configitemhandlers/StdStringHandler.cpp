
#include "StdStringHandler.hpp"
#include <typelib/value.hh>
#include <QTextCodec>
#include <QStandardItem>

bool StdStringHandler::convertFromTypelibValue(QStandardItem *dst, Typelib::Value const &src, QTextCodec *codec) const
{
    const Typelib::Type &type(src.getType());

    if (type.getCategory() != Typelib::Type::Container)
        return false;

    const Typelib::Container &cont = static_cast<const Typelib::Container &>(src.getType());

    if(cont.kind() != "/std/string")
        return false;

    const std::string content = *static_cast<const std::string *>(src.getData());

    if (codec)
    {
        QTextCodec::ConverterState state;
        const QString text = codec->toUnicode(content.c_str(), content.size(), &state);
        if (state.invalidChars > 0)
        {
            return false;
        }
        else if (dst->text().toStdString() != text.toStdString())
        {
            dst->setText(text);
            return true;
        }
    }

    return false;
}

bool StdStringHandler::convertToTypelibValue(Typelib::Value &dst, QStandardItem *src, QTextCodec *codec) const
{
    QString data = src->data(Qt::EditRole).toString();


    std::string valueS = data.toStdString();
    if (codec)
    {
        QByteArray bytes = codec->fromUnicode(data);
        valueS = bytes.toStdString();
    }

    const Typelib::Type &type(dst.getType());

    if (type.getCategory() != Typelib::Type::Container)
        return false;

    const Typelib::Container &cont = static_cast<const Typelib::Container &>(dst.getType());

    if(cont.kind() != "/std/string")
        return false;

    std::string *content = static_cast<std::string *>(dst.getData());

    if (codec)
    {
        QTextCodec::ConverterState state;
        if (*content == valueS)
            return false;

        *content = valueS;
        return true;
    }

    return false;
}

bool StdStringHandler::probe(Typelib::Type const &type, bool editing) const
{
    return type.getName() == "/std/string";
}

