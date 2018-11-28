#ifndef CTRAKERNETWORK_H
#define CTRAKERNETWORK_H

#include <QObject>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QEventLoop>
#include <QFile>
#include <QCryptographicHash>

#include "bencodeparser.h"

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
};

#endif // CTRAKERNETWORK_H
