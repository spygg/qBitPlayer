#include "peerserver.h"

Q_GLOBAL_STATIC(PeerServer, bitTorrentServer)

PeerServer *PeerServer::instance()
{
    return bitTorrentServer();
}


PeerServer::PeerServer()
{

}

qint16 PeerServer::getListenPort()
{
    PeerServer *server = PeerServer::instance();
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

void PeerServer::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket *socket = new QTcpSocket(this);
    socket->setSocketDescriptor(socketDescriptor);

    qDebug() << "本Tcp服务接收到新连接" << socket->peerAddress() << ":" << socket->peerPort();
}


