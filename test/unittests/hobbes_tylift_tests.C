#include <hobbes/lang/tylift.H>

#include "../test.H"

TEST(UnitTests, TyLiftSeq) {
  const hobbes::seq<int> empty;
  EXPECT_TRUE(empty.empty());
  EXPECT_TRUE(empty.head() == nullptr);

  hobbes::seq<int> one(42, nullptr);
  EXPECT_FALSE(one.empty());
  EXPECT_EQ(one.head()->first, 42)
  EXPECT_TRUE(one.head()->second == nullptr);

  const hobbes::seq<int> two(73, &one);
  EXPECT_FALSE(two.empty());
  EXPECT_EQ(two.head()->first, 73)
  EXPECT_TRUE(two.head()->second == &one);
  EXPECT_FALSE(one.empty());
  EXPECT_EQ(one.head()->first, 42)
  EXPECT_TRUE(one.head()->second == nullptr);
}

#ifdef __GNUC__
// #ifndef __clang__
int demangle_global_int = 0;
int* demangle_global_int_ptr = &demangle_global_int;
int* const demangle_global_int_ptr_const = &demangle_global_int;
int& demangle_global_int_ref = demangle_global_int;
const int demangle_global_const_int = 0;
const int* demangle_global_const_ptr = &demangle_global_const_int;
const int * const demangle_global_const_ptr_const = &demangle_global_const_int;
const int& demangle_global_const_int_ptr = demangle_global_const_int;

static int demangle_file_scope_static_int = 0;
static int* demangle_file_scope_static_int_ptr = &demangle_file_scope_static_int;
static int& demangle_file_scope_static_int_ref = demangle_file_scope_static_int;
static const int demangle_file_scope_static_const_int = 0;
static const int* demangle_file_scope_static_const_int_ptr = &demangle_file_scope_static_const_int;
static const int& demangle_file_scope_static_const_int_ref = demangle_file_scope_static_const_int;
namespace {
int demangle_anon_ns_int = 0;
int* demangle_anon_ns_int_ptr = &demangle_anon_ns_int;
int& demangle_anon_ns_int_ref = demangle_anon_ns_int;
const int demangle_anon_ns_const_int = 0;
const int* demangle_anon_ns_const_int_ptr = &demangle_anon_ns_const_int;
const int& demangle_anon_ns_const_int_ref = demangle_anon_ns_const_int;

template <typename T> std::string demangle(T&&) {
  const auto dmn = std::unique_ptr<char[], void (*)(void*)>(
      abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr), ::free);
  if (dmn) {
    return {dmn.get()};
  }
  return {};
}
} // namespace

namespace demangle_ns {
int demangle_named_ns_int = 0;
}

TEST(UnitTests, TyLiftDemangle) {
  EXPECT_EQ(demangle(demangle_global_int), "int");
  EXPECT_EQ(demangle(demangle_global_int_ptr), "int*");
  EXPECT_EQ(demangle(demangle_global_int_ptr_const), "int*");
  EXPECT_EQ(demangle(demangle_global_int_ref), "int");
  EXPECT_EQ(demangle(demangle_global_const_int), "int");
  EXPECT_EQ(demangle(demangle_global_const_ptr), "int const*");
  EXPECT_EQ(demangle(demangle_global_const_ptr_const), "int const*");
  EXPECT_EQ(demangle(demangle_global_const_int_ptr), "int");

  EXPECT_EQ(demangle(demangle_file_scope_static_int), "int");
  EXPECT_EQ(demangle(demangle_file_scope_static_int_ptr), "int*");
  EXPECT_EQ(demangle(demangle_file_scope_static_int_ref), "int");
  EXPECT_EQ(demangle(demangle_file_scope_static_const_int), "int");
  EXPECT_EQ(demangle(demangle_file_scope_static_const_int_ptr), "int const*");
  EXPECT_EQ(demangle(demangle_file_scope_static_const_int_ref), "int");

  EXPECT_EQ(demangle(demangle_anon_ns_int), "int");
  EXPECT_EQ(demangle(demangle_anon_ns_int_ptr), "int*");
  EXPECT_EQ(demangle(demangle_anon_ns_int_ref), "int");
  EXPECT_EQ(demangle(demangle_anon_ns_const_int), "int");
  EXPECT_EQ(demangle(demangle_anon_ns_const_int_ptr), "int const*");
  EXPECT_EQ(demangle(demangle_anon_ns_const_int_ref), "int");

  EXPECT_EQ(demangle(demangle_ns::demangle_named_ns_int), "int");
}
// #endif
#endif
