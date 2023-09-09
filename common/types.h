#ifndef INCLUDED_TYPES
#define INCLUDED_TYPES

#include <functional>

namespace crypto_trader {

namespace common {

class Types {
public:
    // PUBLIC TYPES
    struct Action {
        enum ActionType {
            e_BUY = 0,
            e_SELL = 1
        };

        // DATA
        // Type of the emitted action.
        ActionType d_type;
    }; // struct Action
    typedef std::function<void(const Action&)> Emit;


}; // class Types

} // common
} // crypto_trader


#endif // INCLUDED_TYPES
