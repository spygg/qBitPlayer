#include "bittorrentserver.h"

Q_GLOBAL_STATIC(BitTorrentServer, bitTorrentServer)

BitTorrentServer *BitTorrentServer::instance()
{
    return bitTorrentServer();
}


BitTorrentServer::BitTorrentServer()
{

}

qint16 BitTorrentServer::getListenPort()
{
    BitTorrentServer *server = BitTorrentServer::instance();
    if(!server->isListening())
    {
        for(int i = MIN_PORT; i <= MAX_PORT; i++)
        {
            if(server->listen(QHostAddress::Any, i))
            {
                break;
            }
        }
        //判断是否失败
        if(!server->isListening())
            return 0;
    }

    return server->serverPort();
}


