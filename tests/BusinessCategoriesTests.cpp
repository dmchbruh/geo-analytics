#include <catch2/catch_test_macros.hpp>

#include "config/BusinessCategories.h"

TEST_CASE("findBusinessCategory finds coffee_shop")
{
    auto category = findBusinessCategory("coffee_shop");

    REQUIRE(category.has_value());
    REQUIRE(category->competitorTags.size() == 1);
}

TEST_CASE("findBusinessCategory finds barbershop")
{
    auto category = findBusinessCategory("barbershop");

    REQUIRE(category.has_value());
    REQUIRE(category->competitorTags.size() == 2);
}

TEST_CASE("findBusinessCategory returns nullopt for unknown category")
{
    auto category = findBusinessCategory("nonexistent");

    REQUIRE_FALSE(category.has_value());
}

TEST_CASE("findBusinessCategory includes demand tags")
{
    auto coffee = findBusinessCategory("coffee_shop");
    REQUIRE(coffee->demandTags.size() == 4);

    auto barber = findBusinessCategory("barbershop");
    REQUIRE(barber->demandTags.size() == 3);
}