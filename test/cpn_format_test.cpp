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

namespace utf = boost::unit_test;

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

        virtual void addTransition(const std::string &name, int player, bool urgent,
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

BOOST_AUTO_TEST_CASE(ProductPlaces) {

    class PBuilder : public DummyBuilder {
    public:
        std::map<std::string, int> placemap;
        void addPlace(const std::string& name,
            int tokens,
            bool strict,
            int bound,
            double x,
            double y) {
            if(name.find("DOT") == 0)
            {
                placemap["DOT"] += 1;
            }
            else if(name.find("SIMPLE") == 0)
            {
                placemap["SIMPLE"] += 1;
            }
            else if(name.find("PRODUCT") == 0)
            {
                placemap["PRODUCT"] += 1;
            }
        }

        virtual void addTransition(const std::string &name, int player, bool urgent,
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

    auto f = loadFile("product.xml");
    BOOST_REQUIRE(f);
    ColoredPetriNetBuilder b;
    b.parseNet(f);
    PBuilder p;
    b.unfold(p);
    BOOST_REQUIRE_EQUAL(p.placemap["DOT"], 1);
    BOOST_REQUIRE_EQUAL(p.placemap["SIMPLE"], 2);
    BOOST_REQUIRE_EQUAL(p.placemap["PRODUCT"], 4);
}

BOOST_AUTO_TEST_CASE(PlacesBoundsMap) {

    class PBuilder : public DummyBuilder {

    public:
        size_t n_places = 0;
        std::set<int> bounds{1,2};
        void addPlace(const std::string& name,
            int tokens,
            bool strict,
            int b,
            double x,
            double y) {
            ++n_places;
            BOOST_REQUIRE_EQUAL(name.find("P0"), 0);
            BOOST_REQUIRE_EQUAL(strict, false);
            BOOST_REQUIRE_EQUAL(tokens, 0);
            BOOST_REQUIRE_EQUAL(bounds.count(b), 1);
            bounds.erase(b);
            BOOST_REQUIRE_LE(n_places, 2);
        }

        virtual void addTransition(const std::string &name, int player, bool urgent,
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

    auto f = loadFile("color_inv_map.xml");
    BOOST_REQUIRE(f);
    ColoredPetriNetBuilder b;
    b.parseNet(f);
    PBuilder p;
    b.unfold(p);
    BOOST_REQUIRE_EQUAL(p.n_places, 2);
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
        virtual void addTransition(const std::string &name, int player, bool urgent,
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

BOOST_AUTO_TEST_CASE(RegularArcGuard) {

    class PBuilder : public DummyBuilder {


        void addPlace(const std::string& name,
            int tokens,
            bool strict,
            int bound,
            double x,
            double y) {
        }

        bool seen_transition = false;
        virtual void addTransition(const std::string &name, int player, bool urgent,
            double, double) {
        };

        /* Add timed colored input arc with given arc expression*/
        bool first = true;
        virtual void addInputArc(const std::string &place,
            const std::string &transition,
            bool inhibitor,
            int weight,
            bool lstrict, bool ustrict, int lower, int upper) {
            BOOST_REQUIRE(transition.find("T0") == 0);
            BOOST_REQUIRE(place.find("P0") == 0);
            BOOST_REQUIRE_EQUAL(weight, 1);
            BOOST_REQUIRE_EQUAL(lstrict, false);
            BOOST_REQUIRE_EQUAL(ustrict, false);
            if(first)
            {
                BOOST_REQUIRE_EQUAL(lower, 1);
                BOOST_REQUIRE_EQUAL(upper, 2);
                first = false;
            }
            else
            {
                BOOST_REQUIRE_EQUAL(lower, 2);
                BOOST_REQUIRE_EQUAL(upper, 3);
            }
        };

        /** Add output arc with given weight */
        bool seen_output = false;
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

    auto f = loadFile("color_guard_map.xml");
    BOOST_REQUIRE(f);
    ColoredPetriNetBuilder b;
    b.parseNet(f);
    PBuilder p;
    b.unfold(p);
}


BOOST_AUTO_TEST_CASE(ColoredRegularArc) {

    class PBuilder : public DummyBuilder {
    public:
        size_t n_place = 0;
        void addPlace(const std::string& name,
            int tokens,
            bool strict,
            int bound,
            double x,
            double y) {
            ++n_place;
            if(name.find("P0") == 0)
            {
                BOOST_REQUIRE_EQUAL(strict, false);
                BOOST_REQUIRE_EQUAL(bound, 10);
                BOOST_REQUIRE_EQUAL(tokens, 3);
            }
            else
            {
                BOOST_REQUIRE(name.find("P1") == 0);
                BOOST_REQUIRE_EQUAL(strict, true);
                BOOST_REQUIRE_EQUAL(bound, std::numeric_limits<int>::max());
                BOOST_REQUIRE_EQUAL(tokens, 0);
            }

            BOOST_REQUIRE_LE(n_place, 6);
        }

        size_t n_trans = 0;
        virtual void addTransition(const std::string &name, int player, bool urgent,
            double, double) {
            ++n_trans;
            BOOST_REQUIRE(name.find("T0") == 0);
            BOOST_REQUIRE(!urgent);
            BOOST_REQUIRE_LE(n_trans, 3);
        };

        /* Add timed colored input arc with given arc expression*/
        size_t n_input = 0;
        virtual void addInputArc(const std::string &place,
            const std::string &transition,
            bool inhibitor,
            int weight,
            bool lstrict, bool ustrict, int lower, int upper) {
            ++n_input;
            BOOST_REQUIRE(transition.find("T0") == 0);
            BOOST_REQUIRE(place.find("P0") == 0);
            BOOST_REQUIRE_LE(weight, 2);
            BOOST_REQUIRE_EQUAL(lstrict, false);
            BOOST_REQUIRE_EQUAL(ustrict, false);
            BOOST_REQUIRE_GE(lower, 1);
            BOOST_REQUIRE_LE(upper, 4);
            BOOST_REQUIRE_LE(n_input, 5);
        };

        /** Add output arc with given weight */
        size_t n_output = 0;
        virtual void addOutputArc(const std::string& transition,
            const std::string& place,
            int weight) {
            ++n_output;
            BOOST_REQUIRE(transition.find("T0") == 0);
            BOOST_REQUIRE(place.find("P1") == 0);
            BOOST_REQUIRE_LE(weight, 2);
            BOOST_REQUIRE_LE(n_output, 5);
        };

        /* Add transport arc with given arc expression */
        virtual void addTransportArc(const std::string& source,
            const std::string& transition,
            const std::string& target, int weight,
            bool lstrict, bool ustrict, int lower, int upper) {
            BOOST_REQUIRE(false);
        }
    };

    auto f = loadFile("colored_regular_arc.xml");
    BOOST_REQUIRE(f);
    ColoredPetriNetBuilder b;
    b.parseNet(f);
    PBuilder p;
    b.unfold(p);
    BOOST_REQUIRE_EQUAL(p.n_place, 6);
    BOOST_REQUIRE_EQUAL(p.n_trans, 3);
    BOOST_REQUIRE_EQUAL(p.n_input, 5);
    BOOST_REQUIRE_EQUAL(p.n_output, 5);
}


BOOST_AUTO_TEST_CASE(InhibitorArc) {

    class PBuilder : public DummyBuilder {
    public:
        size_t n_places = 0;
        std::set<std::string> place_names{"P0__0","P0__1", "P0__2"};
        void addPlace(const std::string& name,
            int tokens,
            bool strict,
            int bound,
            double x,
            double y) {
            ++n_places;
            if(name.find("P0") == 0)
            {
                BOOST_REQUIRE(place_names.count(name) == 1);
                place_names.erase(name);
                BOOST_REQUIRE_EQUAL(strict, false);
                BOOST_REQUIRE_EQUAL(bound, 5);
                if(name == "P0__0")
                    BOOST_REQUIRE_EQUAL(tokens, 2);
                else if(name == "P0__1")
                    BOOST_REQUIRE_EQUAL(tokens, 1);
                else if(name == "P0__2")
                    BOOST_REQUIRE_EQUAL(tokens, 5);
            }
            else
            {
                BOOST_REQUIRE_EQUAL(tokens, 8);
            }
            BOOST_REQUIRE_LE(n_places, 4);
        }

        size_t n_trans = 0;
        virtual void addTransition(const std::string &name, int player, bool urgent,
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
            BOOST_REQUIRE_EQUAL("__P0__SUM", place);
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
        std::set<std::string> place_names{"P0__0","P0__1", "P0__2"};
        void addPlace(const std::string& name,
            int tokens,
            bool strict,
            int bound,
            double x,
            double y) {
            ++n_places;
            if(name.compare("__P0__SUM") != 0)
            {
                BOOST_REQUIRE(place_names.count(name) == 1);
                place_names.erase(name);
                BOOST_REQUIRE_EQUAL(strict, false);
                if(name == "P0__0")
                {
                    BOOST_REQUIRE_EQUAL(tokens, 1);
                    BOOST_REQUIRE_EQUAL(bound, 10);
                }
                else if(name == "P0__1")
                {
                    BOOST_REQUIRE_EQUAL(tokens, 3);
                    BOOST_REQUIRE_EQUAL(bound, 1);
                }
                else if(name == "P0__2")
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
        virtual void addTransition(const std::string &name, int player, bool urgent,
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
    BOOST_REQUIRE_EQUAL(p.n_places, 3);
}

BOOST_AUTO_TEST_CASE(TransportArc) {

    class PBuilder : public DummyBuilder {
    public:
        size_t n_places = 0;
        std::set<std::string> place_names{"P0__0","P0__1", "P0__2"};
        void addPlace(const std::string& name,
            int tokens,
            bool strict,
            int bound,
            double x,
            double y) {
            ++n_places;
            if(name.find("P0") == 0)
            {
                if(name.compare("__P0__SUM") != 0)
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
            BOOST_REQUIRE_LE(n_places, 6);
        }

        size_t n_trans = 0;
        virtual void addTransition(const std::string &name, int player, bool urgent,
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
    BOOST_REQUIRE_EQUAL(p.n_places, 6);
    BOOST_REQUIRE_EQUAL(p.n_trans, 3);
    BOOST_REQUIRE_EQUAL(p.n_transport, 3);
}

BOOST_AUTO_TEST_CASE(ColoredGuardTest) {

    class PBuilder : public DummyBuilder {
    public:
        std::map<std::string, int> n_places;
        bool seen_sum = false;
        void addPlace(const std::string& name,
            int tokens,
            bool strict,
            int bound,
            double x,
            double y) {
            if(name.find("P0") == 0)
            {
                n_places["P0"] += 1;
                BOOST_REQUIRE_EQUAL(strict, true);
                BOOST_REQUIRE_EQUAL(bound, std::numeric_limits<int>::max());
                BOOST_REQUIRE_EQUAL(tokens, 0);
            }
            else if(name.find("P1") == 0)
            {
                n_places["P1"] += 1;
                BOOST_REQUIRE_EQUAL(strict, true);
                BOOST_REQUIRE_EQUAL(bound, std::numeric_limits<int>::max());
                BOOST_REQUIRE_EQUAL(tokens, 0);
            }
            else if(name.find("P2") == 0)
            {
                if(name[0] == '_')
                {
                    BOOST_REQUIRE(!seen_sum);
                    seen_sum = true;
                }
                n_places["P2"] += 1;
                BOOST_REQUIRE_EQUAL(strict, true);
                BOOST_REQUIRE_EQUAL(bound, std::numeric_limits<int>::max());
                BOOST_REQUIRE_EQUAL(tokens, 0);
            }
            else if(name == "__P2__SUM")
            {
                BOOST_REQUIRE(!seen_sum);
                seen_sum = true;
                n_places["P2"] += 1;
                BOOST_REQUIRE_EQUAL(strict, true);
                BOOST_REQUIRE_EQUAL(bound, std::numeric_limits<int>::max());
                BOOST_REQUIRE_EQUAL(tokens, 0);
            }
            else
            {
                BOOST_REQUIRE(false);
            }
        }

        size_t n_trans = 0;
        virtual void addTransition(const std::string &name, int player, bool urgent,
            double, double) {
            ++n_trans;
            BOOST_REQUIRE(name.find("T0") == 0);
            BOOST_REQUIRE(!urgent);
            BOOST_REQUIRE_LE(n_trans, 2);
        };

        /* Add timed colored input arc with given arc expression*/
        size_t n_input = 0;
        size_t n_inhib = 0;
        virtual void addInputArc(const std::string &place,
            const std::string &transition,
            bool inhibitor,
            int weight,
            bool lstrict, bool ustrict, int lower, int upper) {
            if(inhibitor)
            {
                ++n_inhib;
                BOOST_REQUIRE(place == "__P2__SUM");
                BOOST_REQUIRE(transition.find("T0") == 0);
                BOOST_REQUIRE_EQUAL(weight, 1);
            }
            else
            {
                ++n_input;
                BOOST_REQUIRE(transition.find("T0") == 0);
                BOOST_REQUIRE(place.find("P0") == 0 || place.find("P1") == 0);
                BOOST_REQUIRE_EQUAL(weight, 1);
            }
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

    auto f = loadFile("guard_test.xml");
    BOOST_REQUIRE(f);
    ColoredPetriNetBuilder b;
    b.parseNet(f);
    PBuilder p;
    b.unfold(p);
    BOOST_REQUIRE_EQUAL(p.n_places["P0"], 2);
    BOOST_REQUIRE_EQUAL(p.n_places["P1"], 2);
    BOOST_REQUIRE_EQUAL(p.n_places["P2"], 3); // should have sum-place
    BOOST_REQUIRE(p.seen_sum);
    BOOST_REQUIRE_EQUAL(p.n_trans, 2);
    BOOST_REQUIRE_EQUAL(p.n_input, 4);
    BOOST_REQUIRE_EQUAL(p.n_inhib, 2);
}

BOOST_AUTO_TEST_CASE(Referendum) {

    class PBuilder : public DummyBuilder {
    public:
        std::map<std::string, int> n_places;
        bool seen_sum = false;
        void addPlace(const std::string& name,
            int tokens,
            bool strict,
            int bound,
            double x,
            double y) {
            if(name.find("ready") == 0)
                BOOST_REQUIRE_EQUAL(tokens, 1);
            else
                BOOST_REQUIRE_EQUAL(tokens, 0);

        }

        size_t n_trans = 0;
        virtual void addTransition(const std::string &name, int player, bool urgent,
            double, double) {
        };

        /* Add timed colored input arc with given arc expression*/
        size_t n_input = 0;
        size_t n_inhib = 0;
        virtual void addInputArc(const std::string &place,
            const std::string &transition,
            bool inhibitor,
            int weight,
            bool lstrict, bool ustrict, int lower, int upper) {
            BOOST_REQUIRE_EQUAL(inhibitor, false);
            BOOST_REQUIRE_EQUAL(weight, 1);
            if(place.find("ready") != 0)
            {
                if(place != "voting__0")
                {
                    BOOST_REQUIRE_EQUAL(upper, 2);
                    BOOST_REQUIRE_LE(lower, 2);
                    BOOST_REQUIRE_GE(lower, 1);
                }
                else
                {
                    BOOST_REQUIRE(
                        (lower == 0 && upper == std::numeric_limits<int>::max()) ||
                        (lower == 1 && upper == 4)
                        );
                }
            }
        };

        /** Add output arc with given weight */
        virtual void addOutputArc(const std::string& transition,
            const std::string& place,
            int weight) {
            BOOST_REQUIRE_EQUAL(weight, 1);
        };

        /* Add transport arc with given arc expression */
        virtual void addTransportArc(const std::string& source,
            const std::string& transition,
            const std::string& target, int weight,
            bool lstrict, bool ustrict, int lower, int upper) {
            BOOST_REQUIRE(false);
        }
    };

    auto f = loadFile("referendum.xml");
    BOOST_REQUIRE(f);
    ColoredPetriNetBuilder b;
    b.parseNet(f);
    PBuilder p;
    b.unfold(p);

}

BOOST_AUTO_TEST_CASE(TupleTest) {
    class PBuilder : public DummyBuilder {
    public:
        void addPlace(const std::string& name,
            int tokens,
            bool strict,
            int bound,
            double x,
            double y) {
        }

        virtual void addTransition(const std::string &name, int player, bool urgent,
            double, double) {
        };

        virtual void addInputArc(const std::string &place,
            const std::string &transition,
            bool inhibitor,
            int weight,
            bool lstrict, bool ustrict, int lower, int upper) {
        };

        /** Add output arc with given weight */
        virtual void addOutputArc(const std::string& transition,
            const std::string& place,
            int weight) {
        };

        /* Add transport arc with given arc expression */
        virtual void addTransportArc(const std::string& source,
            const std::string& transition,
            const std::string& target, int weight,
            bool lstrict, bool ustrict, int lower, int upper) {
        }
    };

    auto f = loadFile("tuple_test.xml");
    BOOST_REQUIRE(f);
    ColoredPetriNetBuilder b;
    b.parseNet(f);
    PBuilder p;
    b.unfold(p);
}

BOOST_AUTO_TEST_CASE(TupleCompTest) {
    class PBuilder : public DummyBuilder {
    public:
        void addPlace(const std::string& name,
            int tokens,
            bool strict,
            int bound,
            double x,
            double y) {
        }

        virtual void addTransition(const std::string &name, int player, bool urgent,
            double, double) {
        };

        virtual void addInputArc(const std::string &place,
            const std::string &transition,
            bool inhibitor,
            int weight,
            bool lstrict, bool ustrict, int lower, int upper) {
        };

        /** Add output arc with given weight */
        virtual void addOutputArc(const std::string& transition,
            const std::string& place,
            int weight) {
        };

        /* Add transport arc with given arc expression */
        virtual void addTransportArc(const std::string& source,
            const std::string& transition,
            const std::string& target, int weight,
            bool lstrict, bool ustrict, int lower, int upper) {
        }
    };

    auto f = loadFile("tuple_comp.xml");
    BOOST_REQUIRE(f);
    ColoredPetriNetBuilder b;
    b.parseNet(f);
    PBuilder p;
    b.unfold(p);
}


BOOST_AUTO_TEST_CASE(TupleCompTest2) {
    class PBuilder : public DummyBuilder {
    public:
        size_t _ntrans = 0;
        size_t _ninput = 0;
        size_t _noutput = 0;

        void addPlace(const std::string& name,
            int tokens,
            bool strict,
            int bound,
            double x,
            double y) {
        }

        virtual void addTransition(const std::string &name, int player, bool urgent,
            double, double) {
            ++_ntrans;
            BOOST_REQUIRE(urgent);
        };

        virtual void addInputArc(const std::string &place,
            const std::string &transition,
            bool inhibitor,
            int weight,
            bool lstrict, bool ustrict, int lower, int upper) {
            ++_ninput;
        };

        /** Add output arc with given weight */
        virtual void addOutputArc(const std::string& transition,
            const std::string& place,
            int weight) {
            ++_noutput;
        };

        /* Add transport arc with given arc expression */
        virtual void addTransportArc(const std::string& source,
            const std::string& transition,
            const std::string& target, int weight,
            bool lstrict, bool ustrict, int lower, int upper) {
        }
    };

    auto f = loadFile("tuple_comp_2.xml");
    BOOST_REQUIRE(f);
    ColoredPetriNetBuilder b;
    b.parseNet(f);
    PBuilder p;
    b.unfold(p);
    BOOST_REQUIRE_EQUAL(1, p._ninput);
    BOOST_REQUIRE_EQUAL(1, p._noutput);
    BOOST_REQUIRE_EQUAL(1, p._ntrans);
}

BOOST_AUTO_TEST_CASE(ColorGuardTest2) {
    class PBuilder : public DummyBuilder {
    public:
        void addPlace(const std::string& name,
            int tokens,
            bool strict,
            int bound,
            double x,
            double y) {
        }

        size_t n_trans = 0;
        virtual void addTransition(const std::string &name, int player, bool urgent,
            double, double) {
            ++n_trans;
        };

        virtual void addInputArc(const std::string &place,
            const std::string &transition,
            bool inhibitor,
            int weight,
            bool lstrict, bool ustrict, int lower, int upper) {
        };

        /** Add output arc with given weight */
        virtual void addOutputArc(const std::string& transition,
            const std::string& place,
            int weight) {
        };

        /* Add transport arc with given arc expression */
        virtual void addTransportArc(const std::string& source,
            const std::string& transition,
            const std::string& target, int weight,
            bool lstrict, bool ustrict, int lower, int upper) {
        }
    };

    auto f = loadFile("guard_test_2.xml");
    BOOST_REQUIRE(f);
    ColoredPetriNetBuilder b;
    b.parseNet(f);
    PBuilder p;
    b.unfold(p);
    BOOST_REQUIRE_EQUAL(p.n_trans, 1);
}


BOOST_AUTO_TEST_CASE(IntRange) {
    class PBuilder : public DummyBuilder {
    public:
        void addPlace(const std::string& name,
            int tokens,
            bool strict,
            int bound,
            double x,
            double y) {
        }

        size_t n_trans = 0;
        virtual void addTransition(const std::string &name, int player, bool urgent,
            double, double) {
            ++n_trans;
        };

        virtual void addInputArc(const std::string &place,
            const std::string &transition,
            bool inhibitor,
            int weight,
            bool lstrict, bool ustrict, int lower, int upper) {
        };

        /** Add output arc with given weight */
        virtual void addOutputArc(const std::string& transition,
            const std::string& place,
            int weight) {
        };

        /* Add transport arc with given arc expression */
        virtual void addTransportArc(const std::string& source,
            const std::string& transition,
            const std::string& target, int weight,
            bool lstrict, bool ustrict, int lower, int upper) {
            BOOST_REQUIRE(false);
        }
    };

    auto f = loadFile("int_range.pnml");
    BOOST_REQUIRE(f);
    ColoredPetriNetBuilder b;
    b.parseNet(f);
    PBuilder p;
    b.unfold(p);
    BOOST_REQUIRE_EQUAL(p.n_trans, 3);
}


BOOST_AUTO_TEST_CASE(UnfoldLoop, * utf::timeout(5)) {
    class PBuilder : public DummyBuilder {
    public:
        void addPlace(const std::string& name,
            int tokens,
            bool strict,
            int bound,
            double x,
            double y) {
        }

        size_t n_trans = 0;
        virtual void addTransition(const std::string &name, int player, bool urgent,
            double, double) {
            ++n_trans;
        };

        virtual void addInputArc(const std::string &place,
            const std::string &transition,
            bool inhibitor,
            int weight,
            bool lstrict, bool ustrict, int lower, int upper) {
        };

        /** Add output arc with given weight */
        virtual void addOutputArc(const std::string& transition,
            const std::string& place,
            int weight) {
        };

        /* Add transport arc with given arc expression */
        virtual void addTransportArc(const std::string& source,
            const std::string& transition,
            const std::string& target, int weight,
            bool lstrict, bool ustrict, int lower, int upper) {
            BOOST_REQUIRE(false);
        }
    };

    auto f = loadFile("unfold_loop.pnml");
    BOOST_REQUIRE(f);
    ColoredPetriNetBuilder b;
    b.parseNet(f);
    PBuilder p;
    b.unfold(p);
}

BOOST_AUTO_TEST_CASE(UnsatCPNGuard, * utf::timeout(5)) {
    class PBuilder : public DummyBuilder {
    public:
        void addPlace(const std::string& name,
            int tokens,
            bool strict,
            int bound,
            double x,
            double y) {
        }

        size_t n_trans = 0;
        virtual void addTransition(const std::string &name, int player, bool urgent,
            double, double) {
            BOOST_REQUIRE(false);
        };

        virtual void addInputArc(const std::string &place,
            const std::string &transition,
            bool inhibitor,
            int weight,
            bool lstrict, bool ustrict, int lower, int upper) {
        };

        /** Add output arc with given weight */
        virtual void addOutputArc(const std::string& transition,
            const std::string& place,
            int weight) {
        };

        /* Add transport arc with given arc expression */
        virtual void addTransportArc(const std::string& source,
            const std::string& transition,
            const std::string& target, int weight,
            bool lstrict, bool ustrict, int lower, int upper) {
            BOOST_REQUIRE(false);
        }
    };

    auto f = loadFile("unsat_cpn_guard.pnml");
    BOOST_REQUIRE(f);
    ColoredPetriNetBuilder b;
    b.parseNet(f);
    PBuilder p;
    b.unfold(p);
}

BOOST_AUTO_TEST_CASE(GameTest) {

    class PBuilder : public DummyBuilder {


        void addPlace(const std::string& name,
            int tokens,
            bool strict,
            int bound,
            double x,
            double y) {
        }

        virtual void addTransition(const std::string &name, int player, bool urgent,
            double, double) {
            if(name.find("CTRL") == 0)
            {
                BOOST_REQUIRE(player == 0);
            }
            else
            {
                BOOST_REQUIRE(player == 1);
            }
        };

        /* Add timed colored input arc with given arc expression*/
        virtual void addInputArc(const std::string &place,
            const std::string &transition,
            bool inhibitor,
            int weight,
            bool lstrict, bool ustrict, int lower, int upper) {
        };

        /** Add output arc with given weight */
        virtual void addOutputArc(const std::string& transition,
            const std::string& place,
            int weight) {
        };

        /* Add transport arc with given arc expression */
        virtual void addTransportArc(const std::string& source,
            const std::string& transition,
            const std::string& target, int weight,
            bool lstrict, bool ustrict, int lower, int upper) {
            BOOST_REQUIRE(false);
        }
    };

    auto f = loadFile("game.xml");
    BOOST_REQUIRE(f);
    ColoredPetriNetBuilder b;
    b.parseNet(f);
    PBuilder p;
    b.unfold(p);
}
