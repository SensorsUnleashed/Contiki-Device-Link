#ifndef SUMESSAGE_H
#define SUMESSAGE_H

#include <QByteArray>
class SUMessage
{
public:
    SUMessage();

//    QByteArray fromPairdata(QVariant pairdata);
//    SUMessage pairmsg(cmp_ctx_t* cmp);
};

class SUPairmsg : public SUMessage {
    SUPairmsg();
};

#endif // SUMESSAGE_H
