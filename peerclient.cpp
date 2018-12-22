#include "peerclient.h"

PeerClient::PeerClient(QObject *parent):QTcpSocket(parent)
{
    m_bSendShake = false;
    m_bRecvivedHandShake = false;
    m_iNextPackLen = -1;
}

void PeerClient::init(QByteArray bytesInfoHash, QByteArray bytesClientId)
{
    m_bytesInfoHash = bytesInfoHash;
    m_bytesClientId = bytesClientId;
}

void PeerClient::connect2Host(QHostAddress stAddr, qint16 uiPort)
{
    connect(this, SIGNAL(connected()), this, SLOT(handShake()));
    connect(this, SIGNAL(readyRead()), this, SLOT(readyReadSlot()));
    connectToHost(stAddr, uiPort);
}

//握手消息,长度为68
void PeerClient::handShake()
{
    if(m_bSendShake)
    {
        qDebug() << "已经发送过握手消息!";
        return;
    }

    //握手： <pstrlen><pstr><reserved><info_hash><peer_id>
    //注意: 其中的info_hash和发送给Tracker的不同,这里为原始不%号化的原始值
    QByteArray data;
    data.append(19);
    data.append("BitTorrent protocol");
    data.append(8, 0);

    data.append(m_bytesInfoHash);
    data.append(m_bytesClientId);

    qDebug() << "发送握手消息" << data << data.size();
    write(data);

    m_bSendShake = true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
/*
 * <length prefix><message ID><payload>。
 * length prefix(长度前缀)是一个4字节的大端(big-endian)值。message ID是单个十进制值。playload与消息相关
 */
/////////////////////////////////////////////////////////////////////////////////////////////////


// Sends a "keep-alive" message to prevent the peer from closing
// the connection when there's no activity
void PeerClient::sendKeepAlive()
{
    const char message[] = {0, 0, 0, 0};
    write(message, sizeof(message));
}

void PeerClient::sendChoke()
{
    //choke: <len=0001><id=0>
    const char message[5] = {0, 0, 0, 1};
    write(message, sizeof(message));
}

void PeerClient::sendUnchoke()
{
    //Unchoke: <len=0001><id=1>
    const char message[5] = {0, 0, 0, 1, 1};
    write(message, sizeof(message));
}

void PeerClient::sendInterested()
{
    //Interested: <len=0001><id=2>
    const char message[5] = {0, 0, 0, 1, 2};
    write(message, sizeof(message));
}

void PeerClient::sendNotInterested()
{
    //NotInterested: <len=0001><id=3>
    const char message[5] = {0, 0, 0, 1, 3};
    write(message, sizeof(message));
}

/*
 * The 'have' message's payload is a single number, the index which that downloader just completed
 *  and checked the hash of.
*/
void PeerClient::sendHave(quint32 uiIndex)
{
    //have: <len=0005><id=4><piece index>
    char message[9] = {0, 0, 0, 5, 4};

    qToBigEndian<quint32>(uiIndex, &message[5]);
    write(message, sizeof(message));
}

/*
 * 'bitfield' is only ever sent as the first message. Its payload is a bitfield with each index that
 * downloader has sent set to one and the rest set to zero. Downloaders which don't have anything yet
 * may skip the 'bitfield' message. The first byte of the bitfield corresponds to indices 0 - 7 from high bit to low bit,
 * respectively. The next one 8-15, etc. Spare bits at the end are set to zero
*/
void PeerClient::sendBitfield(QBitArray bitField)
{
    //bitfield: <len=0001+X><id=5><bitfield>
    if (bitField.count(true) == 0)
        return;

    //不足一个字节的补0
    int size = (bitField.size() + 7) / 8;
    QByteArray data(size, '\0');

    char message[] = {0, 0, 0, 0, 5};
    qToBigEndian<quint32>(size + 1, &message[0]);

    //first byte of the bitfield corresponds to indices 0 - 7 from high bit to low bit
    for(int i = 0; i < bitField.size(); i++)
    {
        if(bitField.testBit(i))
        {
            data[i / 8] = data.at(i / 8) | (1 << (7 - i % 8));
        }
    }

    write(message, sizeof(message));
    write(data);
}

/* index是piece的索引，begin是piece内的偏移，length是请求peer发送的数据的长度。当客户端收到某个peer发来的unchoke消息后，即构造request消息，
 * 向该peer发送数据请求。前面提到，peer之间交换数据是以slice（长度为16KB的块）为单位的，
 * 因此request消息中length的值一般为16K。对于一个256KB的piece，客户端分16次下载，每次下载一个16K的slice。
 *
 *  每一次请求是一个块
 *  每一次请求是一个块
 *
 * 'request' messages contain an index, begin, and length. The last two are byte offsets.
 *  Length is generally a power of two unless it gets truncated by the end of the file.
 *  All current implementations use 2^14 (16 kiB), and close connections which request an amount greater than that
 */
void PeerClient::sendRequest(quint32 uiIndex, quint32 uiBegin, qint32 uiLength)
{
    // request: <len=0013><id=6><index><begin><length>
    char message[17] = {0, 0, 0, 13, 6};

    qToBigEndian<quint32>(uiIndex, &message[5]);
    qToBigEndian<quint32>(uiBegin, &message[9]);
    qToBigEndian<quint32>(uiLength, &message[13]);
    write(message, sizeof(message));
}

/*
 * 注意原始规范在描述⽤⼾协议时也使⽤术语“⽚断”，但与元信息⽂件中的术语“⽚断”不同。
 * 由于该原因，术语“块”将在本规范中⽤来描述⽤⼾之间通过线路交换的数据
 * piece' messages contain an index, begin, and piece. Note that they are correlated with request messages implicitly.
 *  It's possible for an unexpected piece to arrive if choke and unchoke messages are sent in quick succession
 * and/or transfer is going very slowly
*/
void PeerClient::sendBlock(quint32 uiIndex, quint32 uiBegin, QByteArray bytesBlcok)
{
    // piece: <len=0009+X><id=7><index><begin><block>
    char message[13] = {0, 0, 0, 0, 7};
    qToBigEndian<quint32>(9 + bytesBlcok.size(), &message[0]);
    write(message, sizeof(message));

    qToBigEndian<quint32>(uiIndex, &message[5]);
    qToBigEndian<quint32>(uiBegin, &message[9]);

    write(bytesBlcok);
}

/*
 * 'cancel' messages have the same payload as request messages. They are generally only sent towards the end of a download,
 * during what's called 'endgame mode'. When a download is almost complete, there's a tendency for the last few pieces
 * to all be downloaded off a single hosed modem line, taking a very long time. To make sure the last few pieces
 * come in quickly, once requests for all pieces a given downloader doesn't have yet are currently pending,
 * it sends requests for everything to everyone it's downloading from. To keep this from becoming horribly inefficient,
 * it sends cancels to everyone else every time a piece arrives.
*/
void PeerClient::sendCancel(quint32 uiIndex, quint32 uiBegin, qint32 uiLength)
{
    // cancel: <len=0013><id<=8><index><begin><length>
    char message[17] = {0, 0, 0, 13, 8};

    qToBigEndian<quint32>(uiIndex, &message[5]);
    qToBigEndian<quint32>(uiBegin, &message[9]);
    qToBigEndian<quint32>(uiLength, &message[13]);
    write(message, sizeof(message));
}

void PeerClient::sendPort(quint16 uiPort)
{
    // port: <len=0003><id=9><listen-port>
    char message[7] = {0, 0, 0, 3, 9, 0, 0};

    qToBigEndian<quint16>(uiPort, &message[5]);
    write(message, sizeof(message));
}

void PeerClient::readyReadSlot()
{
    if(!m_bRecvivedHandShake)
    {
        if(bytesAvailable() >= 68)
        {
            qDebug() << read(68);
//            int iLen = read(1).toInt();
//            qDebug() << iLen << read(iLen) << read(8);

//            qDebug() << "info_hash:" << read(20) << "peerId:" << read(20);
            m_bRecvivedHandShake = true;
        }

    }
    else
    {
        //<4字节⻓前缀><消息标识符><有效负载>
        //qDebug() << qFromBigEndian<qint32>(data) << "消息ID" << (quint8)data.at(4) << data;
        if(bytesAvailable() < 4)
        {
            return;
        }

        if(m_iNextPackLen == -1)
        {
            char szLen[4] = {0};
            read(szLen, sizeof(szLen));
            m_iNextPackLen = qFromBigEndian<qint32>(szLen);
        }

        if(bytesAvailable() < m_iNextPackLen)
        {
            return;
        }

        QByteArray data = read(m_iNextPackLen);

        m_iNextPackLen = -1;

        switch (data.at(0)) {
        case eChoke:
            {
                qDebug() << "接收到消息:eChoke";
            }
            break;
        case eUnchoke:
            {
                qDebug() << "接收到消息:eUnchoke";
            }
            break;
        case eInterested:
            {
                qDebug() << "接收到消息:eInterested";
            }
            break;
        case eNotInterested:
            {
                qDebug() << "接收到消息:eNotInterested";
            }
            break;
        case eHave:
            {
                qDebug() << "接收到消息:eHave";
            }
            break;
        case eBitfield:
            {
                qDebug() << "接收到消息:eBitfield";
            }
            break;
        case eRequest:
            {
                qDebug() << "接收到消息:eRequest";
            }
            break;
        case ePiece:
            {
                qDebug() << "接收到消息:ePiece";
            }
            break;
        case eCancel:
            {
                qDebug() << "接收到消息:eCancel";
            }
            break;
        case ePort:
            {
                qDebug() << "接收到消息:ePort";
            }
            break;
        default:
            break;
        }
    }


}
