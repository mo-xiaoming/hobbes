#include "format.H"
#include "test.H"

#include <cstdlib>
#include <hobbes/util/perf.H>

#include <algorithm>
#include <fstream>
#include <functional>
#include <ostream>

#include <getopt.h>

namespace test_detail {
TestCoord& TestCoord::instance() {
  static TestCoord tc;
  return tc;
}

bool TestCoord::installTest(const std::string& group, const std::string& test, PTEST pf) {
  tests[group].emplace_back(test, pf);
  results[group].emplace_back(test);
  return true;
}

std::vector<std::string> TestCoord::testGroupNames() const {
  std::vector<std::string> r;
  r.reserve(tests.size());
  std::transform(tests.cbegin(), tests.cend(), std::inserter(r, r.begin()),
                 [](const auto& t) { return std::cref(t.first); });
  return r;
}

int TestCoord::runTestGroups(const Args& args) {
  std::vector<std::string> failures;
  const auto& groups = args.groupNames;

  const auto updateTerminal = [&failures](const std::string& name, const Result& result) {
    std::cout << "    " << name;
    if (result.status == Result::Status::Pass) {
      std::cout << " SUCCESS ";
    } else if (result.status == Result::Status::Fail) {
      std::cout << " FAIL ";
      failures.push_back(result.error);
    }
    std::cout << "(" << hobbes::describeNanoTime(result.duration) << ")" << std::endl;
  };

  std::cout << "Running " << groups.size() << " group" << (groups.size() == 1 ? "" : "s")
            << " of tests\n---------------------------------------------------------------------\n"
            << std::endl;

  long tt0 = hobbes::tick();
  for (const auto& gn : groups) {
    auto gi = this->tests.find(gn);
    if (gi == this->tests.end()) {
      std::cout << "ERROR: no test group named '" << gn << "' exists" << std::endl;
      continue;
    }
    const auto& g = gi->second;
    auto& r = this->results[gn];

    std::cout << "  " << gn << " (" << g.size() << " test" << (g.size() == 1 ? "" : "s")
              << ")\n  ---------------------------------------------------------" << std::endl;

    long gt0 = hobbes::tick();
    for (size_t i = 0; i < g.size(); ++i) {
      const auto& t = g[i];
      auto& result = r[i];
      long t0 = hobbes::tick();
      try {
        t.second();
        result.recordPass(hobbes::tick() - t0);
      } catch (std::exception& ex) {
        result.recordFail(hobbes::tick() - t0, "[" + gn + "/" + t.first + "]: " + ex.what());
      }
      updateTerminal(t.first, result);
    }
    std::cout << "  ---------------------------------------------------------\n  "
              << hobbes::describeNanoTime(hobbes::tick() - gt0) << '\n'
              << std::endl;
  }
  std::cout << "---------------------------------------------------------------------\n"
            << hobbes::describeNanoTime(hobbes::tick() - tt0) << std::endl;

  if (!failures.empty()) {
    std::cout << "\n\nFAILURE" << (failures.size() == 1 ? "" : "S")
              << ":\n---------------------------------------------------------------------\n";
    for (const auto& failure : failures) {
      std::cout << failure << std::endl;
    }
  }

  if (const auto* path = args.report) {
    std::ofstream outfile(path, std::ios::out | std::ios::trunc);
    if (outfile) {
      outfile << toJSON();
      std::cout << "JSON report generated: " << path << std::endl;
    } else {
      std::cerr << "error in generating JSON report: " << ::strerror(errno) << std::endl;
    }
  }

  return static_cast<int>(failures.size());
}

std::string TestCoord::toJSON() {
  std::ostringstream os;
  showJSON(std::vector<GroupedResults::value_type>(results.begin(), results.end()), os);
  return os.str();
}
} // namespace test_detail

namespace {
void listTest() {
  for (const auto& gn : test_detail::TestCoord::instance().testGroupNames()) {
    std::cout << gn << std::endl;
  }
}

void usage() {
  std::cout << "hobbes-test [--list_tests][--tests <name> [--tests <name>...][--json <path>]]\n";
}

bool parseArgs(int argc, char** argv, test_detail::Args& args) {
  // clang-format off
  static const option options[] = {
    {"help",       no_argument,       nullptr, 'h'},
    {"list_tests", no_argument,       nullptr, 'l'},
    {"tests",      required_argument, nullptr, 't'},
    {"json",       required_argument, nullptr, 'r'},
    {nullptr,      no_argument,       nullptr, ' '}
  };
  // clang-format on

  int key = 0;
  while ((key = ::getopt_long(argc, argv, "hlt:r:", options, nullptr)) != -1) {
    switch (key) {
    case 'l':
      listTest();
      return false;
    case 't':
      args.groupNames.emplace_back(optarg);
      break;
    case 'r':
      args.report = optarg;
      break;
    case 'h':
    case '?':
    default:
      usage();
      return false;
    }
  }
  if (args.groupNames.empty()) {
    args.groupNames = test_detail::TestCoord::instance().testGroupNames();
  }
  return true;
}
} // namespace

int main(int argc, char** argv) {
  test_detail::Args args;
  if (!parseArgs(argc, argv, args)) {
    return EXIT_FAILURE;
  }
  return test_detail::TestCoord::instance().runTestGroups(args);
}
