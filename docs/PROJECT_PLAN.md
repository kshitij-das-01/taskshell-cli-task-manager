# Project Plan: C++ CLI Task Manager (WASM / Browser)

*Based on `PROJECT_PLAN_TEMPLATE.md` and `PROJECT_PLANNING_GUIDE.md`. Phases 1–12 completed.*

---

## 1. Problem Statement

> I want to build a C++ task manager so that users can track, prioritize, and complete tasks with a simple CLI-style interface, all through their web browser without installing anything or using a graphical UI.

---

## 2. Constraints & Goals

| Question | Answer |
|---|---|
| Who uses it? | Personal usage |
| What problem does it solve? | Minimalist CLI-based task manager available in a web browser |
| Where does it run? | Browser (WASM) |
| Hard constraints | C++ and WASM-compatible |
| Success criteria (MVP) | Add, list, prioritize, complete, delete tasks |
| Out of scope | Native CLI binary as a shipped product (CLI binary may be built only for local testing) |

---

## 3. Scope Tiers

| Tier | Features |
|---|---|
| **MVP** | Add task (title, priority), list all tasks, mark task complete/incomplete, delete task, change priority. In-memory only (data lost on refresh). |
| **v1** | `localStorage` persistence so tasks survive browser refresh, due dates, filters (all / active / completed), sorting by priority or due date. |
| **Future** | Categories/tags, search, drag-and-drop reordering, native CLI binary as a shipped product, sync/backup, multiple task lists. |

---

## 4. Domain Model

### 4.1 Entities

| Entity | Attributes |
|---|---|
| `Task` | `id` (int), `title` (string), `description` (string, optional), `priority` (enum: none/low/medium/high), `completed` (bool), `created_at` (timestamp) |
| `TaskManager` | Collection of `Task`s, auto-incrementing ID counter |

### 4.2 Business Rules / Invariants

- IDs are unique and auto-incremented.
- A task can only be marked complete/incomplete if it exists.
- Deleting a non-existent task is a no-op / returns an error.
- Priority must be one of the defined enum values (none, low, medium, high).

---

## 5. User Stories / Use Cases

*(Skipped per project decision; to be revisited during implementation.)*

---

## 6. Architecture Diagram

```
┌─────────────────────────────────┐
│ Interface Layer                 │  ← Browser CLI-style UI (HTML/JS)
│ (Terminal-like UI + WASM glue)  │     embind bindings to C++
├─────────────────────────────────┤
│ Application Layer               │  ← Use cases: add/list/complete/
│ (Services)                      │     delete/set-priority tasks
├─────────────────────────────────┤
│ Domain Layer                    │  ← Task struct, TaskManager,
│ (Entities + rules)              │     business invariants (pure C++)
├─────────────────────────────────┤
│ Infrastructure Layer            │  ← MVP: in-memory storage
│ (Persistence adapters)          │     v1: localStorage adapter
└─────────────────────────────────┘
```

**Key rule:** Domain and Application layers are pure C++ with no knowledge of WASM, HTML, or the browser. The Interface layer (JS + embind) sits on top and calls into the C++ core.

---

## 7. Module Decomposition

| Module | Responsibility | Depends On |
|---|---|---|
| `task` | `Task` data struct + priority enum | none |
| `task_manager` | CRUD + business rules (add, list, complete, delete, set-priority) | `task` |
| `wasm_bindings` | Emscripten `embind` glue exposing `TaskManager` to JS | `task_manager` |
| `web_ui` | Browser CLI-style UI (HTML/JS rendering, input handling) | `wasm_bindings` (via JS) |
| `cli_app` | Native terminal REPL for local testing only | `task_manager` |
| `tests` | Unit tests for domain + application logic | `task_manager` |

---

## 8. Interface Signatures

*(Skipped per project decision; to be defined during implementation.)*

---

## 9. Core Data Flow (Example Use Case: "Add Task")

1. User types a task title in the browser UI and submits.
2. JS calls `taskManager.addTask(title, description, priority)` across the WASM/embind boundary.
3. `TaskManager` (pure C++) validates input, assigns a new auto-incremented ID, stores the task.
4. Returns the new `Task` object back to JS.
5. JS re-renders the task list in the browser UI.

---

## 10. Tech Stack

| Concern | Selection |
|---|---|
| Language | C++17/20 |
| Build system | CMake + Emscripten toolchain |
| Testing framework | Catch2 or GoogleTest |
| CI/CD | GitHub Actions |
| Hosting / deployment | GitHub Pages / Netlify (static hosting) |

---

## 11. Build / Test / Deploy Pipeline

1. Compile `task` + `task_manager` + unit tests as native C++ (no WASM).
2. Run unit tests on every change / push.
3. Compile WASM bindings via Emscripten → `task_manager.js` + `task_manager.wasm`.
4. Copy `web/` assets + WASM output to static host (GitHub Pages / Netlify).
5. Smoke test: open the hosted page, add a task, refresh, verify it appears.

---

## 12. Implementation Backlog

| Order | Step | Deliverable |
|---|---|---|
| 1 | Repo init + CMake | Compilable project skeleton |
| 2 | `Task` + `TaskManager` domain logic | Pure C++ library |
| 3 | Unit tests | Passing test suite |
| 4 | Native CLI (`cli_app`) | Working terminal task manager (local testing only) |
| 5 | Emscripten WASM target + `embind` | `task_manager.js` + `.wasm` |
| 6 | Minimal `index.html` + `app.js` | Browser UI renders tasks from WASM |
| 7 | `localStorage` persistence | Tasks survive refresh (v1) |
| 8 | Deploy to static host | Live URL |

---

*Document finalized. Ready for implementation discussion.*