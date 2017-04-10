#ifndef FLAG_H
#define FLAG_H
#include "QFlags"
namespace Protocol {

enum Flag {
    Acknowledgement = 0x01
};
Q_DECLARE_FLAGS(Flags,Flag)
}

Q_DECLARE_OPERATORS_FOR_FLAGS(Protocol::Flags)

#endif // FLAG_H
