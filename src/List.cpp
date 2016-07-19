#include <iostream>
#include <rtt/TaskContext.hpp>
#include <orocos_cpp/CorbaNameService.hpp>

int main(int argc, char** argv)
{
    orocos_cpp::NameService *ns = new orocos_cpp::CorbaNameService();
    
    if(!ns->connect())
    {
        std::cout << "Could not connect to Nameserver " << std::endl;
        return 0;
    }

    std::vector<std::string> tasks = ns->getRegisteredTasks();

    for(const std::string &tname : tasks)
    {
        std::cout << "Task " << tname << std::endl;
    }

    return 0;
    
}
