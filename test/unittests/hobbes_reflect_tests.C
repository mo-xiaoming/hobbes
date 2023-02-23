#include <hobbes/reflect.H>
#include <hobbes/util/codec.H>

#include "../test.H"

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

namespace {
namespace can_show_type {
static_assert(hobbes::can_show_type<int&>::value, "");
static_assert(hobbes::can_show_type<int>::value, "");
struct A {
  [[maybe_unused]] friend std::ostream& operator<<(std::ostream& os, A) { return os; }
};
static_assert(hobbes::can_show_type<A&>::value, "");
static_assert(hobbes::can_show_type<A>::value, "");
struct B {};
static_assert(!hobbes::can_show_type<B>::value, "");
static_assert(!hobbes::can_show_type<B&>::value, "");

static_assert(hobbes::can_show_types<int, A&>::value, "");
static_assert(!hobbes::can_show_types<A, B>::value, "");
static_assert(!hobbes::can_show_types<B>::value, "");
static_assert(hobbes::can_show_types<double>::value, "");
} // namespace can_show_type

namespace align_to {
// { bool }: bool
static_assert(hobbes::alignTo(0, 1) == 0, "");
// { bool, int }: int
static_assert(hobbes::alignTo(1, 4) == 4, "");
// { bool, int, bool }: 2nd bool
static_assert(hobbes::alignTo(8, 1) == 8, "");
// { int }: int
static_assert(hobbes::alignTo(0, 4) == 0, "");
// { int, bool }: bool
static_assert(hobbes::alignTo(4, 1) == 4, "");
// { int, bool, int }: 2nd int
static_assert(hobbes::alignTo(8, 4) == 8, "");
} // namespace align_to

namespace nth {
static_assert(std::is_same<hobbes::nth<0UL, bool>::type, bool>::value, "");
static_assert(std::is_same<hobbes::nth<0UL, bool, int>::type, bool>::value, "");
static_assert(std::is_same<hobbes::nth<1UL, bool, int>::type, int>::value, "");
} // namespace nth

namespace offset_info {
using Unit = hobbes::offsetInfo<0UL, 0UL>;
static_assert(Unit::offset == 0UL, "");
static_assert(Unit::maxAlignment == 1UL, "");
static_assert(Unit::size == 0UL, "");
static_assert(Unit::packed, "");

using Bool = hobbes::offsetInfo<0UL, 0UL, bool>;
static_assert(Bool::offset == 0UL, "");
static_assert(Bool::offset == hobbes::offsetAt<0UL, Bool>::value, "");
static_assert(Bool::maxAlignment == 1UL, "");
static_assert(Bool::size == 1UL, "");
static_assert(Bool::packed, "");

using BoolInt = hobbes::offsetInfo<0UL, 0UL, bool, int>;
static_assert(BoolInt::offset == 0UL, "");
static_assert(BoolInt::offset == hobbes::offsetAt<0UL, BoolInt>::value, "");
static_assert(4UL == hobbes::offsetAt<1UL, BoolInt>::value, "");
static_assert(BoolInt::maxAlignment == 4UL, "");
static_assert(BoolInt::size == 8UL, "");
static_assert(!BoolInt::packed, "");

using IntBool = hobbes::offsetInfo<0UL, 0UL, int, bool>;
static_assert(IntBool::offset == 0UL, "");
static_assert(IntBool::offset == hobbes::offsetAt<0, IntBool>::value, "");
static_assert(4UL == hobbes::offsetAt<1UL, IntBool>::value, "");
static_assert(IntBool::maxAlignment == 4UL, "");
static_assert(IntBool::size == 5UL, "");
static_assert(IntBool::packed, "");

using Big = hobbes::offsetInfo<0UL, 0UL, char[1000]>;
static_assert(Big::offset == 0UL, "");
static_assert(Big::offset == hobbes::offsetAt<0, Big>::value, "");
static_assert(Big::maxAlignment == 1UL, "");
static_assert(Big::size == 1000UL, "");
static_assert(Big::packed, "");
} // namespace offset_info

namespace tuple {
using BoolTuple = hobbes::tuple<bool>;
static_assert(BoolTuple::alignment == 1UL, "");
static_assert(BoolTuple::size == 1UL, "");
static_assert(BoolTuple::packed, "");
static_assert(std::is_same<hobbes::tupType<0UL, BoolTuple>::type, bool>::value, "");

using IntTuple = hobbes::tuple<int>;
static_assert(IntTuple::alignment == 4UL, "");
static_assert(IntTuple::size == 4UL, "");
static_assert(IntTuple::packed, "");
static_assert(std::is_same<hobbes::tupType<0UL, IntTuple>::type, int>::value, "");

using BoolIntTuple = hobbes::tuple<bool, int>;
static_assert(BoolIntTuple::alignment == 4UL, "");
static_assert(BoolIntTuple::size == 8UL, "");
static_assert(!BoolIntTuple::packed, "");
static_assert(std::is_same<hobbes::tupType<0UL, BoolIntTuple>::type, bool>::value, "");
static_assert(std::is_same<hobbes::tupType<1UL, BoolIntTuple>::type, int>::value, "");

using IntBoolTuple = hobbes::tuple<int, bool>;
static_assert(IntBoolTuple::alignment == 4UL, "");
static_assert(IntBoolTuple::size == 8UL, "");
static_assert(!IntBoolTuple::packed, "");
static_assert(std::is_same<hobbes::tupType<0UL, IntBoolTuple>::type, int>::value, "");
static_assert(std::is_same<hobbes::tupType<1UL, IntBoolTuple>::type, bool>::value, "");

using IntIntTuple = hobbes::tuple<int, int>;
static_assert(IntIntTuple::alignment == 4UL, "");
static_assert(IntIntTuple::size == 8UL, "");
static_assert(IntIntTuple::packed, "");
static_assert(std::is_same<hobbes::tupType<0UL, IntIntTuple>::type, int>::value, "");
static_assert(std::is_same<hobbes::tupType<1UL, IntIntTuple>::type, int>::value, "");

using BoolBoolTuple = hobbes::tuple<bool, bool>;
static_assert(BoolBoolTuple::alignment == 1UL, "");
static_assert(BoolBoolTuple::size == 2UL, "");
static_assert(BoolBoolTuple::packed, "");
static_assert(std::is_same<hobbes::tupType<0UL, BoolBoolTuple>::type, bool>::value, "");
static_assert(std::is_same<hobbes::tupType<1UL, BoolBoolTuple>::type, bool>::value, "");

using BigTuple = hobbes::tuple<char[1000]>;
static_assert(BigTuple::alignment == 1UL, "");
static_assert(BigTuple::size == 1000UL, "");
static_assert(BigTuple::packed, "");
static_assert(std::is_same<hobbes::tupType<0UL, BigTuple>::type, char[1000]>::value, "");

static_assert(std::is_same<hobbes::concatT<hobbes::tuple<>>::type, hobbes::tuple<>>::value, "");
static_assert(std::is_same<hobbes::concatT<hobbes::tuple<int>>::type, hobbes::tuple<int>>::value,
              "");
static_assert(std::is_same<hobbes::concatT<hobbes::tuple<int, double>>::type,
                           hobbes::tuple<int, double>>::value,
              "");
static_assert(std::is_same<hobbes::concatT<hobbes::tuple<int>, hobbes::tuple<double>>::type,
                           hobbes::tuple<int, double>>::value,
              "");
static_assert(
    std::is_same<hobbes::concatT<hobbes::tuple<int>, hobbes::tuple<>, hobbes::tuple<double>>::type,
                 hobbes::tuple<int, double>>::value,
    "");
static_assert(std::is_same<hobbes::concatT<hobbes::tuple<int>, hobbes::tuple<bool>,
                                           hobbes::tuple<double>>::type,
                           hobbes::tuple<int, bool, double>>::value,
              "");
static_assert(std::is_same<hobbes::concatT<hobbes::tuple<int>, hobbes::tuple<bool>,
                                           hobbes::tuple<double>, hobbes::tuple<char>>::type,
                           hobbes::tuple<int, bool, double, char>>::value,
              "");

template <typename T> struct W {};

template <typename T> struct TX {
  using type = W<T>;
};

static_assert(std::is_same<hobbes::fmap<TX, hobbes::tuple<>>::type, hobbes::tuple<>>::value, "");
static_assert(
    std::is_same<hobbes::fmap<TX, hobbes::tuple<int>>::type, hobbes::tuple<W<int>>>::value, "");
static_assert(std::is_same<hobbes::fmap<TX, hobbes::tuple<int, double>>::type,
                           hobbes::tuple<W<int>, W<double>>>::value,
              "");

static_assert(std::is_same<hobbes::tupleTail<int>::type, hobbes::tuple<>>::value, "");
static_assert(std::is_same<hobbes::tupleTail<int, double>::type, hobbes::tuple<double>>::value, "");
static_assert(std::is_same<hobbes::tupleTail<int, double, char>::type, hobbes::tuple<double, char>>::value, "");
} // namespace tuple
} // namespace

