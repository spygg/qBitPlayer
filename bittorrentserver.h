#ifndef BITTORRENTSERVER_H
#define BITTORRENTSERVER_H

#include <QTcpServer>

#define MIN_PORT 6681
#define MAX_PORT 6689

class BitTorrentServer : public QTcpServer
{
    Q_OBJECT

public:
    BitTorrentServer();
    static BitTorrentServer *instance();
    qint16 getListenPort();
};

#endif // BITTORRENTSERVER_H
