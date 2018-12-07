#include "trakercommunicate.h"
#include "peerserver.h"

TrakerCommunicate::TrakerCommunicate(QObject *parent) : QObject(parent)
{
    connect(&m_netManger, SIGNAL(finished(QNetworkReply*)), this, SLOT(httpFinished(QNetworkReply*)));

    m_iTimerId = -1;
    m_bFirstCommnicateWithTracker = true;
    m_bLastCommnicateWithTracker = false;
    m_bCompleted = false;
}

QByteArray TrakerCommunicate::getPeerId()
{
    if (m_bytesPeerId.isEmpty()) {
        // Generate peer id
        qint64 startupTime = QDateTime::currentSecsSinceEpoch();

        m_bytesPeerId.append("QBTPLAYER-");
        m_bytesPeerId.append(QByteArray::number(startupTime, 10));
        m_bytesPeerId.append(QByteArray(20 - m_bytesPeerId.size(), '-'));
    }
    return m_bytesPeerId;
}

void TrakerCommunicate::timerEvent(QTimerEvent *event)
{
    if(event->timerId())
    {
//        commnicateTracker();
    }
    else
    {
        QObject::timerEvent(event);
    }
}

static inline void toNetworkData(quint32 num, char *data)
{
    unsigned char *udata = (unsigned char *)data;
    udata[3] = (num & 0xff);
    udata[2] = (num & 0xff00) >> 8;
    udata[1] = (num & 0xff0000) >> 16;
    udata[0] = (num & 0xff000000) >> 24;
}

//typedef struct _PEER_WIRE_MSG
//{
//    char szLen[4];
//    unsigned char byteMsgId;
//    unsigned char* bytesPayload;
//}PEER_WIRE_MSG;

void TrakerCommunicate::commnicateWithTracker()
{
    QBitArray bitField(17);

    bitField[10] = 1;
    bitField[9] = 1;
    bitField[6] = 1;


    qDebug() << bitField;
    int size = (bitField.size() + 7) / 8;
    QByteArray data(size, '\0');

    char message[] = {0, 0, 0, 1, 5};
    qToBigEndian<quint32>(bitField.size() + 1, &message[0]);

    //first byte of the bitfield corresponds to indices 0 - 7 from high bit to low bit
    //0000 0001 0000 0000
    //7654 3210
    // data[0]   data[1]
    //按位或
    unsigned char uc = 0;
    for(int i = 0; i < bitField.size(); i++)
    {
        if(bitField.testBit(i))
        {
            data[i / 8] = data.at(i / 8) | (unsigned char)(1 << (7 - i % 8));
        }
        // |= (bitField.testBit(i) << (7 - i % 8));
    }

    for(int i = 0; i < data.size(); i++)
    {
        qDebug() << (unsigned char)data.at(i);
    }
    //qDebug() << data;
    return;

    char szD[4] = {0};
    toNetworkData(33, szD);
    qToBigEndian<quint32>(33, szD);

    PEER_WIRE_MSG msg;
    msg.bytesPayload = (unsigned char*)malloc(sizeof(33));
    qToBigEndian<quint32>(33, msg.bytesPayload);

    qDebug() << (int)szD[3] << (int)szD[2] << (int)szD[1] << (int)szD[0] << msg.bytesPayload[3];
    return;

    QUrl url = QUrl(m_pBenCodePrase->getAnnounceUrl());
    QUrlQuery query(url);
    bool bCompleted = false;

    //添加infoHash
    m_bytesInfoHash = QCryptographicHash::hash(m_pBenCodePrase->getInfoSection(), QCryptographicHash::Sha1);
    query.addQueryItem("info_hash", m_bytesInfoHash.toPercentEncoding());
    //添加peer_id
    query.addQueryItem("peer_id", getPeerId());
    //添加port
    query.addQueryItem("port", QByteArray::number(PeerServer::instance()->getListenPort()));
    //接受压缩过的地址
    query.addQueryItem("compact", "1");

    //每次需要变化
    /////////////////////////////////////////////////////
    //上传数:客户端已经上传的总量(从客户端发送’started’事件到Tracker算起)，以十进制ASCII表示。
    query.addQueryItem("uploaded", QByteArray::number(0));
    //下载数:已下载的字节总量(从客户端发送’started’事件到Tracker算起)，以十进制ASCII表示。
    query.addQueryItem("downloaded", QByteArray::number(0));
    //剩余数:客户端还没有下载的字节数，以十进制ASCII表示。
    query.addQueryItem("left", QByteArray::number(0));

    //事件,为started(第一个发送到Tracker的请求其event值),
    //stopped,如果正常关闭客户端，必须发送改事件到Tracker。
    //completed,完事发送。如果客户端启动之前，已经下载完成的话，则没有必要发送该事件
    //空(不指定).如果一个请求不指定event，表明它只是每隔一定间隔发送的请求。
    if(m_bFirstCommnicateWithTracker)
    {
        //如果剩余的下载字节为0,则m_bFristSeeding
        if("已经下载完")
        {
            m_bCompleted = true;
        }
    }

    if("已经下载完")
        bCompleted = true;
    else
        bCompleted = false;

    //下载完,并且不是第一次下载完则发送completed
    if(!m_bCompleted && bCompleted)//只发一次,完成时候发送
    {
        m_bCompleted = true;
        query.addQueryItem("event", "completed");

        qDebug() << "下载完成(启动后!)";
    }
    else if(m_bFirstCommnicateWithTracker)
    {
        m_bFirstCommnicateWithTracker = false;
        query.addQueryItem("event", "started");
    }
    else if(m_bLastCommnicateWithTracker)
    {
        query.addQueryItem("event", "stopped");
    }
    else
    {
        //event is null
    }

    //如果之前的announce包含一个tracker id，那么当前的请求必须设置该参数。
    if (!m_bytesTrackerId.isEmpty())
        query.addQueryItem("trackerid", m_bytesTrackerId);
    /////////////////////////////////////////////////////

    url.setQuery(query);
    QNetworkRequest req(url);
    m_netManger.get(req);
}