TEST(UnitTests, ReflectOffsetInfo) {
  constexpr offset_info::Unit a;
  constexpr offset_info::Unit b;
  EXPECT_TRUE(offset_info::Unit::eq(reinterpret_cast<const uint8_t*>(&a),
                                    reinterpret_cast<const uint8_t*>(&b)));
}

TEST(UnitTests, ReflectTuple) {
  std::ostringstream oss;

  using UnitTuple = hobbes::tuple<>;
  static_assert(UnitTuple::alignment == 1UL, "");
  static_assert(UnitTuple::size == 0UL, "");
  UnitTuple u1;
  UnitTuple u2;
  EXPECT_TRUE(u1 == u2);
#if 0
  oss << u1;
  EXPECT_EQ(oss.str(), "()");
  oss.str("");
#endif

  tuple::BoolIntTuple bi1;
  EXPECT_EQ(bi1.at<0>(), false);
  EXPECT_EQ(bi1.at<1>(), 0);
  oss << bi1;
  EXPECT_EQ(oss.str(), "(0, 0)");
  oss.str("");

  tuple::BoolIntTuple bi2(true, 42);
  EXPECT_EQ(bi2.at<0>(), true);
  EXPECT_EQ(bi2.at<1>(), 42);
  oss << bi2;
  EXPECT_EQ(oss.str(), "(1, 42)");
  oss.str("");

  tuple::BoolIntTuple bi3(bi2);
  EXPECT_EQ(bi3.at<0>(), true);
  EXPECT_EQ(bi3.at<1>(), 42);
  oss << bi2;
  EXPECT_EQ(oss.str(), "(1, 42)");
  oss.str("");

  bi2.at<1>() = 47;
  bi3 = bi2;
  EXPECT_TRUE(bi2 == bi3);
  oss << bi2;
  EXPECT_EQ(oss.str(), "(1, 47)");
  oss.str("");
  oss << bi3;
  EXPECT_EQ(oss.str(), "(1, 47)");
  oss.str("");
}

