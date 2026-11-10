//
// Created by Kshitij Das on 2026.11.06.
//
#include "../include/task_manager.h"

#include <algorithm>
#include <iostream>
#include <string>

static Priority s_parsePriority(const std::string& input) {
    std::string lower = input;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                    [](unsigned char c) {return std::tolower(c); });

    if (lower == "none") return Priority::None;
    if (lower == "low") return Priority::Low;
    if (lower == "medium") return Priority::Medium;
    if (lower == "high") return Priority::High;

    return Priority::None; // default
}

static std::string s_priorityToString(Priority priority) {
    switch (priority) {
        case Priority::None: return "none";
        case Priority::Low: return "low";
        case Priority::Medium: return "medium";
        case Priority::High: return "high";
    }
    return "none";
}

int main() {
    TaskManager tm;

    std::cout << "Task Manager (native CLI - local testing only)\n";
    std::cout << "Commands:\n";
    std::cout << "  add <title> [priority]   - Add a task (priority: none/low/medium/high)\n";
    std::cout << "  list                     - List all tasks\n";
    std::cout << "  done <id>                - Mark task complete\n";
    std::cout << "  priority <id> <level>    - Change task priority\n";
    std::cout << "  delete <id>              - Delete a task\n";
    std::cout << "  quit                     - Exit\n";

    std::string line;

    while (std::getline(std::cin, line)) {
        std::cout << '\n';
        if (line == "quit" || line == "exit") {
            break;
        }

        // add <title> [priority]
        if (line.rfind("add ", 0) == 0) {
            std::string rest = line.substr(4); // after "add "

            // Split off optional trailing priority
            size_t lastSpace = rest.find_last_of(' ');
            std::string title = rest;
            Priority priority = Priority::None;

            if (lastSpace != std::string::npos) {
                std::string maybePriority = rest.substr(lastSpace+1);

                Priority parsed_priority = s_parsePriority(maybePriority);

                if (maybePriority == "none" || maybePriority == "low" ||
                    maybePriority == "medium" || maybePriority == "high")
                {
                    title = rest.substr(0, lastSpace);
                    priority = parsed_priority;
                }
            }

            if (title.empty()) {
                std::cout << "Error: Title cannot be empty.\n";
                continue;
            }

            Task final_task = tm.addTask(title, "", priority);
            std::cout << "Added task #" << final_task.id << " [" << s_priorityToString(final_task.priority) << "]: "
                      << final_task.title << "\n";
        }

        // list
        else if (line.rfind("list", 0) == 0) {
            auto tasks = tm.listTasks();

            if (tasks.empty()) {
                std::cout << "No Tasks Yet\n";
            }
            else {
                for (const auto& t : tasks) {
                    std::cout << "[" << (t.completed ? "x" : " ") << "] "
                              << t.id << " (" << s_priorityToString(t.priority) << ") " << t.title << "\n";
                }
            }
        }

        // done <id>
        else if (line.rfind("done ", 0) == 0) {
            try {
                int64_t id = std::stoll(line.substr(5));
                if (tm.setCompleted(id, true)) {
                    std::cout << "Marked task #" << id << " complete\n";
                }
                else {
                    std::cout << "Task #" << id << " not found\n";
                }
            } catch (...) {
                std::cout << "Error: Invalid Id\n";
            }
        }

        // priority <id> <level>
        else if (line.rfind("priority ", 0) == 0) {
            std::string rest = line.substr(9); // after "priority "
            size_t space = rest.find(' ');

            if (space == std::string::npos) {
                std::cout << "Usage: priority <id> <none|low|medium|high>\n";
                continue;
            }

            try {
                int64_t id = std::stoll(rest.substr(0, space));
                std::string level = rest.substr(space+1);
                Priority p = s_parsePriority(level);

                if (tm.setPriority(id, p)) {
                    std::cout << "Task #" << id << " priority set to " << s_priorityToString(p) << '\n';
                }
                else {
                    std::cout << "Task #" << id << " not found\n";
                }
            } catch (...) {
                std::cout << "Error: Invalid Id\n";
            }
        }

        // delete <id>
        else if (line.rfind("delete " , 0) == 0) {
            try {
                int64_t id = std::stoll(line.substr(7));

                if (tm.deleteTask(id)) {
                    std::cout << "Task #" << id << " deleted\n";
                }
                else {
                    std::cout << "Task #" << id << " not found\n";
                }
            } catch (...) {
                std::cout << "Error: Invalid Id\n";
            }
        }

        // unknown
        else {
            std::cout << "Unknown Command. Type 'quit' to exit.\n";
        }
    }

    return 0;
}
