#include <chrono>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <unistd.h>

namespace {
struct Options {
  int timeout = -1;
  bool valid = false;
};

Options getOptions(int argc, char **argv) {
  if (argc != 2) {
    return {};
  }
  std::size_t pos = 0;
  const auto g = std::string(argv[1]);
  const auto t = std::stoi(g, &pos);
  if (pos != g.size() || t <= 0) {
    return {};
  }
  return {.timeout = t, .valid = true};
}

extern "C" [[noreturn]] void exitApp(int /* sig */) {
  std::_Exit(EXIT_SUCCESS);
}
} // namespace

// argv[1] is the seconds before process exists
int main(int argc, char **argv) {
  const auto o = getOptions(argc, argv);
  if (!o.valid) {
    return EXIT_FAILURE;
  }

  (void)std::signal(SIGTERM, exitApp);

  for (int i = 0; i < o.timeout; ++i) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  const int success = 1;
  const auto rc = write(STDOUT_FILENO, &success, sizeof(success));
  if (rc == -1) {
    return EXIT_FAILURE;
  }
}
