
#include "Colored/Expressions.h"
#include "errorcodes.h"


namespace unfoldtacpn {
namespace Colored {
    const Color* ExpressionContext::findColor(const std::string& color) const {
        if (color.compare("dot") == 0)
           return Color::dotConstant();
       for (auto& elem : colorTypes) {
           return &(*elem.second)[color];
       }
       printf("Could not find color: %s\nCANNOT_COMPUTE\n", color.c_str());
       exit(ErrorCode);
    }

    const ProductType* ExpressionContext::findProductColorType(const std::vector<const ColorType*>& types) const {
        for (auto& elem : colorTypes) {
            auto* pt = dynamic_cast<const ProductType*>(elem.second);
            if (pt && pt->containsTypes(types)) {
                return pt;
            }
        }
        return nullptr;
    }
}
}