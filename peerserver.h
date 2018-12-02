#ifndef BITTORRENTSERVER_H
#define BITTORRENTSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include "peerclient.h"

#define MIN_PORT 6681
#define MAX_PORT 6689

//本类用来完成接收peer发来的信息和连接,只有一个即本程序
//嗯,暂时不用线程吧
class PeerServer : public QTcpServer
{
    Q_OBJECT

public:
    PeerServer();
    static PeerServer *instance();
    qint16 getListenPort();


private:
    QList <QTcpSocket*> m_listClientsSocket;

    void incomingConnection(qintptr socketDescriptor) override;

};

#endif // BITTORRENTSERVER_H
