#include "downloadmanager.h"

DownloadManager::DownloadManager(QObject *parent) : QObject(parent)
{

}

void DownloadManager::setTorrentFile(QString szFilePath)
{
    BenCodeParser bencode;
    bencode.setTorrentFile(szFilePath);

    TrakerCommunicate *com = new TrakerCommunicate;
    com->setBenCodeParse(&bencode);
    com->commnicateWithTracker();
    bencode.getFileInfoSection(nullptr);
}
