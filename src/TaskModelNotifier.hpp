#include <QObject>
#include <map>
#include <vector>
#include <unistd.h>

namespace RTT
{
    namespace corba
    {
        class TaskContextProxy;
    }
}

namespace orocos_cpp
{
    class NameService;
}


class TaskModelNotifier : public QObject
{
    Q_OBJECT

    std::map<std::string, RTT::corba::TaskContextProxy *> nameToRegisteredTask;
    std::vector<std::string > disconnectedTasks;
    orocos_cpp::NameService *nameService;
    bool isRunning;
    void queryTasks();
    int connect_trials;
    const int max_connect_trials = 10;
    int numTasks;

public:
    explicit TaskModelNotifier(QObject* parent = 0);

    signals:
    void updateTask(RTT::corba::TaskContextProxy* task, const std::string &taskName, bool reconnect);
    void finished();
    void updateNameServiceStatus(const std::string &status);
    void updateTasksStatus(const std::string &status);

public slots:
    void stopNotifier();
    void initializeNameService(const std::string &nameServiceIP);
    orocos_cpp::NameService *getNameService()
    {
        return nameService;
    }

    void run()
    {
        isRunning = true;

        while (isRunning)
        {
            queryTasks();
            usleep(1000); //FIXME usleep...
        }

        emit finished();
    }
};