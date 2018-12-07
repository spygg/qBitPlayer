#include "peerclient.h"

PeerClient::PeerClient(QObject *parent):QTcpSocket(parent)
{
    m_bShaked = false;
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

//握手消息
void PeerClient::handShake()
{
    //握手： <pstrlen><pstr><reserved><info_hash><peer_id>
    //注意: 其中的info_hash和发送给Tracker的不同,这里为原始不%号化的原始值
    QByteArray data;
    data.append(19);
    data.append("BitTorrent protocol");
    data.append("00000000");

    data.append(m_bytesInfoHash);
    data.append(m_bytesClientId);

    qDebug() << "发送握手消息" << data.size() << m_bytesClientId.size() << m_bytesInfoHash.size();
    write(data);
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

void PeerClient::sendHave(quint32 uiIndex)
{
    //have: <len=0005><id=4><piece index>
    char message[9] = {0, 0, 0, 5, 4};

    qToBigEndian<quint32>(uiIndex, &message[5]);
    write(message, sizeof(message));
}

void PeerClient::sendBitfield(QBitArray bitField)
{
    //bitfield: <len=0001+X><id=5><bitfield>
    if (bitField.count(true) == 0)
        return;

    //不足一个字节的补0
    int size = (bitField.size() + 7) / 8;
    QByteArray data(size, '\0');

    char message[] = {0, 0, 0, 1, 5};
    qToBigEndian<quint32>(bitField.size() + 1, &message[0]);

    //first byte of the bitfield corresponds to indices 0 - 7 from high bit to low bit
    for(int i = 0; i < bitField.size(); i++)
    {
        if(bitField.testBit(i))
        {
            data[i / 8] = data.at(i / 8) | (uchar)(1 << (7 - i % 8));
        }
    }

    write(message, sizeof(message));
    write(data);
}

void PeerClient::sendRequest(quint32 uiIndex, quint32 uiBegin, qint32 uiLength)
{
    // request: <len=0013><id=6><index><begin><length>
    char message[17] = {0, 0, 0, 13, 6};

    qToBigEndian<quint32>(uiIndex, &message[5]);
    qToBigEndian<quint32>(uiBegin, &message[9]);
    qToBigEndian<quint32>(uiLength, &message[13]);
    write(message, sizeof(message));
}

void PeerClient::sendPiece(quint32 uiIndex, quint32 uiBegin, qint32 uiBlock)
{
    // piece: <len=0009+X><id=7><index><begin><block>
    char message[] = {0, 0, 0, 0, 7};
    qToBigEndian<quint32>(9 + 1, &message[0]);
//    qToBigEndian<quint32>(uiIndex, &message[5]);
//    qToBigEndian<quint32>(uiBegin, &message[9]);
//    qToBigEndian<quint32>(uiLength, &message[13]);
    write(message, sizeof(message));


}

void PeerClient::sendCancel(quint32 uiIndex, quint32 uiBegin, qint32 uiLength)
{
    // cancel: <len=0013><id<=8><index><begin><length>
    char message[17] = {0, 0, 0, 13, 8};

    qToBigEndian<quint32>(uiIndex, &message[5]);
    qToBigEndian<quint32>(uiBegin, &message[9]);
    qToBigEndian<quint32>(uiLength, &message[13]);
    write(message, sizeof(message));
}

void PeerClient::readyReadSlot()
{
    qDebug() << this->readAll();
}
