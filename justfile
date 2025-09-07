alias b := build
alias re:=rebuild
alias rm:=remove-db

default:
  just --list

install:
  @echo "not implemented"

rebuild: && build
  rm -rf build

build:
  mkdir -p build
  cmake -B build
  make -j -C build

run:
  ./build/core/task_manager_cli

remove-db:
  rm ~/.local/share/task_manager/task_manager.db
