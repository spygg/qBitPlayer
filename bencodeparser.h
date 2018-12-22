#ifndef BENCODEPARSER_H
#define BENCODEPARSER_H

#include <QVariant>
#include <QFile>
#include <QDateTime>
#include <QDebug>
#include <QSettings>
#include <QFileInfo>

typedef QMap<QByteArray,QVariant> BenDictionary;
Q_DECLARE_METATYPE(BenDictionary)

//单个文件必选字段
typedef struct _SINGLE_FILE_INFO
{
    qint64 length;
    QString path;
}SINGLE_FILE_INFO;

//文件信息,可以包含多个单个文件
typedef struct _FILE_INFO
{
    qint64 iPieceLength;
    QByteArray bytesPieces;
    QString szFolderName;

    QList <SINGLE_FILE_INFO> lFileList;
}FILE_INFO;




class BenCodeParser
{
public:
    explicit BenCodeParser();

public:
    bool setTorrentFile(QString szFileName);
    bool parseTorrentData(const QByteArray data);
    BenDictionary getDict();
    QByteArray getInfoSection();
    QString getErrorString();

    QString getAnnounceUrl();
    QStringList getAnnounceListUrls();
    bool getFileInfoSection(FILE_INFO *fileInfoSection);
    QDateTime getCreateTime();

private:
    bool parseDictionary(BenDictionary *dict);
    bool parseInteger(qint64 *iInterger);
    bool parseString(QByteArray *bString);
    bool parseList(QList<QVariant> *list);

private:
    BenDictionary m_dictTorrent;
    qint64 m_iDataLen;
    qint64 m_iCurrentIndex;
    QByteArray m_bytesData;
    QString m_szErrorString;

    qint64 m_iInfoSectionStart;
    qint64 m_iInfoSectionLength;
};

#endif // BENCODEPARSER_H
