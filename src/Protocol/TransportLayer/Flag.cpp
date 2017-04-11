#include "Flag.h"

#include <QStringList>

namespace Protocol {
namespace TransportLayer {

const char* flagDebug(Flags flag) {
  QStringList result;

  for (int I = 0; I < 2; I++) {
    if ((flag & (1 << I)) != 0) {
      switch (I) {
      case 0:
        result << "Syn";
        break;
      case 1:
        result << "Ack";
        break;
      }
    }
  }

  return result.join("|").toUtf8();
}
}
}
