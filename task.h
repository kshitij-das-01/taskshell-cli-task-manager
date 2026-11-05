//
// Created by Kshitij Das on 2026.11.06.
//

#ifndef TASKSHELL_CLI_TASK_MANAGER_TASK_H
#define TASKSHELL_CLI_TASK_MANAGER_TASK_H

#include <cstdint>
#include <string>

enum class Priority {
    None, Low, Medium, High
};

struct Task {
    int64_t id = 0;
    std::string title;
    std::string description;
    Priority priority = Priority::None;
    bool completed = false;
    int64_t created_at = 0; // epoch milliseconds
};

#endif //TASKSHELL_CLI_TASK_MANAGER_TASK_H
