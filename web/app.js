import createTaskManagerModule from "./task_manager.js";

const terminal = document.getElementById("terminal");
const input = document.getElementById("command-input");
const form = document.getElementById("command-form");
const button = form.querySelector("button");
const status = document.getElementById("status");

let Module;
let manager;

const history = [];
let historyIndex = 0;

// Terminal Output
function print(message = "", type = "") {
    const line = document.createElement("div");

    if (type) {
        line.className = type;
    }

    line.textContent = message;

    terminal.appendChild(line);
    terminal.scrollTop = terminal.scrollHeight;
}

function setStatus(message, type = "") {
    status.textContent = message;
    status.className = `status ${type}`;
}

// Initialization
async function initialize() {
    try {
        setStatus("Loading WebAssembly...");

        Module = await createTaskManagerModule();
        console.log("WASM initialized successfully");
        manager = new Module.TaskManager();
        console.log("TaskManager instance created");

        setStatus("Connect", 'ready');

        print("WebAssembly initialized successfully.", "success");
        print("C++ TaskManager instance created.", "success");
        print("");
        print("Type 'help' to view available commands.");
        print("");

        input.disabled = false;
        button.disabled = false;
        input.focus();
    }
    catch (error) {
        console.error("WASM initialization failed: ", error);

        setStatus("Initialization failed", "error");
        print(`Error: ${error.message}`, "error");
    }
}

// Utilities
function getPriorities() {
    return {
        none: Module.Priority.None,
        low: Module.Priority.Low,
        medium: Module.Priority.Medium,
        high: Module.Priority.High
    };
}

function parseId(value) {
    if (!/^\d+$/.test(value)) {
        throw new Error("Invalid task ID.");
    }

    return BigInt(value);
}

function priorityName(priority) {
    const names = ["none", "low", "medium", "high"];
    return names[priority?.value] ?? "none";
}

// Command processor (JS smoke test)
function executeCommand(command) {
    const trimmed = command.trim();

    if (!trimmed)
        return;

    print(`taskshell > ${trimmed}`, "muted");

    const priorities = getPriorities();

    const [action, ...args] = trimmed.split(/\s+/);

    switch (action.toLowerCase()) {
        case "help":
            print("Available commands:", "success");
            print("  add <title> [priority]");
            print("  list");
            print("  done <id>");
            print("  priority <id> <level>");
            print("  delete <id>");
            print("  clear");
            print("  help");
            break;

        case "add": {
            const rest = trimmed.slice(4).trim();

            if (!rest) {
                throw new Error("Usage: add <title> [priority]");
            }

            const parts = rest.split(/\s+/);
            const last = parts.at(-1).toLowerCase();

            const hasPriority = Object.hasOwn(priorities, last);
            const priority = hasPriority ? priorities[last] : Module.Priority.None;

            const title = hasPriority ? parts.slice(0, -1).join(" ") : rest;

            if (!title.trim()) {
                throw new Error("Task title cannot be empty.");
            }

            const task = manager.addTask(
                title,
                "",
                priority
            );

            print(
                `Added task #${task.id}: ${task.title} [${priorityName(task.priority)}]`,
                "success"
            );
            break;
        }

        case "list": {
            const tasks = manager.listTasks(false);

            try {
                if (tasks.size() === 0) {
                    print("No tasks yet.", "muted");
                    break;
                }

                print("Your tasks: ", "success");

                for (let i = 0; i < tasks.size(); i++) {
                    const task = tasks.get(i);

                    const completed = task.completed ? "x" : " ";
                    const priority = priorityName(task.priority);

                    print(
                        `[${completed}] #${task.id} (${priority}) ${task.title}`
                    );
                }
            }
            finally {
                tasks.delete();
            }

            break;
        }

        case "done": {
            if (args.length !== 1) {
                throw new Error("Usage: done <id>");
            }

            const id = parseId(args[0]);

            if (manager.setCompleted(id, true)) {
                print(`Task #${id} marked completed.`, "success");
            }
            else {
                print(`Task #${id} not found.`, "error");
            }

            break;
        }

        case "priority": {
            if (args.length !== 2) {
                throw new Error("Usage: priority <id> <level>");
            }

            const id = parseId(args[0]);
            const level = args[1].toLowerCase();

            if (!Object.hasOwn(priorities, level)) {
                throw new Error("Priority must be none, low, medium, or high.");
            }

            if (manager.setPriority(BigInt(id), priorities[level])) {
                print(`Task #${id} priority changed to ${level}.`, "success");
            }
            else {
                print(`Task #${id} not found.`, "error");
            }

            break;
        }

        case "delete": {
            if (args.length !== 1) {
                throw new Error("Usage: delete <id>");
            }

            const id = parseId(args[0]);

            if (manager.deleteTask(id)) {
                print(`Task #${id} deleted.`, "success");
            }
            else {
                print(`Task #${id} not found.`, "error");
            }

            break;
        }

        case "clear":
            terminal.replaceChildren();
            break;

        default:
            print(`Unknown command: ${action}`, "error");
            print("Type 'help' for available commands.");
    }
}

// Input handling
form.addEventListener("submit", (event) => {
    event.preventDefault();

    const command = input.value.trim();

    if (!command || !manager)
        return;

    history.push(command);
    historyIndex = history.length;

    try {
        executeCommand(command);
    } catch (error) {
        print(`Error: ${error.message}`, "error");
        console.error(error);
    }

    input.value = "";
    input.focus();
});

input.addEventListener("keydown", (event) => {
    if (event.key === "ArrowUp") {
        event.preventDefault();

        if (historyIndex > 0) {
            historyIndex--;
            input.value = history[historyIndex];
        }
    }

    if (event.key === "ArrowDown") {
        event.preventDefault();

        if (historyIndex < history.length - 1) {
            historyIndex++;
            input.value = history[historyIndex];
        }
        else {
            historyIndex = history.length;
            input.value = "";
        }
    }
});

initialize();