void TrakerCommunicate::setBenCodeParse(BenCodeParser *bencodPrase)
{
    m_pBenCodePrase = bencodPrase;
}

QList<PEER_ADDR> TrakerCommunicate::getPeerList()
{
    return m_listPeers;
}

void TrakerCommunicate::httpFinished(QNetworkReply* reply)
{
    reply->deleteLater();
    if(reply->error())
    {
        qDebug() << QString("网络错误,代码为: %1!").arg(reply->error());
        return;
    }

    //读取Tracker返回值
    QByteArray data = reply->readAll();

    //解析数据
    BenCodeParser parse;
    BenDictionary dict;
    if(!parse.parseTorrentData(data))
    {
        qDebug() << "解析返回值失败!";
        return;
    }

    dict = parse.getDict();
    if(dict.contains("failure reason"))
    {
        qDebug() << dict.value("failure reason");
        return;
    }

    if (dict.contains("warning message"))
    {
        // continue processing
        qDebug() << dict.value("warning message");
    }

    if(dict.contains("trackerId"))
    {
        m_bytesTrackerId = dict.value("tracker id").toByteArray();
    }

    if(dict.contains("interval"))
    {
        if(m_iTimerId != -1)
        {
            killTimer(m_iTimerId);
        }
        m_iTimerId = startTimer(dict.value("interval").toInt() * 1000);
    }

    if(dict.contains("peers"))
    {
        //清空连接队列
        m_listPeers.clear();

        QVariant peers = dict.value("peers");

        //字典模式
        if(peers.type() == QVariant::List)
        {
            QList<QVariant> peerTmp = peers.toList();
            for (int i = 0; i < peerTmp.size(); i++)
            {
                PEER_ADDR tmpAddr;
                BenDictionary peer = qvariant_cast<BenDictionary>(peerTmp.at(i));
                tmpAddr.bPeerId = peer.value("peer id").toByteArray();
                tmpAddr.stPeerAddr.setAddress(QString::fromUtf8(peer.value("ip").toByteArray()));
                tmpAddr.uiPort = peer.value("port").toInt();
                m_listPeers.append(tmpAddr);
            }
        }
        else//二进制模式
        {
            QByteArray peerAddrs = peers.toByteArray();
            for(int i = 0; i < peerAddrs.size(); i += 6)
            {
                uchar *p = (uchar*)peerAddrs.constData() + i;
                //大端字节序
                PEER_ADDR tmpAddr;
                tmpAddr.stPeerAddr.setAddress(qFromBigEndian<quint32>(p));
                tmpAddr.uiPort = qFromBigEndian<quint16>(p + 4);
                m_listPeers.append(tmpAddr);
            }
        }
    }
    else
    {
        return;
    }

    for(int i = 0; i < 1; i++)
    {
        PeerClient *client = new PeerClient(this);
        client->init(m_bytesInfoHash, m_bytesPeerId);
        client->connect2Host(m_listPeers.at(i).stPeerAddr, m_listPeers.at(i).uiPort);
    }
}
