#include "../fraction.h"

#include "gtest/gtest.h"

using namespace testing;

TEST(FractionTest, Fractions) {
  Fraction a(4, 6);
  EXPECT_EQ(a.n, 2);
  EXPECT_EQ(a.d, 3);
  EXPECT_EQ(a.to_string(), "2/3");

  EXPECT_LT(a, Fraction::infinity());
  EXPECT_GT(a, Fraction::epsilon());

  EXPECT_LT(Fraction(10000, 1), Fraction::infinity());
  EXPECT_GT(Fraction(1, 10000), Fraction::epsilon());

  Fraction b(3, 5);
  EXPECT_EQ(b.to_string(), "3/5");
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
  EXPECT_EQ(f.to_string(), "1/1");

  f = Fraction(2, 3) + Fraction(3, 2);
  EXPECT_EQ(f.n, 13);
  EXPECT_EQ(f.d, 6);
  EXPECT_EQ(f.to_string(), "13/6");
}