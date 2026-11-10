# Emscripten Build Guide — taskshell-cli-task-manager

Complete reference for building the C++ domain logic as WebAssembly with `embind` bindings exposed to JavaScript.

---

## 1. Project Layout

```
.
├── CMakeLists.txt               # Build configuration (native + WASM targets)
├── include/
│   ├── task.h                   # Task struct + Priority enum (pure C++)
│   └── task_manager.h           # TaskManager class (pure C++)
├── src/
│   ├── task.cpp
│   ├── task_manager.cpp         # CRUD + business rules (pure C++)
│   └── wasm_bindings.cpp        # embind glue (exposes TaskManager to JS)
├── web/
│   ├── task_manager.js          # Generated WASM glue (browser + node)
│   ├── task_manager.wasm        # Generated WASM binary
│   └── smoke.cjs / smoke.mjs    # Smoke-test scripts
├── build-wasm/                  # WASM build directory (gitignored)
├── docs/
│   ├── PROJECT_PLAN.md
│   └── EMSCRIPTEN_BUILD_GUIDE.md (this file)
└── test/                        # Unit tests (native only)
```

---

## 2. Prerequisites

| Tool | Purpose | Install |
|------|---------|---------|
| `emsdk` | Emscripten SDK manager | `git clone https://github.com/emscripten-core/emsdk.git ~/dev/emsdk` |
| `emcc` / `em++` | Emscripten C/C++ compiler | `~/dev/emsdk/emsdk install latest && ~/dev/emsdk/emsdk activate latest` |
| `emcmake` | CMake wrapper for Emscripten | Comes with emsdk |
| `node` | Runtime for smoke tests | `brew install node` or from nodejs.org |

Verify:

```bash
emcc --version
emcmake --version
```

Expected: `emcc (Emscripten gcc/clang-like replacement...) 3.1.x`

---

## 3. The Two Build Flavours

### 3.1 Native build (local testing only)

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

- Produces `build/taskshell_cli` (when `BUILD_CLI_APP=ON`).
- Does **not** produce WASM.
- The `task_manager` static library (`libtask_manager.a`) is built here too.

### 3.2 WASM build (browser / node)

```bash
rm -rf build-wasm
emcmake cmake -B build-wasm -DCMAKE_BUILD_TYPE=Debug
cmake --build build-wasm
```

- `emcmake` injects the Emscripten toolchain file into CMake.
- Produces `web/task_manager.js` + `web/task_manager.wasm`.
- The `.js` file is the embind glue + WASM loader; the `.wasm` is the compiled module.

> **Tip:** Always delete `build-wasm/` before switching between native and WASM builds to avoid stale cache issues.

---

## 4. CMakeLists.txt — WASM Section Explained

The `if(EMSCRIPTEN)` block in `CMakeLists.txt` controls the WASM target. Emscripten defines the `EMSCRIPTEN` variable automatically when running under `emcmake`.

```cmake
if (EMSCRIPTEN)
    add_executable(taskshell_wasm
        src/wasm_bindings.cpp   # only the embind glue; task.cpp/task_manager.cpp
    )                          # come from libtask_manager.a via link

    target_include_directories(taskshell_wasm PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    target_link_libraries(taskshell_wasm PRIVATE task_manager)

    set_target_properties(taskshell_wasm PROPERTIES
        OUTPUT_NAME "task_manager"           # → task_manager.js / .wasm
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/web
    )

    target_link_options(taskshell_wasm PRIVATE
        SHELL:--bind                          # enable embind runtime
        SHELL:-sENVIRONMENT=web,node          # works in browser + node
        SHELL:-sALLOW_MEMORY_GROWTH=1         # grow heap if tasks exceed initial size
        SHELL:-sEXPORTED_FUNCTIONS=['_force_keep_bindings']
    )
endif()
```

### Key flags

