
#pragma once

#include "vizkitplugin.hpp"

QT_BEGIN_NAMESPACE

class QDesignerCustomWidgetInterface;

QT_END_NAMESPACE

namespace rock_display {

class SonarViewPlugin : public rockdisplay::vizkitplugin::Plugin {
    Q_OBJECT;
public:
    SonarViewPlugin();
    virtual bool probeOutputPort(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names) override;
    virtual bool probeInputPort(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names) override;
    virtual bool probeProperty(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names) override;
    virtual rockdisplay::vizkitplugin::Widget *createWidget() override;
    virtual std::string getName() override;
    virtual unsigned getFlags() override;
private:
    QDesignerCustomWidgetInterface *widgetInterface;
};

}