#include <gtest/gtest.h>

#include "../spaceball.h"

TEST(SpaceballTests, Version) {
    Spaceball sb{"/dev/null"};
    EXPECT_EQ(sb.getVersion(), 1);
}
