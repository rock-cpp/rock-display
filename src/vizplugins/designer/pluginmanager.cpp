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

#include "pluginmanager_p.h"

#include <QtUiPlugin/customwidget.h>

#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qpluginloader.h>
#include <QtCore/qlibrary.h>
#include <QtCore/qdebug.h>
#include <QtCore/qmap.h>
#include <QtCore/qcoreapplication.h>

static const char propertyToolTipC[] = "tooltip";

enum { debugPluginManager = 0 };

/* Custom widgets: Loading custom widgets is a 2-step process: PluginManager
 * scans for its plugins in the constructor. At this point, it might not be safe
 * to immediately initialize the custom widgets it finds, because the rest of
 * Designer is not initialized yet.
 * Later on, in ensureInitialized(), the plugin instances (including static ones)
 * are iterated and the custom widget plugins are initialized and added to internal
 * list of custom widgets and parsed data. Should there be a parse error or a language
 * mismatch, it kicks out the respective custom widget. The m_initialized flag
 * is used to indicate the state.
 * Later, someone might call registerNewPlugins(), which agains clears the flag via
 * registerPlugin() and triggers the process again.
 * Also note that Jambi fakes a custom widget collection that changes its contents
 * every time the project is switched. So, custom widget plugins can actually
 * disappear, and the custom widget list must be cleared and refilled in
 * ensureInitialized() after registerNewPlugins. */

QStringList rockdisplay::QDesignerPluginManager::defaultPluginPaths()
{
    QStringList result;

    const QStringList path_list = QCoreApplication::libraryPaths();

    const QString designer = QStringLiteral("designer");
    for (const QString &path : path_list) {
        QString libPath = path;
        libPath += QDir::separator();
        libPath += designer;
        result.append(libPath);
    }

    QString homeLibPath = QDir::homePath();
    homeLibPath += QDir::separator();
    homeLibPath += QStringLiteral(".designer");
    homeLibPath += QDir::separator();
    homeLibPath += QStringLiteral("plugins");

    result.append(homeLibPath);
    return result;
}

// ----------------  QDesignerCustomWidgetData

// Wind a QXmlStreamReader  until it finds an element. Returns index or one of FindResult
enum FindResult { FindError = -2, ElementNotFound = -1 };

static inline QString msgXmlError(const QString &name, const QString &errorMessage)
{
    return rockdisplay::QDesignerPluginManager::tr("An XML error was encountered when parsing the XML of the custom widget %1: %2").arg(name, errorMessage);
}

static inline QString msgAttributeMissing(const QString &name)
{
    return rockdisplay::QDesignerPluginManager::tr("A required attribute ('%1') is missing.").arg(name);
}

// ---------------- QDesignerPluginManagerPrivate

namespace rockdisplay {
class QDesignerPluginManagerPrivate {
    public:
    using ClassNamePropertyNameKey = QPair<QString, QString>;

    QDesignerPluginManagerPrivate();

    void clearCustomWidgets();
    bool addCustomWidget(QDesignerCustomWidgetInterface *c);
    void addCustomWidgets(const QObject *o);

    QStringList m_pluginPaths;
    QStringList m_registeredPlugins;

    typedef QMap<QString, QString> FailedPluginMap;
    FailedPluginMap m_failedPlugins;

    // Synced lists of custom widgets and their data. Note that the list
    // must be ordered for collections to appear in order.
    QList<QDesignerCustomWidgetInterface *> m_customWidgets;

    QStringList defaultPluginPaths() const;

    bool m_initialized;
};
}

rockdisplay::QDesignerPluginManagerPrivate::QDesignerPluginManagerPrivate() :
   m_initialized(false)
{
}

void rockdisplay::QDesignerPluginManagerPrivate::clearCustomWidgets()
{
    m_customWidgets.clear();
}

// Add a custom widget to the list if it parses correctly
// and is of the right language
bool rockdisplay::QDesignerPluginManagerPrivate::addCustomWidget(QDesignerCustomWidgetInterface *c)
{
    if (debugPluginManager)
        qDebug() << Q_FUNC_INFO << c->name();

    m_customWidgets.push_back(c);
    return true;
}

// Check the plugin interface for either a custom widget or a collection and
// add all contained custom widgets.
void rockdisplay::QDesignerPluginManagerPrivate::addCustomWidgets(const QObject *o)
{
    if (QDesignerCustomWidgetInterface *c = qobject_cast<QDesignerCustomWidgetInterface*>(o)) {
        addCustomWidget(c);
        return;
    }
    if (const QDesignerCustomWidgetCollectionInterface *coll = qobject_cast<QDesignerCustomWidgetCollectionInterface*>(o)) {
        const auto &collCustomWidgets = coll->customWidgets();
        for (QDesignerCustomWidgetInterface *c : collCustomWidgets) {
            addCustomWidget(c);
        }
    }
}


// ---------------- QDesignerPluginManager
// As of 4.4, the header will be distributed with the Eclipse plugin.

rockdisplay::QDesignerPluginManager::QDesignerPluginManager() :
    QObject(),
    m_d(new rockdisplay::QDesignerPluginManagerPrivate())
{
    m_d->m_pluginPaths = defaultPluginPaths();

    // Register plugins
    updateRegisteredPlugins();
}

rockdisplay::QDesignerPluginManager::~QDesignerPluginManager()
{
    delete m_d;
}

