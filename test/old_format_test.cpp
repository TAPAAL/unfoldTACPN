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

#define BOOST_TEST_MODULE old_format_test

#include "DummyBuilder.h"
#include "Colored/ColoredPetriNetBuilder.h"

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
    ss << getenv("TEST_FILES") << "/old_format/" << file;
    std::cerr << "Loading '" << ss.str() << "'" << std::endl;
    return std::ifstream(ss.str());
}

BOOST_AUTO_TEST_CASE(Places) {

    class PBuilder : public DummyBuilder {

        void addPlace(const std::string& name,
            int tokens,
            bool strict,
            int bound,
            double x,
            double y) {
            BOOST_REQUIRE_EQUAL("TAPN1_P0Sub0", name);
            BOOST_REQUIRE_EQUAL(strict, false);
            BOOST_REQUIRE_EQUAL(bound, 9);
            BOOST_REQUIRE_EQUAL(tokens, 3);
        }

        virtual void addTransition(const std::string &name, bool urgent,
            double, double) {
            BOOST_REQUIRE(false);
        };

        /* Add timed colored input arc with given arc expression*/
        virtual void addInputArc(const std::string &place,
            const std::string &transition,
            bool inhibitor,
            int weight,
            bool lstrict, bool ustrict, int lower, int upper) {
            BOOST_REQUIRE(false);
        };

        /** Add output arc with given weight */
        virtual void addOutputArc(const std::string& transition,
            const std::string& place,
            int weight) {
            BOOST_REQUIRE(false);
        };

        /* Add transport arc with given arc expression */
        virtual void addTransportArc(const std::string& source,
            const std::string& transition,
            const std::string& target, int weight,
            bool lstrict, bool ustrict, int lower, int upper) {
            BOOST_REQUIRE(false);
        }
    };

    auto f = loadFile("single_place.xml");
    BOOST_REQUIRE(f);
    ColoredPetriNetBuilder b;
    b.parseNet(f);
    PBuilder p;
    b.unfold(p);
}


BOOST_AUTO_TEST_CASE(RegularArc) {

    class PBuilder : public DummyBuilder {

        void addPlace(const std::string& name,
            int tokens,
            bool strict,
            int bound,
            double x,
            double y) {
            BOOST_REQUIRE_EQUAL("TAPN1_P1", name);
            BOOST_REQUIRE_EQUAL(strict, false);
            BOOST_REQUIRE_EQUAL(bound, 9);
            BOOST_REQUIRE_EQUAL(tokens, 10);
        }

        virtual void addTransition(const std::string &name, bool urgent,
            double, double) {
            BOOST_REQUIRE_EQUAL("TAPN1_T6", name);
            BOOST_REQUIRE(!urgent);
        };

        /* Add timed colored input arc with given arc expression*/
        virtual void addInputArc(const std::string &place,
            const std::string &transition,
            bool inhibitor,
            int weight,
            bool lstrict, bool ustrict, int lower, int upper) {
            BOOST_REQUIRE_EQUAL("TAPN1_T6", transition);
            BOOST_REQUIRE_EQUAL("TAPN1_P1", place);
            BOOST_REQUIRE_EQUAL(weight, 3);
            BOOST_REQUIRE_EQUAL(lstrict, false);
            BOOST_REQUIRE_EQUAL(ustrict, false);
            BOOST_REQUIRE_EQUAL(lower, 1);
            BOOST_REQUIRE_EQUAL(upper, 5);
        };

        /** Add output arc with given weight */
        virtual void addOutputArc(const std::string& transition,
            const std::string& place,
            int weight) {
            BOOST_REQUIRE_EQUAL("TAPN1_T6", transition);
            BOOST_REQUIRE_EQUAL("TAPN1_P1", place);
            BOOST_REQUIRE_EQUAL(weight, 4);
        };

        /* Add transport arc with given arc expression */
        virtual void addTransportArc(const std::string& source,
            const std::string& transition,
            const std::string& target, int weight,
            bool lstrict, bool ustrict, int lower, int upper) {
            BOOST_REQUIRE(false);
        }
    };

    auto f = loadFile("regular_arc.xml");
    BOOST_REQUIRE(f);
    ColoredPetriNetBuilder b;
    b.parseNet(f);
    PBuilder p;
    b.unfold(p);
}