| Flag | What it does |
|------|--------------|
| `--bind` | Links the embind runtime and enables `EMSCRIPTEN_BINDINGS` registration. |
| `-sENVIRONMENT=web,node` | Tells Emscripten the module runs in both browser and node. |
| `-sALLOW_MEMORY_GROWTH=1` | Allows the WASM heap to grow; prevents OOM aborts when many tasks exist. |
| `-sEXPORTED_FUNCTIONS=['_force_keep_bindings']` | Exports a C++ keeper function so Emscripten does not tree-shake embind static initializers. |
| `-sMODULARIZE=1` | (Optional) Wraps output as `module.exports = createModule(...)` for `require()`. |
| `-sEXPORT_NAME=createModule` | Names the factory function when MODULARIZE is used. |

> **Note on `SHELL:` prefix:** CMake 3.13+ allows `SHELL:` to pass arguments through to the underlying shell, avoiding quoting issues with `['...']` lists.

---

## 5. wasm_bindings.cpp — embind Glue

This is the only file that touches Emscripten. The domain (`task.cpp`, `task_manager.cpp`) remains pure C++ with no WASM knowledge.

```cpp
#include "task_manager.h"
#include "task.h"

#include <emscripten/bind.h>
#include <vector>

using namespace emscripten;

EMSCRIPTEN_BINDINGS(task_manager_module) {
    // 1. Register std::vector<Task> so listTasks() can return it to JS
    register_vector<Task>("TaskList");

    // 2. Expose the Priority enum
    enum_<Priority>("Priority")
        .value("None",   Priority::None)
        .value("Low",    Priority::Low)
        .value("Medium", Priority::Medium)
        .value("High",   Priority::High);

    // 3. Expose the Task struct as a plain JS object
    value_object<Task>("Task")
        .field("id",          &Task::id)
        .field("title",       &Task::title)
        .field("description", &Task::description)
        .field("priority",    &Task::priority)
        .field("completed",   &Task::completed)
        .field("created_at",  &Task::created_at);

    // 4. Expose TaskManager class
    class_<TaskManager>("TaskManager")
        .constructor<>()
        .function("addTask",      &TaskManager::addTask)
        .function("listTasks",    &TaskManager::listTasks)
        .function("setCompleted", &TaskManager::setCompleted)
        .function("setPriority",  &TaskManager::setPriority)
        .function("deleteTask",   &TaskManager::deleteTask);
}

// Keeper: references TaskManager so Emscripten keeps embind type registrations
extern "C" EMSCRIPTEN_KEEPALIVE
TaskManager* force_keep_bindings() {
    return new TaskManager();
}
```

### Registration order matters

Inside `EMSCRIPTEN_BINDINGS { ... }`, register types **before** they are referenced:

1. `register_vector<Task>` — must precede any function returning `std::vector<Task>`.
2. `enum_<Priority>` — must precede any method taking/returning `Priority`.
3. `value_object<Task>` — must precede any class that uses `Task` in signatures.
4. `class_<TaskManager>` — last, since methods reference all the above.

### Vector lifetime (important)

`listTasks()` returns an embind `TaskList` object (JS-owned heap wrapper). You **must** call `.delete()` on it to avoid leaking WASM memory:

```js
const list = tm.listTasks();
// ... iterate ...
list.delete();   // required
```

---

## 6. Smoke Test

### Using `require()` (CommonJS, recommended)

```js
// web/smoke.cjs
const M = require('./task_manager.js');

const tm = new M.TaskManager();

const t = tm.addTask('Buy milk', '2% fat', M.Priority.High);
console.log('added:', t.id, t.title, t.priority);

const list = tm.listTasks();
for (let i = 0; i < list.size; i++) {
    console.log('task:', list.get(i).title);
}
list.delete();

console.log('complete:', tm.setCompleted(t.id, true));
console.log('delete:', tm.deleteTask(t.id));
```

Run: `node web/smoke.cjs`

### Using ESM `import` (optional)

Requires `-sMODULARIZE=1 -sEXPORT_ES6=1` in CMake. The output supports `import createModule from './task_manager.js'` in Node ≥14 (`.mjs` extension) or bundlers.

---

## 7. Build Commands Cheat Sheet

