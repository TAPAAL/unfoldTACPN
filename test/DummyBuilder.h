/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   DummyBuilder.h
 * Author: Peter G. Jensen
 *
 * Created on 27 November 2021, 21.05
 */

#ifndef DUMMYBUILDER_H
#define DUMMYBUILDER_H

#include <TAPNBuilderInterface.h>

class DummyBuilder : public unfoldtacpn::TAPNBuilderInterface {
    /** Adds a new colored and timed place with a unique name */
    virtual void addPlace(const std::string& name,
            int tokens,
            bool strict,
            int bound,
            double x = 0,
            double y = 0) {};

    // add a time transition with a unique name
    virtual void addTransition(const std::string &name, bool urgent,
            double, double) {};

    /* Add timed colored input arc with given arc expression*/
    virtual void addInputArc(const std::string &place,
            const std::string &transition,
            bool inhibitor,
            int weight,
            bool lstrict, bool ustrict, int lower, int upper) {};

    /** Add output arc with given weight */
    virtual void addOutputArc(const std::string& transition,
            const std::string& place,
            int weight) {};

    /* Add transport arc with given arc expression */
    virtual void addTransportArc(const std::string& source,
            const std::string& transition,
            const std::string& target, int weight,
            bool lstrict, bool ustrict, int lower, int upper) {};
};
#endif /* DUMMYBUILDER_H */

