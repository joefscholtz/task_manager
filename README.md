## Task Manager

Calendar and task management library

## Prerequisites

- cmake
- make
- just \[Optional\]

## Build

Using just:

```bash
just build
```

Without just:

```bash
mkdir -p build
cmake -B build
make -j -C build
```

## CLI

Using just:

```bash
just run
```

Without just:

```bash
./build/core/task_manager_cli
```

You'll enter the task_manager_cli and the available commands are as follows:

```bash
task_manager> help
Available commands:
  create_event <title> - Create a new event with the given title
  help [command] - Show help for commands
  list_events - List all events

```
