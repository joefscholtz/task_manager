alias b := build
alias re:=rebuild
alias rm:=remove-db

default:
  just --list

install:
  @echo "not implemented"

rebuild: && build
  rm -rf build

configure:
    cmake --preset default

build: configure
    cmake --build --preset default --parallel

run:
  ./build/core/task_manager_cli

remove-db:
  rm ~/.local/share/task_manager/task_manager.sqlite3
  rm .env/gcal_token.json
