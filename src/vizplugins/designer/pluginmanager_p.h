/* This file was taken from Qt Designer 5.12 and modified/reduced to fit. */
/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QtCore/qobject.h>
#include <QtCore/qstringlist.h>

QT_BEGIN_NAMESPACE

class QDesignerCustomWidgetInterface;

QT_END_NAMESPACE

namespace rockdisplay {

class QDesignerPluginManagerPrivate;

class QDesignerPluginManager: public QObject
{
    Q_OBJECT
public:
    using CustomWidgetList = QList<QDesignerCustomWidgetInterface *>;

    explicit QDesignerPluginManager();
    ~QDesignerPluginManager() override;

    QObject *instance(const QString &plugin) const;

    QStringList registeredPlugins() const;

    QStringList findPlugins(const QString &path);

    QStringList pluginPaths() const;
    void setPluginPaths(const QStringList &plugin_paths);

    QStringList failedPlugins() const;
    QString failureReason(const QString &pluginName) const;

    QObjectList instances() const;

    CustomWidgetList registeredCustomWidgets() const;

    QDesignerCustomWidgetInterface *findWidgetByName(const QString &widgetName) const;

    bool registerNewPlugins();

    static QDesignerPluginManager *getInstance();

public slots:
    void ensureInitialized();

private:
    void updateRegisteredPlugins();
    void registerPath(const QString &path);
    void registerPlugin(const QString &plugin);

private:
    static QStringList defaultPluginPaths();

    QDesignerPluginManagerPrivate *m_d;
    static QDesignerPluginManager *m_inst;
};

}

#endif // PLUGINMANAGER_H
