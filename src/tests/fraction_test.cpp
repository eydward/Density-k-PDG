#include "../fraction.h"

#include "gtest/gtest.h"

using namespace testing;

TEST(FractionTest, Fractions) {
  Fraction a(4, 6);
  EXPECT_EQ(a.n, 2);
  EXPECT_EQ(a.d, 3);

  Fraction b(3, 5);
  EXPECT_TRUE(a > b);
  EXPECT_TRUE(a >= b);
  EXPECT_TRUE(b < a);
  EXPECT_TRUE(b <= a);
  EXPECT_TRUE(b != a);
  EXPECT_FALSE(b == a);
}

TEST(FractionTest, Add) {
  Fraction f = Fraction(2, 3) + Fraction(1, 3);
  EXPECT_EQ(f.n, 1);
  EXPECT_EQ(f.d, 1);

  f = Fraction(2, 3) + Fraction(3, 2);
  EXPECT_EQ(f.n, 13);
  EXPECT_EQ(f.d, 6);
}