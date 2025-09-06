alias b := build
alias re:=rebuild

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
