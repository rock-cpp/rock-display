

#include "rockdisplay_pluginmanager.hpp"
#include <QStringList>
#include <QCoreApplication>
#include <QDir>
#include <QLibrary>
#include <QPluginLoader>
#include "vizplugins/vizkitplugin.hpp"

static QStringList defaultPluginPaths()
{
    QStringList result;

    const QStringList path_list = QCoreApplication::libraryPaths();

    const QString pathcomponent = QStringLiteral("rockdisplay");
    for (const QString &path : path_list)
    {
        QString libPath = path;
        libPath += QDir::separator();
        libPath += pathcomponent;
        result.append(libPath);
    }

    return result;
}

static QStringList findLibraries(const QString &path)
{
    const QDir dir(path);
    printf("Looking in %s...",path.toLocal8Bit().data());
    if (!dir.exists()) {
        printf(" does not exist\n");
        return QStringList();
    }

    const QFileInfoList infoList = dir.entryInfoList(QDir::Files);
    if (infoList.isEmpty()){
        printf(" is empty\n");
        return QStringList();
    }

    printf("\n");
    // Load symbolic links but make sure all file names are unique as not
    // to fall for something like 'libplugin.so.1 -> libplugin.so'
    QStringList result;
    const QFileInfoList::const_iterator icend = infoList.constEnd();
    for (QFileInfoList::const_iterator it = infoList.constBegin(); it != icend; ++it)
    {
        QString fileName;
        if (it->isSymLink())
        {
            const QFileInfo linkTarget = QFileInfo(it->symLinkTarget());
            if (linkTarget.exists() && linkTarget.isFile())
                fileName = linkTarget.absoluteFilePath();
        }
        else
        {
            fileName = it->absoluteFilePath();
        }
        if (!fileName.isEmpty() && QLibrary::isLibrary(fileName) && !result.contains(fileName))
            result += fileName;
    }
    return result;
}


rockdisplay::PluginManager::PluginManager()
{
    QStringList pluginPaths = defaultPluginPaths();

    QStringList pluginLibraries;
    for (const QString &path : qAsConst(pluginPaths))
    {
        pluginLibraries += findLibraries(path);
    }

    for (const QString &plugin : pluginLibraries)
    {
        tryRegisterPlugin(plugin);
    }
}

void rockdisplay::PluginManager::tryRegisterPlugin(QString const &plugin)
{
    QPluginLoader loader(plugin);
    if (loader.isLoaded() || loader.load())
    {
        QObject *o = loader.instance();
        rockdisplay::vizkitplugin::PluginFactory *pluginfactory =
            qobject_cast<rockdisplay::vizkitplugin::PluginFactory *>(o);
        if (pluginfactory)
        {
            std::vector< rockdisplay::vizkitplugin::Plugin * > plugins =
                pluginfactory->createPlugins();
            m_vizkitplugins.insert(m_vizkitplugins.end(), plugins.begin(), plugins.end());
        }
        //Here we can add another type of factory, or combine as needed.
    }
}

