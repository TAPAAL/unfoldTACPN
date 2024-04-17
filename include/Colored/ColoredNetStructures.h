/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   ColoredNetStructures.h
 * Author: Klostergaard
 *
 * Created on 17. februar 2018, 17:07
 */

#ifndef COLOREDNETSTRUCTURES_H
#define COLOREDNETSTRUCTURES_H

#include <vector>
#include <set>
#include "Colors.h"
#include "Expressions.h"
#include "Multiset.h"
#include "TimeInterval.h"
#include "TimeInvariant.h"

namespace unfoldtacpn {
    namespace Colored {

        struct Arc {
            uint32_t place;
            uint32_t transition;
            ArcExpression_ptr expr;
            bool input;
            bool inhibitor = false;
            int weight;
            std::vector<Colored::TimeInterval> interval;
        };

        struct TransportArc {
            uint32_t source;
            uint32_t transition;
            uint32_t destination;
            ArcExpression_ptr in_expr;
            ArcExpression_ptr out_expr;
            int weight;
            std::vector<Colored::TimeInterval> interval;
        };

        struct Transition {
            std::string name;
            GuardExpression_ptr guard;
            int player;
            bool urgent;
            std::vector<Arc> arcs;
            std::vector<TransportArc> transport;
            float probabilityRate = -1;
        };

        struct Place {
            std::string name;
            const ColorType* type;
            Multiset marking;
            std::vector<Colored::TimeInvariant> invariants;
            bool inhibiting = false;
        };
    }
}

#endif /* COLOREDNETSTRUCTURES_H */

