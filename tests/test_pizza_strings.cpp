/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** pizza string helpers
*/

#include <gtest/gtest.h>

#include "plazza/pizza/PizzaSize.hpp"
#include "plazza/pizza/PizzaType.hpp"

using plazza::pizza::PizzaSize;
using plazza::pizza::PizzaType;
using plazza::pizza::sizeFromString;
using plazza::pizza::typeFromString;

TEST(TypeFromString, KnownLowercase)
{
    EXPECT_EQ(typeFromString("regina"), PizzaType::Regina);
    EXPECT_EQ(typeFromString("margarita"), PizzaType::Margarita);
    EXPECT_EQ(typeFromString("americana"), PizzaType::Americana);
    EXPECT_EQ(typeFromString("fantasia"), PizzaType::Fantasia);
}

TEST(TypeFromString, KnownMixedCase)
{
    EXPECT_EQ(typeFromString("REGINA"), PizzaType::Regina);
    EXPECT_EQ(typeFromString("Regina"), PizzaType::Regina);
    EXPECT_EQ(typeFromString("ReGiNa"), PizzaType::Regina);
}

TEST(TypeFromString, Unknown)
{
    EXPECT_EQ(typeFromString("hawaiian"), std::nullopt);
    EXPECT_EQ(typeFromString(""), std::nullopt);
    EXPECT_EQ(typeFromString("regina!"), std::nullopt);
}

TEST(SizeFromString, KnownUppercase)
{
    EXPECT_EQ(sizeFromString("S"), PizzaSize::S);
    EXPECT_EQ(sizeFromString("M"), PizzaSize::M);
    EXPECT_EQ(sizeFromString("L"), PizzaSize::L);
    EXPECT_EQ(sizeFromString("XL"), PizzaSize::XL);
    EXPECT_EQ(sizeFromString("XXL"), PizzaSize::XXL);
}

TEST(SizeFromString, RejectsLowercase)
{
    EXPECT_EQ(sizeFromString("s"), std::nullopt);
    EXPECT_EQ(sizeFromString("xxl"), std::nullopt);
}

TEST(SizeFromString, Unknown)
{
    EXPECT_EQ(sizeFromString(""), std::nullopt);
    EXPECT_EQ(sizeFromString("XXXL"), std::nullopt);
    EXPECT_EQ(sizeFromString("XS"), std::nullopt);
}
