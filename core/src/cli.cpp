#include "core.hpp"
#include <iostream>
#include <replxx.hxx>
#include <sstream>

using namespace task_manager;
using namespace replxx;

int main() {
  Calendar calendar;
  Replxx rx;

  rx.history_load("history.txt"); // save/restore history
  rx.set_max_history_size(1000);

  while (true) {
    char const *cinput = rx.input("task_manager> ");
    if (cinput == nullptr)
      break; // EOF / Ctrl+D
    std::string input(cinput);

    if (input == "exit" || input == "quit")
      break;

    std::istringstream iss(input);
    std::string cmd;
    iss >> cmd;

    if (cmd == "create_task") {
      std::string title;
      std::getline(iss, title);
      if (!title.empty() && title[0] == ' ')
        title.erase(0, 1);
      // auto id = core.create_task(title);
      std::cout << "Created task " << ": " << title << "\n";
      // } else if (cmd == "list_event") {
      //   for (auto &t : calendar.list_events()) {
      //     std::cout << t.id << ": " << t.title << "\n";
      //   }
    } else {
      std::cout << "Unknown command\n";
    }

    rx.history_add(input);
  }

  rx.history_save("history.txt");
}
