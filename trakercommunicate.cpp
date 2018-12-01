#include "trakercommunicate.h"
#include "bittorrentserver.h"

TrakerCommunicate::TrakerCommunicate(QObject *parent) : QObject(parent)
{
    connect(&m_netManger, SIGNAL(finished(QNetworkReply*)), this, SLOT(httpFinished(QNetworkReply*)));
}

QByteArray TrakerCommunicate::getPeerId()
{
    if (m_bPeerId.isEmpty()) {
        // Generate peer id
        qint64 startupTime = QDateTime::currentSecsSinceEpoch();

        m_bPeerId.append("QBTPLAYER-");
        m_bPeerId.append(QByteArray::number(startupTime, 10));
        m_bPeerId.append(QByteArray(20 - m_bPeerId.size(), '-'));
    }
    return m_bPeerId;
}

void TrakerCommunicate::commnicateTracker()
{
//    QByteArray da("3232235777");
//    qDebug() << strlen(da.constData());
//    QHostAddress ad("192.168.1.1");

//    qDebug() << ad.toIPv4Address();
//    return;

    QUrl url = QUrl(m_pBenCodePrase->getAnnounceUrl());
    QUrlQuery query(url);

    //添加infoHash
    QByteArray infoHash = QCryptographicHash::hash(m_pBenCodePrase->getInfoSection(), QCryptographicHash::Sha1);
    query.addQueryItem("info_hash", infoHash.toPercentEncoding());

    //添加peer_id
    query.addQueryItem("peer_id", getPeerId());

    //添加port
    query.addQueryItem("port", QByteArray::number(BitTorrentServer::instance()->getListenPort()));
    query.addQueryItem("compact", "1");

    //上传数
    query.addQueryItem("uploaded", QByteArray::number(0));
    //下载数
    query.addQueryItem("downloaded", "0");

    //剩余数
    query.addQueryItem("left", "0");

    //事件,为started, stopped, completed
    query.addQueryItem("event", "started");

    url.setQuery(query);

    QNetworkRequest req(url);
    m_netManger.get(req);
}

void TrakerCommunicate::setBenCodeParse(BenCodeParser *bencodPrase)
{
    m_pBenCodePrase = bencodPrase;
}

void TrakerCommunicate::httpFinished(QNetworkReply* reply)
{
    reply->deleteLater();
    if(reply->error())
    {
        qDebug() << QString("网络错误,代码为: %1!").arg(reply->error());
        return;
    }

    QByteArray data = reply->readAll();
    qDebug() << data;

    BenCodeParser parse;
    parse.parseTorrentData(data);

    if(parse.getDict().contains("peers"))
    {
        QVariant peers = parse.getDict().value("peers");

        if(peers.type() == QVariant::List)
        {

        }
        else
        {
            QByteArray peerAddrs = peers.toByteArray();
            for(int i = 0; i < peerAddrs.size(); i += 6)
            {
                uchar *p = (uchar*)peerAddrs.constData() + i;
                //大端字节序
                PEER_ADDR tmpAddr;
                tmpAddr.uiIp = qFromBigEndian<quint32>(p);
                tmpAddr.uiPort = qFromBigEndian<quint16>(p + 4);
                m_listPeers.append(tmpAddr);
            }
        }


        for(int j = 0; j < m_listPeers.size(); j++)
        {
            qDebug() << m_listPeers.at(j).uiIp << ":" << m_listPeers.at(j).uiPort;
        }
        //qDebug() << peers;
    }
    //解析
}
