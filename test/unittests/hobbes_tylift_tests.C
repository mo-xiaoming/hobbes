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
