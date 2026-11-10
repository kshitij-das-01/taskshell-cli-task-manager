//
// Created by Kshitij Das on 2026.11.06.
//
#include "task_manager.h"

#include <chrono>

Task TaskManager::addTask(std::string title, std::string description,
                            const Priority priority) {
    Task task;
    task.id = m_next_id++;
    task.title = std::move(title);
    task.description = std::move(description);
    task.priority = priority;
    task.completed = false;
    task.created_at = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    m_tasks.push_back(task);
    return m_tasks.back();
}

std::vector<Task> TaskManager::listTasks(const bool completed_only) const {
    std::vector<Task> result;
    for (const auto& task : m_tasks) {
        if (!completed_only || task.completed) {
            result.push_back(task);
        }
    }
    return result;
}

bool TaskManager::setCompleted(const int64_t id, const bool completed) {
    for (auto& task : m_tasks) {
        if (task.id == id) {
            task.completed = completed;
            return true;
        }
    }
    return false;
}

bool TaskManager::setPriority(const int64_t id, const Priority priority) {
    for (auto& task : m_tasks) {
        if (task.id == id) {
            task.priority = priority;
            return true;
        }
    }
    return false;
}

bool TaskManager::deleteTask(const int64_t id) {
    for (auto it = m_tasks.begin(); it != m_tasks.end(); ++it) {
        if (it->id == id) {
            m_tasks.erase(it);
            return true;
        }
    }
    return false;
}
