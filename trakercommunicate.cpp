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

    qDebug() << "start download " << url.url() << ".....";

}

void TrakerCommunicate::setBenCodeParse(BenCodeParser *bencodPrase)
{
    m_pBenCodePrase = bencodPrase;
}


//void TrakerCommunicate::httpReadyRead()
//{
//    // this slot gets called every time the QNetworkReply has new data.
//    // We read all of its new data and write it into the file.
//    // That way we use less RAM than when reading it at the finished()
//    // signal of the QNetworkReply
//    m_bData.append(m_pReply->readAll());
//    m_file.write(m_pReply->readAll());
//}


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

//    "d8:completei1552e10:incompletei83e8:intervali1800e5:peers300"

    BenCodeParser parse;
    parse.parseTorrentData(data);

    //解析
}