| Task | Command |
|------|---------|
| Full clean WASM rebuild | `rm -rf build-wasm && emcmake cmake -B build-wasm -DCMAKE_BUILD_TYPE=Debug && cmake --build build-wasm` |
| Incremental WASM rebuild | `cmake --build build-wasm` |
| Native build | `cmake -B build && cmake --build build` |
| Run smoke test | `node web/smoke.cjs` |
| Check Emscripten version | `emcc --version` |
| Verify WASM output | `ls -la web/task_manager.js web/task_manager.wasm` |

---

## 8. Common Bugs & Fixes

### 8.1 `UnboundTypeError: Cannot call TaskManager.addTask due to unbound types: x`

**Symptoms:** Smoke test fails at `tm.addTask(...)` with `UnboundTypeError`.

**Cause:** Emscripten's dead-code elimination stripped the embind static initializer (`embind_init_task_manager_module`) from the generated JS. The `TaskManager` class exists as a shell but its parameter types (`std::string`, `Priority`, `Task`) are unbound.

**Fixes (try in order):**

1. **Add `EMSCRIPTEN_KEEPALIVE` keeper function** in `wasm_bindings.cpp`:

   ```cpp
   extern "C" EMSCRIPTEN_KEEPALIVE
   TaskManager* force_keep_bindings() {
       return new TaskManager();
   }
   ```

2. **Export the keeper** in `CMakeLists.txt`:

   ```cmake
   target_link_options(taskshell_wasm PRIVATE
       SHELL:--bind
       SHELL:-sENVIRONMENT=web,node
       SHELL:-sALLOW_MEMORY_GROWTH=1
       SHELL:-sEXPORTED_FUNCTIONS=['_force_keep_bindings']
   )
   ```

3. **Add `-O0`** to disable Emscripten's optimizer (Debug build type does this automatically).

4. **Clean rebuild:** `rm -rf build-wasm && emcmake cmake -B build-wasm && cmake --build build-wasm`.

### 8.2 `createModule is not a function` / `require(...)` returns empty object

**Symptoms:** `const M = require('./task_manager.js')` gives `{}` or `createModule` is undefined.

**Cause:** `-sMODULARIZE=1` is missing or commented out in CMakeLists.txt. Without it, Emscripten emits a global `Module` script, not a CommonJS module.

**Fix:** Ensure `CMakeLists.txt` has:

```cmake
target_link_options(taskshell_wasm PRIVATE
    ...
    SHELL:-sMODULARIZE=1
    SHELL:-sEXPORT_NAME=createTaskManagerModule
)
```

Then rebuild. If you don't want MODULARIZE, use the global pattern:

```js
// No MODULARIZE: the script defines a global `Module`-like object
const M = require('./task_manager.js'); // may need different handling
```

Or use the classic HTML `<script src="task_manager.js"></script>` pattern in a browser.

### 8.3 `TypeError: Cannot read properties of undefined (reading 'High')`

**Symptoms:** `M.Priority` is undefined; `M.Prority.High` (typo).

**Cause:** Typo in smoke test (`Prority` instead of `Priority`), or embind enum registration was stripped (see 8.1).

**Fix:** Correct the spelling to `M.Priority.High`. If still undefined, re-apply fix 8.1.

### 8.4 `error: expected parameter declarator` on `register_vector<Task>("TaskList")`

**Symptoms:** Compile error at namespace scope on `register_vector` line.

**Cause:** `register_vector` is a statement and cannot live at file/namespace scope. It must be inside `EMSCRIPTEN_BINDINGS { ... }`.

**Fix:** Move `register_vector<Task>("TaskList");` to be the **first line** inside the `EMSCRIPTEN_BINDINGS` block.

### 8.5 `no viable constructor or deduction guide for deduction of template arguments of 'arg'`

**Symptoms:** Compile error on `.function("addTask", &TaskManager::addTask, arg("title"), ...)`.

**Cause:** `arg(...)` template deduction doesn't accept string literals with default values reliably in Emscripten 3.1.x.

**Fix:** Omit `arg(...)` wrappers entirely. C++ default arguments carry through to JS automatically:

```cpp
.function("addTask", &TaskManager::addTask)   // defaults from C++ header apply
```

### 8.6 `wasm-ld: error: symbol exported via --export not found: force_keep_bindings`

