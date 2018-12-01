#ifndef CTRAKERNETWORK_H
#define CTRAKERNETWORK_H

#include <QObject>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QEventLoop>
#include <QFile>
#include <QCryptographicHash>
#include <QHostAddress>
#include <QtEndian>
#include "bencodeparser.h"

typedef struct _PEER_ADDR{
    quint32 uiIp;
    quint16 uiPort;

    _PEER_ADDR()
    {
        uiIp = 0;
        uiPort = 0;
    }
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
    void commnicateTracker();
    void setBenCodeParse(BenCodeParser *bencodPrase);

private:
    QNetworkAccessManager m_netManger;
    BenCodeParser *m_pBenCodePrase;
    QByteArray m_bPeerId;

private:
    QByteArray getPeerId();

    QList <PEER_ADDR> m_listPeers;
};

#endif // CTRAKERNETWORK_H
