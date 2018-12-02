#ifndef PEERWIREPROTOCAL_H
#define PEERWIREPROTOCAL_H

#include <QObject>

//与peer交换数据的协议

class PeerWireProtocal : public QObject
{
    Q_OBJECT
public:
    explicit PeerWireProtocal(QObject *parent = nullptr);

    QByteArray handShake();
signals:

public slots:


private:
    QByteArray m_bytesInfoHash;
    QByteArray m_bytesClientId;
};

#endif // PEERWIREPROTOCAL_H
