## Task Manager

Calendar and task management library. Project structure:

```
core/
├── CMakeLists.txt
├── include/
│   ├── calendar.hpp
│   ├── cli.hpp
│   ├── core.hpp
│   ├── db.hpp
│   ├── defines.hpp
│   ├── event.hpp
│   └── time.hpp
└── src/
    ├── calendar.cpp
    ├── cli.cpp
    ├── db.cpp
    └── event.cpp
deamon/
CMakeLists.txt
history.txt
justfile
README.md

```

## Prerequisites

- cmake
- make
- sqlite3
- vcpkg
-
- just \[Optional\]
  Daemon
- pkg-config
- systemd-devel

you migh need to

```bash
sudo find /usr -name "systemd.pc"
#check if its it present in
pkg-config --variable pc_path pkg-config
#if it is not, you should add to PKG_CONFIG_PATH manually

PKG_CONFIG_PATH_TO_ADD=$(pkg-config --variable pc_path pkg-config)

export PKG_CONFIG_PATH=$PKG_CONFIG_PATH_TO_ADD:$PKG_CONFIG_PATH
```

consider adding it to your `~/.bashrc` or `~/.zshrc` file

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

##TODO

In `calendar.cpp`:

- [] use log library

Whole project:

- [] Continuous integration
  - [] database schema change detection
