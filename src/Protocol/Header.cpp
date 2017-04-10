#include "Header.h"

#include <QDataStream>

namespace Protocol {

Header::Header()
{

}

QDataStream &operator<<(QDataStream &stream, const Header &object)
{
    return stream << object.ackNum << object.seqNum << qint8(object.flags);
}

QDataStream &operator>>(QDataStream &stream, Header &object)
{
    qint8 flags;
    stream >> object.ackNum >> object.seqNum >> flags;
    object.flags = Flags(flags);

    return stream;
}

} // namespace Protocol
