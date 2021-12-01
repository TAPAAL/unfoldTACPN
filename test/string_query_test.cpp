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
#include "PQL/Visitor.h"

#include <boost/test/unit_test.hpp>
#include <string>
#include <fstream>
#include <sstream>

using namespace unfoldtacpn;

BOOST_AUTO_TEST_CASE(DirectoryTest) {
    BOOST_REQUIRE(getenv("TEST_FILES"));
}

std::ifstream loadFile(const char* file) {
    std::stringstream ss;
    ss << getenv("TEST_FILES") << "/string_query/" << file;
    std::cerr << "Loading '" << ss.str() << "'" << std::endl;
    return std::ifstream(ss.str());
}

BOOST_AUTO_TEST_CASE(SimpleQuery) {
    auto f = loadFile("product.xml");
    BOOST_REQUIRE(f);
    ColoredPetriNetBuilder b;
    b.parseNet(f);
    DummyBuilder p;
    b.unfold(p);

    for (std::string s :{"EF true", "EG true", "AG true", "AF true"}) {
        std::stringstream ss(s);
        auto res = unfoldtacpn::parse_string_queries(b, ss);
        BOOST_REQUIRE_EQUAL(res.size(), 1);
        if (s.find("EF") == 0)
            BOOST_REQUIRE(std::dynamic_pointer_cast<PQL::EFCondition>(res[0]) != nullptr);
        if (s.find("EG") == 0)
            BOOST_REQUIRE(std::dynamic_pointer_cast<PQL::EGCondition>(res[0]) != nullptr);
        if (s.find("AF") == 0)
            BOOST_REQUIRE(std::dynamic_pointer_cast<PQL::AFCondition>(res[0]) != nullptr);
        if (s.find("AG") == 0)
            BOOST_REQUIRE(std::dynamic_pointer_cast<PQL::AGCondition>(res[0]) != nullptr);
    }
}
using namespace PQL;
class DummyVisitor : public PQL::Visitor {
public:

    DummyVisitor() {
    }
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

    }

    virtual void _accept(const LiteralExpr* element)
    {

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

class PVisitor : public DummyVisitor {
    public:
        std::set<std::string> _seen;
    virtual void _accept(const UnfoldedIdentifierExpr* element)
    {
        _seen.emplace(element->name());
    }
};

BOOST_AUTO_TEST_CASE(UnfoldTest) {
    auto f = loadFile("product.xml");
    BOOST_REQUIRE(f);
    ColoredPetriNetBuilder b;
    b.parseNet(f);
    DummyBuilder p;
    b.unfold(p);

    for (std::string s :{"EF DOT = 1", "EF SIMPLE = 1", "EF PRODUCT = 1"}) {
        std::stringstream ss(s);
        auto res = unfoldtacpn::parse_string_queries(b, ss);
        BOOST_REQUIRE_EQUAL(res.size(), 1);
        PVisitor v;
        res[0]->visit(v);
        if(s.find("DOT") != std::string::npos)
        {
            BOOST_REQUIRE_EQUAL(v._seen.size(), 1);
            BOOST_REQUIRE_EQUAL(*v._seen.begin(), "DOT");
        }
        if(s.find("SIMPLE") != std::string::npos)
        {
            BOOST_REQUIRE_EQUAL(v._seen.size(), 2);
            for(auto& s : v._seen)
            {
                BOOST_REQUIRE(s.find("SIMPLE") == 0);
            }
        }
        if(s.find("PRODUCT") != std::string::npos)
        {
            BOOST_REQUIRE_EQUAL(v._seen.size(), 4);
            for(auto& s : v._seen)
            {
                BOOST_REQUIRE(s.find("PRODUCT") == 0);
            }
        }
    }
}