**Symptoms:** Link fails because `-sEXPORTED_FUNCTIONS=['force_keep_bindings']` lacks the leading underscore.

**Cause:** WASM symbol names are mangled with a leading `_` prefix in the export list.

**Fix:** Use `-sEXPORTED_FUNCTIONS=['_force_keep_bindings']` (with underscore). Or drop the flag entirely and rely on `EMSCRIPTEN_KEEPALIVE` alone.

### 8.7 `Memory growth` / `OOM` abort

**Symptoms:** `abort("Cannot enlarge memory arrays")` or `OOM` when adding many tasks.

**Cause:** Default WASM heap (16 MB) is exhausted.

**Fix:** `-sALLOW_MEMORY_GROWTH=1` is already in the CMakeLists. If still hitting limits, increase initial heap: `SHELL:-sINITIAL_MEMORY=33554432` (32 MB).

### 8.8 `TaskList` not deleted → memory leak

**Symptoms:** WASM memory grows over time even after tasks are deleted.

**Cause:** `listTasks()` returns an embind-owned `TaskList` JS object that isn't freed.

**Fix:** Always call `list.delete()` after iterating:

```js
const list = tm.listTasks();
// ... use ...
list.delete();
```

### 8.9 CMake cache stale between native and WASM builds

**Symptoms:** `emcmake cmake` complains about mismatched toolchain, or wrong compiler used.

**Cause:** CMake cache from a native build (`build/`) leaks into the WASM configure step.

**Fix:** Always use separate directories and clean before switching:

```bash
rm -rf build-wasm
emcmake cmake -B build-wasm   # fresh
```

Never reuse `build/` for WASM.

---

## 9. Architecture Notes

- **Domain layer is pure C++:** `task.cpp`, `task_manager.cpp`, and headers contain zero Emscripten/embind code. They compile identically for native and WASM.
- **Interface layer is `wasm_bindings.cpp`:** Only this file includes `<emscripten/bind.h>` and uses `EMSCRIPTEN_BINDINGS`.
- **`listTasks(bool completed_only)` behavior:** When `completed_only=false` it returns **all** tasks (despite the name). When `true` it returns only completed tasks. This is the current domain contract; JS callers should rely on this documented behavior, not the name.

---

## 10. Adding New Bound Methods

1. Add the method to `TaskManager` in `include/task_manager.h` and `src/task_manager.cpp`.
2. Add `.function("name", &TaskManager::name)` to `wasm_bindings.cpp` inside `EMSCRIPTEN_BINDINGS`.
3. If the method takes/returns a new type (e.g. `std::optional`, `std::map`), add the corresponding embind registration (`register_vector`, `value_object`, `enum_`, etc.).
4. Rebuild: `cmake --build build-wasm`.
5. Update `web/smoke.cjs` to exercise the new method.

---

## 11. Deploying to Static Host

The `web/` folder contains static assets:

- `task_manager.js` — embind glue + WASM loader
- `task_manager.wasm` — compiled module
- `index.html` + `app.js` — browser UI (to be built in Step 6 of the project plan)

Deploy by copying `web/` to any static host (GitHub Pages, Netlify, Vercel). No server-side runtime needed.

---

## 12. Quick Troubleshooting Checklist

```
□ emcc --version shows 3.1.x
□ emcmake was used (not plain cmake) for build-wasm
□ build-wasm/ was deleted before switching build flavours
□ wasm_bindings.cpp has register_vector inside EMSCRIPTEN_BINDINGS (not outside)
□ register_vector appears before enum_, value_object, class_ in the block
□ --bind is in target_link_options
□ EMSCRIPTEN_KEEPALIVE keeper function exists in wasm_bindings.cpp
□ _force_keep_bindings is in -sEXPORTED_FUNCTIONS
□ smoke test uses require() if no MODULARIZE, or import if MODULARIZE+EXPORT_ES6
□ list.delete() is called after listTasks()
□ Typo check: M.Priority, not M.Prority
```

---

*End of guide. Keep this file updated as the WASM surface area grows (Step 6 UI, Step 7 localStorage, etc.).*
