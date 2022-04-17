#include <gtest/gtest.h>

#include "../spaceball.h"

TEST(SpaceballTests, Version) {
    Spaceball sb{};
    EXPECT_EQ(sb.getVersion(), 1);
}
