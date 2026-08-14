/* Copyright (C) 2021 Peter G. Jensen <root@petergjoel.dk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define BOOST_TEST_MODULE string_query_test

#include "DummyBuilder.h"
#include "Colored/ColoredPetriNetBuilder.h"
#include "unfoldtacpn.h"
#include "PQL/Expressions.h"
#include "PQL/Contexts.h"
#include "PQL/Visitor.h"
#include "PetriParse/QueryXMLParser.h"

#include <boost/test/unit_test.hpp>
#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>

using namespace unfoldtacpn;

BOOST_AUTO_TEST_CASE(DirectoryTest) {
    BOOST_REQUIRE(getenv("TEST_FILES"));
}

std::ifstream loadTestFile(const std::string& file) {
    std::stringstream ss;
    ss << getenv("TEST_FILES") << "/" << file;
    std::cerr << "Loading '" << ss.str() << "'" << std::endl;
    return std::ifstream(ss.str());
}

std::ifstream loadFile(const char* file) {
    return loadTestFile(std::string("xml_query/") + file);
}

BOOST_AUTO_TEST_CASE(ReferendumQuery) {
    auto f = loadFile("referendum_query.xml");
    BOOST_REQUIRE(f);
    QueryXMLParser parser;
    std::set<size_t> to_read{0};
    parser.parse(f, to_read);
    BOOST_REQUIRE_EQUAL(parser.queries.size(), 2);
    BOOST_REQUIRE_EQUAL(parser.queries[0].parsingResult, QueryItem::PARSING_OK);
    BOOST_REQUIRE(parser.queries[0].query != nullptr);
    BOOST_REQUIRE_EQUAL(parser.queries[1].parsingResult, QueryItem::PARSING_OK);
    BOOST_REQUIRE(parser.queries[1].query == nullptr); // not parsed,
}

BOOST_AUTO_TEST_CASE(GameQuery) {
    auto f = loadFile("game_queries.xml");
    BOOST_REQUIRE(f);
    QueryXMLParser parser;
    std::set<size_t> to_read{0, 1};
    parser.parse(f, to_read);
    BOOST_REQUIRE_EQUAL(parser.queries.size(), 2);
    BOOST_REQUIRE_EQUAL(parser.queries[0].parsingResult, QueryItem::PARSING_OK);
    BOOST_REQUIRE(parser.queries[0].query != nullptr);
    BOOST_REQUIRE_EQUAL(parser.queries[1].parsingResult, QueryItem::PARSING_OK);
    BOOST_REQUIRE(parser.queries[1].query != nullptr);
}

using namespace PQL;
class DummyVisitor : public PQL::Visitor {
public:
    std::vector<std::string> identifiers;
protected:

    virtual void _accept(const NotCondition* element) { (*element)[0]->visit(*this); };
    virtual void _accept(const AndCondition* element) { for(auto& e : *element) e->visit(*this); };
    virtual void _accept(const OrCondition* element) { for(auto& e : *element) e->visit(*this); };
    virtual void _accept(const LessThanCondition* element) { for(auto i : {0,1}) (*element)[i]->visit(*this); };
    virtual void _accept(const LessThanOrEqualCondition* element) { for(auto i : {0,1}) (*element)[i]->visit(*this); };
    virtual void _accept(const EqualCondition* element) { for(auto i : {0,1}) (*element)[i]->visit(*this); };
    virtual void _accept(const NotEqualCondition* element) { for(auto i : {0,1}) (*element)[i]->visit(*this); };
    virtual void _accept(const DeadlockCondition* element) { };

    // Quantifiers, most uses of the visitor will not use the quantifiers - so we give a default implementation.
    // default behaviour is error

    virtual void _accept(const EFCondition* element) {
        (*element)[0]->visit(*this);
    };

    virtual void _accept(const EGCondition* element) {
        (*element)[0]->visit(*this);
    };

    virtual void _accept(const AGCondition* element) {
        (*element)[0]->visit(*this);
    };

    virtual void _accept(const AFCondition* element) {
        (*element)[0]->visit(*this);
    };

    virtual void _accept(const EXCondition* element) {
        (*element)[0]->visit(*this);
    };

    virtual void _accept(const AXCondition* element) {
        (*element)[0]->visit(*this);
    };

    virtual void _accept(const EUCondition* element) {
        (*element)[0]->visit(*this);
        (*element)[1]->visit(*this);
    };

    virtual void _accept(const AUCondition*element) {
        (*element)[0]->visit(*this);
        (*element)[1]->visit(*this);
    };

    // shallow elements, neither of these should exist in a compiled expression

    virtual void _accept(const KSafeCondition* element) {
    };

    virtual void _accept(const BooleanCondition* element) {
    };

    // Expression
    virtual void _accept(const UnfoldedIdentifierExpr* element)
    {
        identifiers.emplace_back(element->name());
    }

    virtual void _accept(const LiteralIntExpr* element) {

    }

    virtual void _accept(const LiteralRealExpr* element) {
        
    }

    virtual void _accept(const PlusExpr* element)
    {
        for(auto& e : (*element))
            e->visit(*this);
    }

    virtual void _accept(const MultiplyExpr* element)
    {
        for(auto& e : (*element))
            e->visit(*this);
    }

    virtual void _accept(const MinusExpr* element)
    {

        (*element)[0]->visit(*this);
    }

    virtual void _accept(const SubtractExpr* element) {
        for(auto& e : (*element))
            e->visit(*this);
    }

    virtual void _accept(const IdentifierExpr* element) {

    };
};

BOOST_AUTO_TEST_CASE(ColorExpressionQuery) {
    auto pfile = loadFile("philosophers.pnml");
    BOOST_REQUIRE(pfile);

    ColoredPetriNetBuilder builder;
    builder.parseNet(pfile);

    DummyBuilder dummyBuilder;
    builder.unfold(dummyBuilder);

    NamingContext context(builder.getUnfoldedPlaceNames(),
                          builder.getUnfoldedTransitionNames(),
                          &builder);

    auto analyze = [&context](const char* file) {
        auto qfile = loadFile(file);
        BOOST_REQUIRE(qfile);
        QueryXMLParser parser;
        BOOST_REQUIRE(parser.parse(qfile, {0}));
        BOOST_REQUIRE_EQUAL(parser.queries.size(), 1);
        BOOST_REQUIRE_EQUAL(parser.queries[0].parsingResult, QueryItem::PARSING_OK);
        BOOST_REQUIRE(parser.queries[0].query);
        parser.queries[0].query->analyze(context);

        DummyVisitor visitor;
        parser.queries[0].query->visit(visitor);
        std::sort(visitor.identifiers.begin(), visitor.identifiers.end());
        return visitor.identifiers;
    };

    const std::vector<std::string> single{
        "Philosophers_dash_COL_dash_000005__think__2"
    };
    auto actual = analyze("philosophers_query.xml");
    BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(), single.begin(), single.end());

    std::vector<std::string> expected{
        "Philosophers_dash_COL_dash_000005__catch1__2",
        "Philosophers_dash_COL_dash_000005__catch2__0",
        "Philosophers_dash_COL_dash_000005__catch2__4",
        "Philosophers_dash_COL_dash_000005__eat__0",
        "Philosophers_dash_COL_dash_000005__eat__1",
        "Philosophers_dash_COL_dash_000005__eat__2",
        "Philosophers_dash_COL_dash_000005__eat__3",
        "Philosophers_dash_COL_dash_000005__eat__4",
        "Philosophers_dash_COL_dash_000005__think__1"
    };
    std::sort(expected.begin(), expected.end());
    actual = analyze("philosophers_color_query.xml");
    BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(), expected.begin(), expected.end());

    std::string unfolded;
    BOOST_CHECK(!builder.resolvePlace("Philosophers_dash_COL_dash_000005__think", "4294967296", unfolded));
}

BOOST_AUTO_TEST_CASE(TupleColorExpressionQuery) {
    auto pfile = loadTestFile("cpn_format/product.xml");
    BOOST_REQUIRE(pfile);

    ColoredPetriNetBuilder builder;
    builder.parseNet(pfile);
    DummyBuilder dummyBuilder;
    builder.unfold(dummyBuilder);

    auto query = loadFile("product_query.xml");
    BOOST_REQUIRE(query);

    QueryXMLParser parser;
    BOOST_REQUIRE(parser.parse(query, {0}));
    BOOST_REQUIRE_EQUAL(parser.queries.size(), 1);
    BOOST_REQUIRE(parser.queries[0].query);

    NamingContext context(builder.getUnfoldedPlaceNames(),
                          builder.getUnfoldedTransitionNames(),
                          &builder);
    parser.queries[0].query->analyze(context);

    DummyVisitor visitor;
    parser.queries[0].query->visit(visitor);
    BOOST_REQUIRE_EQUAL(visitor.identifiers.size(), 1);
    BOOST_CHECK_EQUAL(visitor.identifiers[0], "PRODUCT__2");
}

BOOST_AUTO_TEST_CASE(TokenRingColorExpressionQuery) {
    auto pfile = loadFile("token_ring.pnml");
    BOOST_REQUIRE(pfile);

    ColoredPetriNetBuilder builder;
    builder.parseNet(pfile);
    DummyBuilder dummyBuilder;
    builder.unfold(dummyBuilder);

    auto query = loadFile("token_ring_color_query.xml");
    BOOST_REQUIRE(query);

    QueryXMLParser parser;
    BOOST_REQUIRE(parser.parse(query, {0}));
    BOOST_REQUIRE_EQUAL(parser.queries.size(), 1);
    BOOST_REQUIRE_EQUAL(parser.queries[0].parsingResult, QueryItem::PARSING_OK);
    BOOST_REQUIRE(parser.queries[0].query);

    NamingContext context(builder.getUnfoldedPlaceNames(),
                          builder.getUnfoldedTransitionNames(),
                          &builder);
    parser.queries[0].query->analyze(context);

    DummyVisitor visitor;
    parser.queries[0].query->visit(visitor);
    std::sort(visitor.identifiers.begin(), visitor.identifiers.end());
    BOOST_REQUIRE_EQUAL(visitor.identifiers.size(), 2);
    BOOST_CHECK_EQUAL(visitor.identifiers[0], "TokenRing_dash_COL_dash_005__P0");
    BOOST_CHECK_EQUAL(visitor.identifiers[1], "TokenRing_dash_COL_dash_005__state__20");
}