namespace {
namespace variant {
static_assert(hobbes::TSizeOfF<int>::value == sizeof(int), "");
static_assert(hobbes::TSizeOfF<bool>::value == sizeof(bool), "");

static_assert(hobbes::TAlignOfF<int>::value == sizeof(int), "");
static_assert(hobbes::TAlignOfF<bool>::value == sizeof(bool), "");

static_assert(std::is_same<hobbes::maximum<hobbes::TSizeOfF, int>::type, int>::value, "");
static_assert(std::is_same<hobbes::maximum<hobbes::TSizeOfF, int, bool>::type, int>::value, "");
static_assert(std::is_same<hobbes::maximum<hobbes::TSizeOfF, bool, int>::type, int>::value, "");
static_assert(std::is_same<hobbes::maximum<hobbes::TSizeOfF, int, bool, char>::type, int>::value,
              "");
static_assert(std::is_same<hobbes::maximum<hobbes::TSizeOfF, bool, int, char>::type, int>::value,
              "");
static_assert(std::is_same<hobbes::maximum<hobbes::TSizeOfF, bool, char, int>::type, int>::value,
              "");

static_assert(std::is_same<hobbes::maximum<hobbes::TAlignOfF, int>::type, int>::value, "");
static_assert(std::is_same<hobbes::maximum<hobbes::TAlignOfF, int, bool>::type, int>::value, "");
static_assert(std::is_same<hobbes::maximum<hobbes::TAlignOfF, bool, int>::type, int>::value, "");
static_assert(std::is_same<hobbes::maximum<hobbes::TAlignOfF, int, bool, char>::type, int>::value,
              "");
static_assert(std::is_same<hobbes::maximum<hobbes::TAlignOfF, bool, int, char>::type, int>::value,
              "");
static_assert(std::is_same<hobbes::maximum<hobbes::TAlignOfF, bool, char, int>::type, int>::value,
              "");

static_assert(hobbes::CtorIndexOf<int, int>::value == 0U, "");
static_assert(hobbes::CtorIndexOf<int, int, double>::value == 0U, "");
static_assert(hobbes::CtorIndexOf<int, double, int>::value == 1U, "");

static_assert(std::is_same<hobbes::First<int>::type, int>::value, "");
static_assert(std::is_same<hobbes::First<int, double>::type, int>::value, "");
} // namespace variant
} // namespace

TEST(UnitTests, ReflectString) {
}
