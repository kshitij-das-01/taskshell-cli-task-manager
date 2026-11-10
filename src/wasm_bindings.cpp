//
// Embind glue: exposes TaskManager to JS. Domain Layer stays pure C++
//

#include "task_manager.h"
#include "task.h"

#include <emscripten/bind.h>
#include <vector>

using namespace emscripten;

EMSCRIPTEN_BINDINGS(task_manager_module) {
    // enum class Priority -> JS: Module.Priority.High etc
    enum_<Priority>("Priority")
        .value("None", Priority::None)
        .value("Low", Priority::Low)
        .value("Medium", Priority::Medium)
        .value("High", Priority::High);

    // Task struct -> plain JS object, copied by value across the boundary
    value_object<Task>("Task")
        .field("id", &Task::id)
        .field("title", &Task::title)
        .field("description", &Task::description)
        .field("priority", &Task::priority)
        .field("completed", &Task::completed)
        .field("created_at", &Task::created_at);

    register_vector<Task>("TaskList");

    // TaskManager class -> JS class: new Module.TaskManager()
    class_<TaskManager>("TaskManager")
        .constructor<>()
        .function("addTask", &TaskManager::addTask)
        .function("listTasks", &TaskManager::listTasks)
        .function("setCompleted", &TaskManager::setCompleted)
        .function("setPriority", &TaskManager::setPriority)
        .function("deleteTask", &TaskManager::deleteTask);
}
