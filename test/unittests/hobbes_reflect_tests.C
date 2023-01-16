#include <hobbes/reflect.H>

#include "../test.H"
#include "hobbes/util/codec.H"

namespace {
constexpr char toNumChar(std::size_t i) noexcept {
  assert(i < 10);
  return "0123456789"[i];
}

constexpr std::size_t N = 10;
constexpr char s[N] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

// at
namespace at {
// within boundary returns char
static_assert(hobbes::at(0, s) == toNumChar(0), "");
static_assert(hobbes::at(1, s) == toNumChar(1), "");
static_assert(hobbes::at(2, s) == toNumChar(2), "");
static_assert(hobbes::at(3, s) == toNumChar(3), "");
static_assert(hobbes::at(4, s) == toNumChar(4), "");
static_assert(hobbes::at(5, s) == toNumChar(5), "");
static_assert(hobbes::at(6, s) == toNumChar(6), "");
static_assert(hobbes::at(7, s) == toNumChar(7), "");
static_assert(hobbes::at(8, s) == toNumChar(8), "");
static_assert(hobbes::at(9, s) == toNumChar(9), "");
// outside boundary returns \0
static_assert(hobbes::at(N + 1, s) == '\0', "");
} // namespace at
} // namespace

// at8S
TEST(UnitTests, ReflectAt8S) {
  using PackedType = decltype(hobbes::at8S(0, 0, s));
  static_assert(std::is_same<PackedType, size_t>::value, "hobbes::at8S currently returns size_t");
  static_assert(sizeof(PackedType) == 8 * sizeof(char),
                "hobbes::at8S works on the assumption that size_t is 8 bytes long");

  PackedType r{};
  // packing the first 8 bytes into PackedType
  ::memcpy(&r, s, 8);
  EXPECT_EQ(hobbes::at8(0, s), r);

  r = {};
  // packing the remaining bytes into PackedType
  ::memcpy(&r, s + 8, N - 8);
  EXPECT_EQ(hobbes::at8(8, s), r);

  // PRIV_HPPF_TSTR stores compile-time char array to a size_t array
  // "0123456789" -> size_t[] {'76543210', 000000'98'};
  // its ::str() cast this array  to const char*, which makes it "0123456789" again
  EXPECT_EQ(::strcmp(PRIV_HPPF_TSTR(s)::str(), "0123456789"), 0);
}

namespace {
namespace unit {
static_assert(hobbes::unit{} == hobbes::unit{}, "");
static_assert(!(hobbes::unit{} < hobbes::unit{}), "");
} // namespace unit

namespace tuple_tail {
static_assert(std::is_same<hobbes::tupleTail<int, double>::type, hobbes::tuple<double>>::value, "");
static_assert(
    std::is_same<hobbes::tupleTail<int, double, int>::type, hobbes::tuple<double, int>>::value, "");
} // namespace tuple_tail
} // namespace

TEST(UnitTests, ReflectUnit) {
  std::ostringstream oss;
  oss << hobbes::unit{};
  EXPECT_EQ(oss.str(), "()");
}

TEST(UnitTests, ReflectTy) {
  {
    const auto a = std::static_pointer_cast<const hobbes::ty::Nat>(hobbes::ty::nat(42UL));
    EXPECT_EQ(a->tid, PRIV_HPPF_TYCTOR_SIZE);
    EXPECT_EQ(a->x, 42UL);
  }

  {
    const auto a = std::static_pointer_cast<const hobbes::ty::Prim>(hobbes::ty::prim("unit"));
    EXPECT_EQ(a->tid, PRIV_HPPF_TYCTOR_PRIM);
    EXPECT_EQ(std::static_pointer_cast<const hobbes::ty::Prim>(a)->n, "unit");
    EXPECT_FALSE(a->rep);
  }

  {
    const auto a = std::static_pointer_cast<const hobbes::ty::Var>(hobbes::ty::var("n"));
    EXPECT_EQ(a->tid, PRIV_HPPF_TYCTOR_TVAR);
    EXPECT_EQ(a->n, "n");
  }

  {
    const auto a = hobbes::ty::array(hobbes::ty::prim("int"), hobbes::ty::nat(42UL));
    EXPECT_EQ(a->tid, PRIV_HPPF_TYCTOR_FIXEDARR);

    const auto t = std::static_pointer_cast<const hobbes::ty::Prim>(
        std::static_pointer_cast<const hobbes::ty::FArr>(a)->t);
    EXPECT_EQ(t->tid, PRIV_HPPF_TYCTOR_PRIM);
    EXPECT_EQ(t->n, "int");
    EXPECT_FALSE(t->rep);

    const auto len = std::static_pointer_cast<const hobbes::ty::Nat>(
        std::static_pointer_cast<const hobbes::ty::FArr>(a)->len);
    EXPECT_EQ(len->tid, PRIV_HPPF_TYCTOR_SIZE);
    EXPECT_EQ(len->x, 42UL);
  }

  {
    const auto a = hobbes::ty::array(hobbes::ty::prim("int"));
    EXPECT_EQ(a->tid, PRIV_HPPF_TYCTOR_ARR);

    const auto t = std::static_pointer_cast<const hobbes::ty::Prim>(
        std::static_pointer_cast<const hobbes::ty::Arr>(a)->t);
    EXPECT_EQ(t->tid, PRIV_HPPF_TYCTOR_PRIM);
    EXPECT_EQ(t->n, "int");
    EXPECT_FALSE(t->rep);
  }
}

TEST(UnitTests, ReflectEncode) {
  using TID = decltype(PRIV_HPPF_TYCTOR_SIZE);

  std::size_t i{};

  hobbes::ty::bytes out;
  const auto a = std::static_pointer_cast<hobbes::ty::Nat>(hobbes::ty::nat(42UL));
  hobbes::ty::encode(a, &out);
  EXPECT_EQ(out.size(), i + sizeof(PRIV_HPPF_TYCTOR_SIZE) + sizeof(a->x)); 

  // tid
  TID tid{};
  ::memcpy(&tid, out.data() + i, sizeof(TID));
  EXPECT_EQ(tid, PRIV_HPPF_TYCTOR_SIZE);

  // size_t
  decltype(a->x) x{};
  ::memcpy(&x, out.data() + sizeof(TID) + i, sizeof(a->x));
  EXPECT_EQ(x, 42UL);
  i += sizeof(a->x);
}
