#include "../edge_gen.h"

#include "gtest/gtest.h"

using namespace testing;

TEST(EdgeGeneratorTest, Generate) {
  EdgeGenerator edge_gen;
  edge_gen.initialize(2, 3);
}
