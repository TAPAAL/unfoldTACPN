/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "Colored/Colors.h"
#include "errorcodes.h"
#include <sstream>
#include <string>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <cassert>

//@{
// From: https://stackoverflow.com/a/236803
template<typename Out>
void split(const std::string &s, char delim, Out result) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}
//}@

namespace unfoldtacpn {
    namespace Colored {
        std::ostream& operator<<(std::ostream& stream, const Color& color) {
            stream << color.toString();
            return stream;
        }

        Color::Color() {
            _id = 0;
            _colorName = "*";
            _colorType = StarColorType::starColorType();
        }

        Color::Color(const ColorType* colorType, uint32_t id, const std::vector<const Color*>& colors)
                : _tuple(colors), _colorType(colorType), _colorName(""), _id(id)
        {
            if (colorType != nullptr)
                assert(id <= colorType->size());
        }

        Color::Color(const ColorType* colorType, uint32_t id, const char* color)
                : _tuple(), _colorType(colorType), _colorName(color), _id(id)
        {
            if (colorType != nullptr)
                assert(id <= colorType->size());
        }


        const Color* Color::operator[] (size_t index) const {
            if (!this->isTuple()) {
                throw "Cannot access tuple if not a tuple color";
            }
            return _tuple[index];
        }

        bool Color::operator< (const Color& other) const {
            if (_colorType == other._colorType) {
                throw "Cannot compare colors from different types";
            }
            return _id < other._id;
        }

        bool Color::operator> (const Color& other) const {
            if (_colorType == other._colorType) {
                throw "Cannot compare colors from different types";
            }
            return _id > other._id;
        }

        bool Color::operator<= (const Color& other) const {
            if (_colorType == other._colorType) {
                throw "Cannot compare colors from different types";
            }
            return _id <= other._id;
        }

        bool Color::operator>= (const Color& other) const {
            if (_colorType == other._colorType) {
                throw "Cannot compare colors from different types";
            }
            return _id >= other._id;
        }

        const Color& Color::operator++ () const {
            if (_id >= _colorType->size() - 1) {
                return (*_colorType)[0];
            }
            assert(_id + 1 < _colorType->size());
            return (*_colorType)[_id + 1];
        }

        const Color& Color::operator-- () const {
            if (_id <= 0) {
                return (*_colorType)[_colorType->size() - 1];
            }
            assert(_id - 1 >= 0);
            return (*_colorType)[_id - 1];
        }

        std::string Color::toString() const {
            return toString(this);
        }

        std::string Color::toString(const Color* color) {
            if (color->isTuple()) {
                std::ostringstream oss;
                oss << "(";
                for (size_t i = 0; i < color->_tuple.size(); i++) {
                    oss << color->_tuple[i]->toString();
                    if (i < color->_tuple.size() - 1) oss << ",";
                }
                oss << ")";
                return oss.str();
            }
            return std::string(color->_colorName);
        }

        std::string Color::toString(const std::vector<const Color*>& colors) {
            std::ostringstream oss;
            if (colors.size() > 1)
                oss << "(";

            for (size_t i = 0; i < colors.size(); i++) {
                oss << colors[i]->toString();
                if (i < colors.size() - 1) oss << ",";
            }

            if (colors.size() > 1)
                oss << ")";
            return oss.str();
        }


        const Color* Color::dotConstant() {
            static ColorType* _instance = nullptr;
            if(_instance == nullptr){
                _instance = new ColorType("dot");
                _instance->addColor("dot");
            }
            return &(*_instance)[0];
        }

        StarColorType::StarColorType() : ColorType("*") {

        }

        void ColorType::addColor(const char* colorName) {
            auto nid = _colors.size();
            _colors.emplace_back(this, nid, colorName);
            for(size_t i = 0; i < _colors.size(); ++i)
                assert(i == _colors[i].getId());
        }

        void ColorType::addColor(std::vector<const Color*>& colors) {
            auto nid = _colors.size();
            _colors.emplace_back(this, nid, colors);
            for(size_t i = 0; i < _colors.size(); ++i)
                assert(i == _colors[i].getId());
        }

        const Color& ColorType::operator[] (const char* index) const {
            for (size_t i = 0; i < _colors.size(); i++) {
                if (strcmp(operator[](i).toString().c_str(), index) == 0)
                    return operator[](i);
            }
            std::cerr << "ERROR: Couldn't find color '" << index << "'";
            std::exit(ErrorCode);
        }

        const Color& ScopeType::operator[] (const char* index) const
        {
            for(auto& type : _constituents)
            {
                for(auto& c: *type)
                {
                    if(c.isTuple()) continue;
                    if(c.getColorName() == index)
                        return c;
                }
            }
            std::cerr << "ERROR: Couldn't find color '" << index << "'";
            std::exit(ErrorCode);
        }

        const Color& ProductType::operator[](size_t index) const {
            if (cache.count(index) < 1) {
                size_t mod = 1;
                size_t div = 1;

                std::vector<const Color*> colors;
                for (auto & constituent : constituents) {
                    mod = constituents.size();
                    colors.push_back(&(*constituent)[(index / div) % mod]);
                    div *= mod;
                }

                cache.emplace(index, Color(this, index, colors));
            }

            return cache.at(index);
        }

        const Color* ProductType::getColor(const std::vector<const Color*>& colors) const {
            size_t product = 1;
            size_t sum = 0;

            if (constituents.size() != colors.size()) return nullptr;

            for (size_t i = 0; i < constituents.size(); ++i) {
                if (!(*colors[i]->getColorType() == *constituents[i]))
                    return nullptr;

                sum += product * colors[i]->getId();
                product *= constituents[i]->size();
            }
            return &operator[](sum);
        }

        const Color& ProductType::operator[](const char* index) const {
            return operator[](std::string(index));
        }

        const Color& ProductType::operator[](const std::string& index) const {
            std::string str(index.empty() || index[0] != '(' ? index : index.substr(1, index.size() - 2));
            std::vector<std::string> parts = split(str, ',');

            if (parts.size() != constituents.size()) {
                std::cerr << "ERROR: Zero color constituents" << std::endl;
                std::exit(ErrorCode);
            }

            size_t sum = 0;
            size_t mult = 1;
            for (size_t i = 0; i < parts.size(); ++i) {
                sum += mult * (*constituents[i])[parts[i]].getId();
                mult *= constituents[i]->size();
            }

            return operator[](sum);
        }

    }
}
