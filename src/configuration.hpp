
#pragma once

#include <string>
#include <rtt/TaskContext.hpp>

class TaskItem;

bool save_configuration(RTT::TaskContext* task, TaskItem *titem, std::string const &filename);
