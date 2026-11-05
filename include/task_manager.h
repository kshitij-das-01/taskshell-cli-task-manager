//
// Created by Kshitij Das on 2026.11.06.
//

#ifndef TASKSHELL_CLI_TASK_MANAGER_TASK_MANAGER_H
#define TASKSHELL_CLI_TASK_MANAGER_TASK_MANAGER_H

#include "task.h"

#include <cstdint>
#include <vector>

class TaskManager {
public:
    TaskManager() = default;

    Task addTask(std::string title, std::string description = {},
                Priority priority = Priority::None);

    std::vector<Task> listTasks(bool completed_only = false) const;

    bool setCompleted(int64_t id, bool completed);
    bool setPriority(int64_t id, Priority priority);
    bool deleteTask(int64_t id);

private:
    std::vector<Task> m_tasks;
    int64_t m_next_id = 1;
};

#endif //TASKSHELL_CLI_TASK_MANAGER_TASK_MANAGER_H
