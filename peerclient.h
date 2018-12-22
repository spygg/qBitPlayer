#ifndef PEERCOMMUNICATE_H
#define PEERCOMMUNICATE_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QtEndian>
#include <QBitArray>

typedef struct _PEER_WIRE_MSG
{
    quint32 iLen;
    unsigned char byteMsgId;
    QByteArray bytesData;
}PEER_WIRE_MSG;

enum eMsgType{
    eChoke,
    eUnchoke,
    eInterested,
    eNotInterested,
    eHave,
    eBitfield = 5,
    eRequest,
    ePiece,
    eCancel,
    ePort
};

//本类用来完成发送信息给peer
//使用的协议为peerWire 协议
class PeerClient : public QTcpSocket
{
    Q_OBJECT

public:
    PeerClient(QObject *parent = nullptr);
    void init(QByteArray bytesInfoHash, QByteArray bytesClientId);

    void connect2Host(QHostAddress stAddr, qint16 uiPort);

private slots:
    void handShake();
    void readyReadSlot();

private:
    QByteArray m_bytesInfoHash;
    QByteArray m_bytesClientId;
    bool m_bSendShake;
    bool m_bRecvivedHandShake;
    qint32 m_iNextPackLen;
private:
//    void sendData();
    void sendKeepAlive();
    void sendChoke();
    void sendUnchoke();
    void sendInterested();
    void sendNotInterested();
    void sendHave(quint32 uiIndex);
    void sendBitfield(QBitArray bitField);
    void sendRequest(quint32 uiIndex, quint32 uiBegin, qint32 uiLength);
    void sendBlock(quint32 uiIndex, quint32 uiBegin, QByteArray bytesBlcok);
    void sendCancel(quint32 uiIndex, quint32 uiBegin, qint32 uiLength);
    void sendPort(quint16 uiPort);

};

#endif // PEERCOMMUNICATE_H
