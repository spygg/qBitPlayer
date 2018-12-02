#ifndef CTRAKERNETWORK_H
#define CTRAKERNETWORK_H

#include <QObject>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QCryptographicHash>
#include <QtEndian>
#include <QHostAddress>
#include <QTimerEvent>
#include "bencodeparser.h"


//#include <QEventLoop>
//#include <QFile>
#include <QTcpSocket>


typedef struct _PEER_ADDR{
    QByteArray bPeerId;
    QHostAddress stPeerAddr;
    quint16 uiPort;
}PEER_ADDR;

class TrakerCommunicate : public QObject
{
    Q_OBJECT
public:
    explicit TrakerCommunicate(QObject *parent = nullptr);

signals:

public slots:

private slots:
    void httpFinished(QNetworkReply*);

public:
    void commnicateWithTracker();
    void setBenCodeParse(BenCodeParser *bencodPrase);
    QList <PEER_ADDR> getPeerList();

private:
    QByteArray getPeerId();
    void timerEvent(QTimerEvent *event);

private:
    QNetworkAccessManager m_netManger;
    BenCodeParser *m_pBenCodePrase;
    QByteArray m_bytesPeerId;
    QList <PEER_ADDR> m_listPeers;
    QByteArray m_bytesTrackerId;
    int m_iTimerId;
    bool m_bFirstCommnicateWithTracker;
    bool m_bLastCommnicateWithTracker;
    bool m_bCompleted;
};

#endif // CTRAKERNETWORK_H