QStringList rockdisplay::QDesignerPluginManager::findPlugins(const QString &path)
{
    if (debugPluginManager)
        qDebug() << Q_FUNC_INFO << path;
    const QDir dir(path);
    if (!dir.exists())
        return QStringList();

    const QFileInfoList infoList = dir.entryInfoList(QDir::Files);
    if (infoList.isEmpty())
        return QStringList();

    // Load symbolic links but make sure all file names are unique as not
    // to fall for something like 'libplugin.so.1 -> libplugin.so'
    QStringList result;
    const QFileInfoList::const_iterator icend = infoList.constEnd();
    for (QFileInfoList::const_iterator it = infoList.constBegin(); it != icend; ++it) {
        QString fileName;
        if (it->isSymLink()) {
            const QFileInfo linkTarget = QFileInfo(it->symLinkTarget());
            if (linkTarget.exists() && linkTarget.isFile())
                fileName = linkTarget.absoluteFilePath();
        } else {
            fileName = it->absoluteFilePath();
        }
        if (!fileName.isEmpty() && QLibrary::isLibrary(fileName) && !result.contains(fileName))
            result += fileName;
    }
    return result;
}

void rockdisplay::QDesignerPluginManager::setPluginPaths(const QStringList &plugin_paths)
{
    m_d->m_pluginPaths = plugin_paths;
    updateRegisteredPlugins();
}

QStringList rockdisplay::QDesignerPluginManager::failedPlugins() const
{
    return m_d->m_failedPlugins.keys();
}

QString rockdisplay::QDesignerPluginManager::failureReason(const QString &pluginName) const
{
    return m_d->m_failedPlugins.value(pluginName);
}

QStringList rockdisplay::QDesignerPluginManager::registeredPlugins() const
{
    return m_d->m_registeredPlugins;
}

QStringList rockdisplay::QDesignerPluginManager::pluginPaths() const
{
    return m_d->m_pluginPaths;
}

QObject *rockdisplay::QDesignerPluginManager::instance(const QString &plugin) const
{
    QPluginLoader loader(plugin);
    return loader.instance();
}

void rockdisplay::QDesignerPluginManager::updateRegisteredPlugins()
{
    if (debugPluginManager)
        qDebug() << Q_FUNC_INFO;
    m_d->m_registeredPlugins.clear();
    for (const QString &path : qAsConst(m_d->m_pluginPaths))
        registerPath(path);
}

bool rockdisplay::QDesignerPluginManager::registerNewPlugins()
{
    if (debugPluginManager)
        qDebug() << Q_FUNC_INFO;

    const int before = m_d->m_registeredPlugins.size();
    for (const QString &path : qAsConst(m_d->m_pluginPaths))
        registerPath(path);
    const bool newPluginsFound = m_d->m_registeredPlugins.size() > before;
    // We force a re-initialize as Jambi collection might return
    // different widget lists when switching projects.
    m_d->m_initialized = false;
    ensureInitialized();

    return newPluginsFound;
}

void rockdisplay::QDesignerPluginManager::registerPath(const QString &path)
{
    if (debugPluginManager)
        qDebug() << Q_FUNC_INFO << path;
    const QStringList &candidates = findPlugins(path);
    for (const QString &plugin : candidates)
        registerPlugin(plugin);
}

void rockdisplay::QDesignerPluginManager::registerPlugin(const QString &plugin)
{
    if (debugPluginManager)
        qDebug() << Q_FUNC_INFO << plugin;
    if (m_d->m_registeredPlugins.contains(plugin))
        return;

    QPluginLoader loader(plugin);
    if (loader.isLoaded() || loader.load()) {
        m_d->m_registeredPlugins += plugin;
        QDesignerPluginManagerPrivate::FailedPluginMap::iterator fit = m_d->m_failedPlugins.find(plugin);
        if (fit != m_d->m_failedPlugins.end())
            m_d->m_failedPlugins.erase(fit);
        return;
    }

    const QString errorMessage = loader.errorString();
    m_d->m_failedPlugins.insert(plugin, errorMessage);
}



void rockdisplay::QDesignerPluginManager::ensureInitialized()
{
    if (debugPluginManager)
        qDebug() << Q_FUNC_INFO <<  m_d->m_initialized << m_d->m_customWidgets.size();

    if (m_d->m_initialized)
        return;

    m_d->clearCustomWidgets();
    // Add the static custom widgets
    const QObjectList staticPluginObjects = QPluginLoader::staticInstances();
    if (!staticPluginObjects.isEmpty()) {
        const QString staticPluginPath = QCoreApplication::applicationFilePath();
        for (QObject *o : staticPluginObjects)
            m_d->addCustomWidgets(o);
    }
    for (const QString &plugin : qAsConst(m_d->m_registeredPlugins)) {
        if (QObject *o = instance(plugin))
            m_d->addCustomWidgets(o);
    }

    m_d->m_initialized = true;
}

rockdisplay::QDesignerPluginManager::CustomWidgetList rockdisplay::QDesignerPluginManager::registeredCustomWidgets() const
{
    const_cast<rockdisplay::QDesignerPluginManager*>(this)->ensureInitialized();
    return m_d->m_customWidgets;
}

QDesignerCustomWidgetInterface *rockdisplay::QDesignerPluginManager::findWidgetByName(const QString &widgetName) const
{
    for (QDesignerCustomWidgetInterface *widgetInterface: registeredCustomWidgets())
    {
        if (widgetInterface->name() == widgetName)
        {
            return widgetInterface;
        }
    }
    return nullptr;
}

QObjectList rockdisplay::QDesignerPluginManager::instances() const
{
    const QStringList &plugins = registeredPlugins();

    QObjectList lst;
    for (const QString &plugin : plugins) {
        if (QObject *o = instance(plugin))
            lst.append(o);
    }

    return lst;
}

rockdisplay::QDesignerPluginManager *rockdisplay::QDesignerPluginManager::m_inst = nullptr;

rockdisplay::QDesignerPluginManager *rockdisplay::QDesignerPluginManager::getInstance()
{
    if(!m_inst)
        m_inst = new QDesignerPluginManager();

    return m_inst;
}

