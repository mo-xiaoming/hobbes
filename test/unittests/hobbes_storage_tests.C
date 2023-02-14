#include <hobbes/storage.H>

#include <utility>

#include "../test.H"

namespace {
// readInt
namespace read_int {
constexpr const char s1[] = "$1";
static_assert(hobbes::storage::internal::readInt(&s1[1], &s1[2]) == 1U, "");
constexpr const char s2[] = "$123";
static_assert(hobbes::storage::internal::readInt(&s2[1], &s2[4]) == 123U, "");
constexpr const char s3[] = " $123";
static_assert(hobbes::storage::internal::readInt(&s3[2], &s3[5]) == 123U, "");
constexpr const char s4[] = " $123 ";
static_assert(hobbes::storage::internal::readInt(&s4[2], &s4[5]) == 123U, "");
constexpr const char s5[] = " $1 $3 ";
static_assert(hobbes::storage::internal::readInt(&s5[5], &s5[6]) == 3U, "");
} // namespace read_int

// maxVarRef
namespace max_var_ref {
static_assert(hobbes::storage::maxVarRef("") == 0U, "");
static_assert(hobbes::storage::maxVarRef("abc") == 0U, "");
static_assert(hobbes::storage::maxVarRef("$0") == 1U, "");
static_assert(hobbes::storage::maxVarRef("$124") == 125U, "");
static_assert(hobbes::storage::maxVarRef("$1$4") == 5U, "");
static_assert(hobbes::storage::maxVarRef("$7$4") == 8U, "");
static_assert(hobbes::storage::maxVarRef("$7 $7") == 8U, "");
static_assert(hobbes::storage::maxVarRef("$7 $9$10") == 11U, "");
} // namespace max_var_ref

// hstore_payload_types
namespace hstore_payload_types {
static_assert(decltype(hobbes::storage::makePayloadTypes())::count == 0, "");
static_assert(decltype(hobbes::storage::makePayloadTypes(3))::count == 1, "");
static_assert(decltype(hobbes::storage::makePayloadTypes(3, "hello", 7.9, 'c'))::count == 4, "");
static_assert(decltype(hobbes::storage::makePayloadTypes(3, 3))::count == 2, "");
static_assert(decltype(hobbes::storage::makePayloadTypes(std::make_tuple(3, 3, 3), 3))::count == 2,
              "");
static_assert(
    std::is_same<decltype(hobbes::storage::makePayloadTypes(4.2, 3))::head_type, double>::value,
    "");
} // namespace hstore_payload_types

// namedValue
namespace named_value {
static_assert(HNAME("hello", 3).value == 3, "");
static_assert(std::is_same<decltype(HNAME(hello, 3)), decltype(HNAME(hello, 7))>::value,
              "two param types defines namedValue type");
} // namespace named_value
} // namespace

TEST(UnitTests, StorageHStorePayloadTypes) {
  using A = hobbes::storage::internal::hstore_argl_name<int>;
  char buf[1024];
  A::nameDesc(".f.hello.%ld", 42, buf, sizeof(buf));
  EXPECT_EQ(std::strcmp(buf, ".f.hello.42"), 0);
  EXPECT_TRUE(A::isTuple);

  using B = hobbes::storage::internal::hstore_argl_name<decltype(HNAME(hello, 3))>;
  B::nameDesc(nullptr, 0U, buf, sizeof(buf));
  EXPECT_EQ(std::strcmp(buf, "hello"), 0);
  EXPECT_FALSE(B::isTuple);
}

TEST(UnitTests, StorageMakeTyDescF) {
  hobbes::storage::bytes b;
  std::string d;
  // unit
  // TODO(mo-xiaoming): haven't figure out an easy way to test bytes data
  hobbes::storage::makeTyDescF<decltype(hobbes::storage::makePayloadTypes())>(&b, &d);
}

namespace {
template <typename T> void testPrimStore(const char* name) {
  static_assert(hobbes::storage::store<T>::can_memcpy, "");
  EXPECT_EQ(hobbes::storage::store<T>::type()->tid, PRIV_HPPF_TYCTOR_PRIM);
  EXPECT_EQ(hobbes::storage::store<T>::size(T{}), sizeof(T));
  EXPECT_EQ(std::static_pointer_cast<hobbes::ty::Prim>(hobbes::storage::store<T>::type())->n, name);
}
} // namespace

TEST(UnitTests, StoragePrimStore) {
  testPrimStore<bool>("bool");
  testPrimStore<uint8_t>("byte");
  testPrimStore<char>("char");
  testPrimStore<int16_t>("short");
  testPrimStore<uint16_t>("short");
  testPrimStore<int32_t>("int");
  testPrimStore<uint32_t>("int");
  testPrimStore<int64_t>("long");
  testPrimStore<uint64_t>("long");
  testPrimStore<__int128>("int128");
  testPrimStore<float>("float");
  testPrimStore<double>("double");

  static_assert(!hobbes::storage::store<hobbes::unit>::can_memcpy, "");
  EXPECT_EQ(hobbes::storage::store<hobbes::unit>::type()->tid, PRIV_HPPF_TYCTOR_PRIM);
  EXPECT_EQ(hobbes::storage::store<hobbes::unit>::size(), 0UL);
  EXPECT_EQ(
      std::static_pointer_cast<hobbes::ty::Prim>(hobbes::storage::store<hobbes::unit>::type())->n,
      "unit");
}

namespace {
struct WithStorageName {};
} // namespace

namespace hobbes {
namespace storage {
template <> struct store<WithStorageName> {
  static std::string storageName() {
    return "X";
  }
};
} // namespace storage
} // namespace hobbes

TEST(UnitTests, StorageStoreNames) {
  std::vector<std::string> a;
  hobbes::storage::storeNames<WithStorageName>::accum(&a);
  EXPECT_EQ(a.size(), 1UL);
  EXPECT_EQ(a[0], "X");

  a.clear();
  hobbes::storage::storeNames<int>::accum(&a);
  EXPECT_TRUE(a.empty());
}