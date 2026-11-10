//
// Created by Kshitij Das on 2026.11.10.
//
#include "task_manager.h"

#include <cstdint>
#include <exception>
#include <functional>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <stdexcept>

namespace {
    int passed = 0;
    int failed = 0;

    void check(bool condition, const std::string& message) {
        if (!condition) {
            throw std::runtime_error(message);
        }
    }

    void runTest(const std::string& name, const std::function<void()>& test) {
        try {
            test();
            ++passed;
            std::cout << "[PASS] " << name << '\n';
        }
        catch (const std::exception& error) {
            ++failed;
            std::cerr << "[FAIL] " << name << ": " << error.what() << '\n';
        }
    }

    // 1. Adding a task
    void testAddTask() {
        TaskManager manager;

        const Task task = manager.addTask(
            "Study C++",
            "Practice unit testing",
            Priority::High
        );

        check(task.id == 1, "First task should have ID 1");
        check(task.title == "Study C++", "Title was not stored correctly");
        check(task.description == "Practice unit testing",
            "Description was not stored correctly");
        check(task.priority == Priority::High, "Priority was not stored correctly");
        check(!task.completed, "New task should not be completed");
        check(task.created_at > 0, "Creation timestamp should be valid");
    }

    // 2. Default task values
    void testDefaultValues() {
        TaskManager manager;

        const Task task = manager.addTask("Default task");

        check(task.priority == Priority::None,
            "Default priority should be None");
        check(!task.completed, "Default completion status should be false");
        check(task.description.empty(), "Default description should be empty");
    }

    // 3. Unique and sequential IDs
    void testUniqueIds() {
        TaskManager manager;

        const Task first = manager.addTask("First task");
        const Task second = manager.addTask("Second task");
        const Task third = manager.addTask("Third task");

        check(first.id != second.id, "Task IDs should be unique");
        check(second.id != third.id, "Task IDs should be unique");
        check(first.id == 1 && second.id == 2 && third.id == 3,
            "Task IDs should be sequential");
    }

    // 4. Listing tasks
    void testListTasks() {
        TaskManager manager;

        manager.addTask("First");
        manager.addTask("Second");
        manager.addTask("Third");

        const auto tasks = manager.listTasks();

        check(tasks.size() == 3, "Should return all three tasks");
        check(tasks[0].title == "First", "Incorrect first task");
        check(tasks[1].title == "Second", "Incorrect second task");
        check(tasks[2].title == "Third", "Incorrect third task");
    }

    // 5. Completing and reopening tasks
    void testSetCompleted() {
        TaskManager manager;
        const Task task = manager.addTask("Complete me");

        check(manager.setCompleted(task.id, true),
              "Should successfully complete an existing task");

        auto tasks = manager.listTasks();
        check(tasks[0].completed, "Task should be completed");

        check(manager.setCompleted(task.id, false),
              "Should successfully reopen an existing task");

        tasks = manager.listTasks();
        check(!tasks[0].completed, "Task should be reopened");
    }

    // 6. Filtering completed tasks
    void testCompletedFilter() {
        TaskManager manager;

        const Task first = manager.addTask("First");
        const Task second = manager.addTask("Second");
        manager.addTask("Third");

        manager.setCompleted(first.id, true);
        manager.setCompleted(second.id, true);

        const auto completed = manager.listTasks(true);
        const auto all = manager.listTasks(false);

        check(completed.size() == 2,
              "Completed filter should return two tasks");
        check(all.size() == 3,
              "Unfiltered list should return all three tasks");

        check(completed[0].id == first.id,
              "First completed task is incorrect");
        check(completed[1].id == second.id,
              "Second completed task is incorrect");
    }

    // 7. Changing priority
    void testSetPriority() {
        TaskManager manager;
        const Task task = manager.addTask("Change priority", "", Priority::Low);

        check(manager.setPriority(task.id, Priority::High),
              "Should successfully update priority");

        const auto tasks = manager.listTasks();

        check(tasks[0].priority == Priority::High,
              "Priority was not updated");
    }

    // 8. Deleting tasks
    void testDeleteTask() {
        TaskManager manager;

        const Task first = manager.addTask("First");
        const Task second = manager.addTask("Second");

        check(manager.deleteTask(first.id),
              "Should successfully delete an existing task");

        const auto tasks = manager.listTasks();

        check(tasks.size() == 1, "Task count should decrease after deletion");
        check(tasks[0].id == second.id, "Incorrect task remains after deletion");
    }

    // 9. Invalid IDs
    void testInvalidIds() {
        TaskManager manager;
        manager.addTask("Existing task");

        constexpr int64_t invalidId = 999;

        check(!manager.setCompleted(invalidId, true),
              "Completing an invalid ID should return false");

        check(!manager.setPriority(invalidId, Priority::High),
              "Changing priority of an invalid ID should return false");

        check(!manager.deleteTask(invalidId),
              "Deleting an invalid ID should return false");

        check(manager.listTasks().size() == 1,
              "Invalid operations should not modify existing tasks");
    }

    // 10. Deleting all tasks
    void testDeleteAllTasks() {
        TaskManager manager;

        const Task first = manager.addTask("First");
        const Task second = manager.addTask("Second");

        check(manager.deleteTask(first.id), "First deletion failed");
        check(manager.deleteTask(second.id), "Second deletion failed");

        check(manager.listTasks().empty(),
              "Task list should be empty after deleting all tasks");
    }

}

int main() {
    runTest("Add task", testAddTask);
    runTest("Default values", testDefaultValues);
    runTest("Unique IDs", testUniqueIds);
    runTest("List tasks", testListTasks);
    runTest("Set completed status", testSetCompleted);
    runTest("Filter completed tasks", testCompletedFilter);
    runTest("Set priority", testSetPriority);
    runTest("Delete task", testDeleteTask);
    runTest("Invalid IDs", testInvalidIds);
    runTest("Delete all tasks", testDeleteAllTasks);

    std::cout << "\n-----------------------------\n";
    std::cout << "Tests passed: " << passed << '\n';
    std::cout << "Tests failed: " << failed << '\n';
    std::cout << "-----------------------------\n";

    return failed == 0 ? 0 : 1;
}
