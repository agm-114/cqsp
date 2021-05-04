/*
 * Copyright 2021 Conquer Space
*/
#include <gtest/gtest.h>

#include "common/components/orbit.h"

// Tests for input from client options
TEST(Common_OrbitTest, toVec2Test) {
    namespace cqspb = conquerspace::components::bodies;
    cqspb::Orbit orbit1;
    orbit1.semiMajorAxis = 100;
    orbit1.eccentricity = 0;
    orbit1.argument = 0;
    orbit1.theta = 0;
    cqspb::Vec2 vec1 = cqspb::toVec2(orbit1);

    orbit1.theta = 360;
    cqspb::Vec2 vec2 = cqspb::toVec2(orbit1);
    EXPECT_EQ(vec1.x, vec2.x);
    EXPECT_EQ(vec1.y, vec2.y);

    orbit1.semiMajorAxis = 120;
    orbit1.eccentricity = 0.5;
    orbit1.argument = 50;
    orbit1.theta = 0;
    vec1 = cqspb::toVec2(orbit1);

    orbit1.theta = 360;
    vec2 = cqspb::toVec2(orbit1);
    EXPECT_EQ(vec1.x, vec2.x);
    EXPECT_EQ(vec1.y, vec2.y);

    orbit1.theta = 50;
    vec1 = cqspb::toVec2(orbit1);

    orbit1.theta = 360 + 50;
    vec2 = cqspb::toVec2(orbit1);
    EXPECT_EQ(vec1.x, vec2.x);
    EXPECT_EQ(vec1.y, vec2.y);

    orbit1.theta = 10;
    vec1 = cqspb::toVec2(orbit1);

    orbit1.theta = 360 + 10;
    vec2 = cqspb::toVec2(orbit1);
    EXPECT_EQ(vec1.x, vec2.x);
    EXPECT_EQ(vec1.y, vec2.y);
}

TEST(Common_OrbitTest, ToRadianTest) {
    namespace cqspb = conquerspace::components::bodies;
    namespace boostc = boost::math::constants;
    EXPECT_DOUBLE_EQ(boostc::pi<double>()/2, cqspb::toRadian(90));
    EXPECT_DOUBLE_EQ(boostc::pi<double>(), cqspb::toRadian(180));
    EXPECT_DOUBLE_EQ(boostc::pi<double>() * 2.f, cqspb::toRadian(360));
    EXPECT_DOUBLE_EQ(boostc::pi<double>()/6, cqspb::toRadian(30));
    EXPECT_DOUBLE_EQ(boostc::pi<double>()/3, cqspb::toRadian(60));
    EXPECT_DOUBLE_EQ(boostc::pi<double>()/4, cqspb::toRadian(45));
}

TEST(Common_OrbitTest, ToDegreeTest) {
    namespace cqspb = conquerspace::components::bodies;
    namespace boostc = boost::math::constants;
    EXPECT_DOUBLE_EQ(30, cqspb::toDegree(boostc::pi<double>()/6));
    EXPECT_DOUBLE_EQ(45, cqspb::toDegree(boostc::pi<double>()/4));
    EXPECT_DOUBLE_EQ(60, cqspb::toDegree(boostc::pi<double>()/3));
    EXPECT_DOUBLE_EQ(90, cqspb::toDegree(boostc::pi<double>()/2));
    EXPECT_DOUBLE_EQ(180, cqspb::toDegree(boostc::pi<double>()));
    EXPECT_DOUBLE_EQ(360, cqspb::toDegree(boostc::pi<double>()*2));
}