#ifndef FLAG_H
#define FLAG_H
#include "QFlags"
namespace Protocol {

enum Flag {
    Sync            = 0x01,
    Acknowledgement = 0x02,
};
Q_DECLARE_FLAGS(Flags,Flag)
}

Q_DECLARE_OPERATORS_FOR_FLAGS(Protocol::Flags)

#endif // FLAG_H
