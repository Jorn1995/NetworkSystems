#ifndef FLAG_H
#define FLAG_H

#include "QFlags"

namespace Protocol {
namespace TransportLayer {

enum Flag {
  Sync = 0x01,
  Acknowledgement = 0x02,
};

Q_DECLARE_FLAGS(Flags, Flag)
const char *flagDebug(Flags flag);
}
}

Q_DECLARE_OPERATORS_FOR_FLAGS(Protocol::TransportLayer::Flags)

#endif // FLAG_H
