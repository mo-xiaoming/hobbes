/*
 * test : a simple system for introducing tests
 */

#ifndef HOBBES_TEST_SYSTEM_HPP_INCLUDED
#define HOBBES_TEST_SYSTEM_HPP_INCLUDED

#include <functional>
#include <hobbes/util/stream.H>

#include <array>
#include <cstring>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <libgen.h>
#include <unistd.h>

#if defined(__APPLE__) && defined(__MACH__)
#  include <mach-o/dyld.h>
#elif __linux__
#  include <linux/limits.h>
#else
#  error "platform is not supported"
#endif

template <class Fn> auto execPath(Fn&& fn) -> void {

#if defined(__APPLE__) && defined(__MACH__)
#  if !defined(PROC_PIDPATHINFO_MAXSIZE) && !defined(PROC_PIDPATHINFO_MAXSIZE_HOBBES_FEATURED)
#    define PROC_PIDPATHINFO_MAXSIZE_HOBBES_FEATURED 2048
#  elif defined(PROC_PIDPATHINFO_MAXSIZE)
#    define PROC_PIDPATHINFO_MAXSIZE_HOBBES_FEATURED = PROC_PIDPATHINFO_MAXSIZE
#  endif
  std::array<char, PROC_PIDPATHINFO_MAXSIZE_HOBBES_FEATURED> r{};
  uint32_t count = PROC_PIDPATHINFO_MAXSIZE_HOBBES_FEATURED;
  if (-1 != _NSGetExecutablePath(r.data(), &count)) {
#elif __linux__
  std::array<char, PATH_MAX> r{};
  if (-1 != ::readlink("/proc/self/exe", r.data(), r.size())) {
#endif
    fn(std::string(::dirname(r.data()))); // NOLINT
  } else {
    throw std::runtime_error("failed to readlink(\"/proc/self/exe\") ...");
  }
}

namespace test_detail {
struct Args final {
  std::vector<std::string> groupNames;
  const char* report{"test_report.json"};
};

struct Result final {
  enum class Status {
    Pass,
    Fail,
    Skipped,
  };

  explicit Result(std::string name) : testcase(std::move(name)) {}

  void recordPass(long d) {
    status = Status::Pass;
    duration = d;
    error.clear();
  }

  void recordFail(long d, const std::string& msg) {
    status = Status::Fail;
    duration = d;
    error = msg;
  }

  std::string testcase;
  Status status{Status::Skipped};
  long duration{0};
  std::string error;
};

class TestCoord {
public:
  using PTEST = void (*)();
  static TestCoord& instance();
  bool installTest(const std::string& group, const std::string& test, PTEST pf);
  std::vector<std::string> testGroupNames() const;
  int runTestGroups(const Args&);

private:
  std::string toJSON();
  using Tests = std::vector<std::pair<std::string, PTEST>>;
  using GroupedTests = std::map<std::string, Tests>;
  using Results = std::vector<Result>;
  using GroupedResults = std::map<std::string, Results>;
  GroupedTests tests;
  GroupedResults results;
};

inline std::string fileinfo(const char* file, int line) {
  std::array<char, PATH_MAX> r{};
  std::strncpy(r.data(), file, r.size() - 1);
  std::ostringstream oss;
  oss << '(' << basename(r.data()) << ':' << line << ')';
  return std::move(oss).str();
}
} // namespace test_detail

#define TEST(G, N) \
  void test_##G##_##N(); \
  const bool install_##G##_##N = test_detail::TestCoord::instance().installTest(#G, #N, &test_##G##_##N); \
  void test_##G##_##N()

#define FILEINFO test_detail::fileinfo(__FILE__, __LINE__)

#define EXPECT_TRUE(p) \
  if (!(p)) { \
    std::ostringstream __errmsg; \
    __errmsg << "Expression false, expected true: " #p << " " << FILEINFO; \
    throw std::runtime_error(__errmsg.str()); \
  }
#define EXPECT_FALSE(p) \
  if ((p)) { \
    std::ostringstream __errmsg; \
    __errmsg << "Expression true, expected false: " #p << " " << FILEINFO; \
    throw std::runtime_error("Expression true, expected false: " #p); \
  }
#define EXPECT_EQ(p, x) \
  { \
    auto v = (p); \
    auto z = (x); \
    if (!(v == z)) { \
      std::ostringstream __errmsg; \
      __errmsg << "Expression '" #p "' == " << v << ", but expected " << z << " " << FILEINFO; \
      throw std::runtime_error(__errmsg.str()); \
    } \
  }
#define EXPECT_NEQ(p, x) \
  { \
    auto v = (p); \
    auto z = (x); \
    if (v == z) { \
      std::ostringstream __errmsg; \
      __errmsg << "Expression '" #p "' == " << v << ", but expected anything else" \
               << " " << FILEINFO; \
      throw std::runtime_error(__errmsg.str()); \
    } \
  }
#define EXPECT_ALMOST_EQ(p, x, eps) \
  { \
    auto v = (p); \
    auto z = (x); \
    auto d = v - z; \
    if (std::abs(d) > (eps)) { \
      std::ostringstream __errmsg; \
      __errmsg << "Expression '" #p "' == " << v << ", but expected " << z << " " << FILEINFO; \
      throw std::runtime_error(__errmsg.str()); \
    } \
  }

#define EXPECT_EXCEPTION(p) \
  { \
    bool __pass = true; \
    try { \
      (p); \
      __pass = false; \
    } catch (...) { \
    } \
    if (!__pass) { \
      std::ostringstream __errmsg; \
      __errmsg << "Expression '" #p "' ran successfully but an std::exception was expected " \
               << FILEINFO; \
      throw std::runtime_error(__errmsg.str()); \
    } \
  }

#define EXPECT_EXCEPTION_MSG(p, expect_msg) \
  { \
    bool pass = false; \
    try { \
      (p); \
    } catch (const std::exception& e) { \
      const std::string actual_msg = e.what(); \
      if (actual_msg.find(expect_msg) != std::string::npos) { \
        pass = true; \
      } else { \
        std::ostringstream errmsg; \
        errmsg << "Expression '" #p "' expects a std::exception "; \
        errmsg << "which contains string '" #expect_msg "', but got message '" << actual_msg \
               << "' instead " << FILEINFO; \
        throw std::runtime_error(errmsg.str()); \
      } \
    } \
    if (!pass) { \
      std::ostringstream errmsg; \
      errmsg << "Expression '" #p "' ran successfully but an std::exception was expected " \
             << FILEINFO; \
      throw std::runtime_error(errmsg.str()); \
    } \
  }

#endif
