#ifndef HEADER_H
#define HEADER_H

#include "Flag.h"

namespace Protocol {

struct Header
{
    qint32 seqNum;
    qint32 ackNum;
    Flags  flags;

public:
    Header();
};

QDataStream & operator<< (QDataStream & stream, const Header & object);
QDataStream & operator>> (QDataStream & stream, Header & object);

} // namespace Protocol

#endif // HEADER_H
