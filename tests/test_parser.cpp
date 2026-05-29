/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** parser
*/

#include <gtest/gtest.h>

#include "plazza/reception/Order.hpp"
#include "plazza/reception/Parser.hpp"

using plazza::pizza::PizzaSize;
using plazza::pizza::PizzaType;
using plazza::reception::Order;
using plazza::reception::Parser;

TEST(Parser, SingleOrder)
{
    auto result = Parser::parse("regina XXL x2");
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1u);
    EXPECT_EQ((*result)[0].type, PizzaType::Regina);
    EXPECT_EQ((*result)[0].size, PizzaSize::XXL);
    EXPECT_EQ((*result)[0].count, 2u);
}

TEST(Parser, MultipleOrders)
{
    auto result = Parser::parse("regina XXL x2; fantasia M x3; margarita S x1");
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 3u);
    EXPECT_EQ((*result)[1].type, PizzaType::Fantasia);
    EXPECT_EQ((*result)[1].size, PizzaSize::M);
    EXPECT_EQ((*result)[1].count, 3u);
    EXPECT_EQ((*result)[2].type, PizzaType::Margarita);
    EXPECT_EQ((*result)[2].count, 1u);
}

TEST(Parser, LargeMultiDigitCount)
{
    auto result = Parser::parse("americana L x42");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*result)[0].count, 42u);
}

TEST(Parser, WhitespaceTolerance)
{
    auto result = Parser::parse("   regina   XXL   x2  ;   fantasia M x3   ");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->size(), 2u);
}

TEST(Parser, EmptyRejected) { EXPECT_FALSE(Parser::parse("").has_value()); }

TEST(Parser, WhitespaceOnlyRejected) { EXPECT_FALSE(Parser::parse("   ").has_value()); }

TEST(Parser, MissingCountRejected) { EXPECT_FALSE(Parser::parse("regina XXL").has_value()); }

TEST(Parser, MissingSizeRejected) { EXPECT_FALSE(Parser::parse("regina x2").has_value()); }

TEST(Parser, UnknownTypeRejected) { EXPECT_FALSE(Parser::parse("hawaiian XXL x2").has_value()); }

TEST(Parser, UnknownSizeRejected) { EXPECT_FALSE(Parser::parse("regina XXXL x2").has_value()); }

TEST(Parser, LowercaseSizeRejected) { EXPECT_FALSE(Parser::parse("regina xxl x2").has_value()); }

TEST(Parser, CountMissingXRejected) { EXPECT_FALSE(Parser::parse("regina XXL 2").has_value()); }

TEST(Parser, CountZeroRejected) { EXPECT_FALSE(Parser::parse("regina XXL x0").has_value()); }

TEST(Parser, CountLeadingZeroRejected)
{
    EXPECT_FALSE(Parser::parse("regina XXL x01").has_value());
    EXPECT_FALSE(Parser::parse("regina XXL x007").has_value());
}

TEST(Parser, CountWithLetterRejected) { EXPECT_FALSE(Parser::parse("regina XXL x2a").has_value()); }

TEST(Parser, TrailingSemicolonRejected)
{
    EXPECT_FALSE(Parser::parse("regina XXL x2;").has_value());
}

TEST(Parser, DoubleSemicolonRejected)
{
    EXPECT_FALSE(Parser::parse("regina XXL x2;; fantasia M x3").has_value());
}

TEST(Parser, ExtraTokensRejected)
{
    EXPECT_FALSE(Parser::parse("regina XXL x2 extra").has_value());
}

TEST(Parser, LeadingSemicolonRejected)
{
    EXPECT_FALSE(Parser::parse("; regina XXL x2").has_value());
}
