#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include "trakercommunicate.h"

#include <QObject>

class DownloadManager : public QObject
{
    Q_OBJECT
public:
    explicit DownloadManager(QObject *parent = nullptr);

    void setTorrentFile(QString szFilePath);
signals:

public slots:
};

#endif // DOWNLOADMANAGER_H
