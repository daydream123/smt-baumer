#ifndef TST_BAUMERCAMERA_H
#define TST_BAUMERCAMERA_H

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <CameraWrapper.h>

using namespace testing;

TEST(Lib_bcameraTest, BaumerCamera)
{
    EXPECT_EQ(1, 1);
    ASSERT_THAT(0, Eq(0));
}

#endif // TST_BAUMERCAMERA_H
