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
    ss << getenv("TEST_FILES") << "/cpn_format/" << file;
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
            BOOST_REQUIRE_EQUAL("P0", name);
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

        bool seen_place = false;
        void addPlace(const std::string& name,
            int tokens,
            bool strict,
            int bound,
            double x,
            double y) {
            BOOST_REQUIRE_EQUAL("P0", name);
            BOOST_REQUIRE_EQUAL(strict, false);
            BOOST_REQUIRE_EQUAL(bound, 9);
            BOOST_REQUIRE_EQUAL(tokens, 10);
            BOOST_REQUIRE(!seen_place);
            seen_place = true;
        }

        bool seen_transition = false;
        virtual void addTransition(const std::string &name, bool urgent,
            double, double) {
            BOOST_REQUIRE_EQUAL("T2", name);
            BOOST_REQUIRE(!urgent);
            BOOST_REQUIRE(!seen_transition);
            seen_transition = true;
        };

        /* Add timed colored input arc with given arc expression*/
        bool seen_input = false;
        virtual void addInputArc(const std::string &place,
            const std::string &transition,
            bool inhibitor,
            int weight,
            bool lstrict, bool ustrict, int lower, int upper) {
            BOOST_REQUIRE_EQUAL("T2", transition);
            BOOST_REQUIRE_EQUAL("P0", place);
            BOOST_REQUIRE_EQUAL(weight, 3);
            BOOST_REQUIRE_EQUAL(lstrict, false);
            BOOST_REQUIRE_EQUAL(ustrict, false);
            BOOST_REQUIRE_EQUAL(lower, 1);
            BOOST_REQUIRE_EQUAL(upper, 5);
            BOOST_REQUIRE(!seen_input);
            seen_input = true;
        };

        /** Add output arc with given weight */
        bool seen_output = false;
        virtual void addOutputArc(const std::string& transition,
            const std::string& place,
            int weight) {
            BOOST_REQUIRE_EQUAL("T2", transition);
            BOOST_REQUIRE_EQUAL("P0", place);
            BOOST_REQUIRE_EQUAL(weight, 4);
            BOOST_REQUIRE(!seen_output);
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

BOOST_AUTO_TEST_CASE(InhibitorArc) {

    class PBuilder : public DummyBuilder {
    public:
        size_t n_places = 0;
        std::set<std::string> place_names{"P0Sub0","P0Sub1", "P0Sub2"};
        void addPlace(const std::string& name,
            int tokens,
            bool strict,
            int bound,
            double x,
            double y) {
            ++n_places;
            if(name.compare("P0Sum") != 0)
            {
                BOOST_REQUIRE(place_names.count(name) == 1);
                place_names.erase(name);
                BOOST_REQUIRE_EQUAL(strict, false);
                BOOST_REQUIRE_EQUAL(bound, 5);
                if(name == "P0Sub0")
                    BOOST_REQUIRE_EQUAL(tokens, 2);
                else if(name == "P0Sub1")
                    BOOST_REQUIRE_EQUAL(tokens, 1);
                else if(name == "P0Sub2")
                    BOOST_REQUIRE_EQUAL(tokens, 5);
            }
            else
            {
                BOOST_REQUIRE_EQUAL(tokens, 8);
            }
            BOOST_REQUIRE_LE(n_places, 4);
        }

        size_t n_trans = 0;
        virtual void addTransition(const std::string &name, bool urgent,
            double, double) {
            ++n_trans;
            BOOST_REQUIRE_EQUAL("T0", name);
            BOOST_REQUIRE(!urgent);
            BOOST_REQUIRE_LE(n_trans, 1);
        };

        /* Add timed colored input arc with given arc expression*/
        size_t n_input = 0;
        virtual void addInputArc(const std::string &place,
            const std::string &transition,
            bool inhibitor,
            int weight,
            bool lstrict, bool ustrict, int lower, int upper) {
            ++n_input;
            BOOST_REQUIRE_EQUAL("T0", transition);
            BOOST_REQUIRE_EQUAL("P0Sum", place);
            BOOST_REQUIRE_EQUAL(weight, 3);
            BOOST_REQUIRE_EQUAL(lstrict, false);
            BOOST_REQUIRE_EQUAL(ustrict, true);
            BOOST_REQUIRE_EQUAL(lower, 0);
            BOOST_REQUIRE_EQUAL(inhibitor, true);
            BOOST_REQUIRE_EQUAL(upper, std::numeric_limits<int>::max());
            BOOST_REQUIRE_LE(n_input, 1);
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

    auto f = loadFile("inhib_arc.xml");
    BOOST_REQUIRE(f);
    ColoredPetriNetBuilder b;
    b.parseNet(f);
    PBuilder p;
    b.unfold(p);
    BOOST_REQUIRE_EQUAL(p.n_places, 4);
    BOOST_REQUIRE_EQUAL(p.n_trans, 1);
    BOOST_REQUIRE_EQUAL(p.n_input, 1);
}


BOOST_AUTO_TEST_CASE(SinglePlace) {

    class PBuilder : public DummyBuilder {
    public:
        size_t n_places = 0;
        std::set<std::string> place_names{"P0Sub0","P0Sub1", "P0Sub2"};
        void addPlace(const std::string& name,
            int tokens,
            bool strict,
            int bound,
            double x,
            double y) {
            ++n_places;
            if(name.compare("P0Sum") != 0)
            {
                BOOST_REQUIRE(place_names.count(name) == 1);
                place_names.erase(name);
                BOOST_REQUIRE_EQUAL(strict, false);
                if(name == "P0Sub0")
                {
                    BOOST_REQUIRE_EQUAL(tokens, 1);
                    BOOST_REQUIRE_EQUAL(bound, 10);
                }
                else if(name == "P0Sub1")
                {
                    BOOST_REQUIRE_EQUAL(tokens, 3);
                    BOOST_REQUIRE_EQUAL(bound, 1);
                }
                else if(name == "P0Sub2")
                {
                    BOOST_REQUIRE_EQUAL(tokens, 5);
                    BOOST_REQUIRE_EQUAL(bound, 15);
                }
            }
            else
            {
                BOOST_REQUIRE_EQUAL(tokens, 9);
            }
            BOOST_REQUIRE_LE(n_places, 4);
        }

        size_t n_trans = 0;
        virtual void addTransition(const std::string &name, bool urgent,
            double, double) {
            ++n_trans;
            BOOST_REQUIRE(false);
        };

        /* Add timed colored input arc with given arc expression*/
        size_t n_input = 0;
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

    auto f = loadFile("single_colored_place.xml");
    BOOST_REQUIRE(f);
    ColoredPetriNetBuilder b;
    b.parseNet(f);
    PBuilder p;
    b.unfold(p);
    BOOST_REQUIRE_EQUAL(p.n_places, 4);
}

BOOST_AUTO_TEST_CASE(TransportArc) {

    class PBuilder : public DummyBuilder {
    public:
        size_t n_places = 0;
        std::set<std::string> place_names{"P0Sub0","P0Sub1", "P0Sub2"};
        void addPlace(const std::string& name,
            int tokens,
            bool strict,
            int bound,
            double x,
            double y) {
            ++n_places;
            if(name.find("P0") == 0)
            {
                if(name.compare("P0Sum") != 0)
                {
                    BOOST_REQUIRE(place_names.count(name) == 1);
                    place_names.erase(name);
                    BOOST_REQUIRE_EQUAL(strict, false);
                    BOOST_REQUIRE_EQUAL(bound, 10);
                    BOOST_REQUIRE_EQUAL(tokens, 3);
                }
                else
                {
                    BOOST_REQUIRE_EQUAL(tokens, 9);
                }
            }
            else {
                BOOST_REQUIRE(name.find("P1") == 0);
                BOOST_REQUIRE_EQUAL(tokens, 0);
                BOOST_REQUIRE_EQUAL(strict, true);
                BOOST_REQUIRE_EQUAL(bound, std::numeric_limits<int>::max());
            }
            BOOST_REQUIRE_LE(n_places, 8);
        }

        size_t n_trans = 0;
        virtual void addTransition(const std::string &name, bool urgent,
            double, double) {
            ++n_trans;
            BOOST_REQUIRE(name.find("T0") == 0);
            BOOST_REQUIRE(!urgent);
            BOOST_REQUIRE_LE(n_trans, 3);
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

        size_t n_transport = 0;
        virtual void addTransportArc(const std::string& source,
            const std::string& transition,
            const std::string& target, int weight,
            bool lstrict, bool ustrict, int lower, int upper) {
            ++n_transport;


        }
    };

    auto f = loadFile("transport_arc.xml");
    BOOST_REQUIRE(f);
    ColoredPetriNetBuilder b;
    b.parseNet(f);
    PBuilder p;
    b.unfold(p);
    BOOST_REQUIRE_EQUAL(p.n_places, 8);
    BOOST_REQUIRE_EQUAL(p.n_trans, 3);
    BOOST_REQUIRE_EQUAL(p.n_transport, 3);
}