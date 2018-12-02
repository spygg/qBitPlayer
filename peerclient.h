#ifndef PEERCOMMUNICATE_H
#define PEERCOMMUNICATE_H

#include <QObject>
#include <QTcpSocket>

//本类用来完成发送信息给peer
class PeerClient : public QTcpSocket
{
    Q_OBJECT

public:
    PeerClient();
};

#endif // PEERCOMMUNICATE_H
