
#include "Colored/Expressions.h"
#include "errorcodes.h"


namespace unfoldtacpn {
namespace Colored {
    const Color* ExpressionContext::findColor(const std::string& color) const {
        if (color.compare("dot") == 0)
           return DotConstant::dotConstant(nullptr);
       for (auto& elem : colorTypes) {
           auto col = (*elem.second)[color];
           if (col)
               return col;
       }
       printf("Could not find color: %s\nCANNOT_COMPUTE\n", color.c_str());
       exit(ErrorCode);
    }

    ProductType* ExpressionContext::findProductColorType(const std::vector<const ColorType*>& types) const {
        for (auto& elem : colorTypes) {
            auto* pt = dynamic_cast<ProductType*>(elem.second);
            if (pt && pt->containsTypes(types)) {
                return pt;
            }
        }
        return nullptr;
    }
}
}