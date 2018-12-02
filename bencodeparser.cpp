#include "bencodeparser.h"

BenCodeParser::BenCodeParser()
{
    m_iCurrentIndex = 0;
    m_iDataLen = 0;
}

bool BenCodeParser::setTorrentFile(QString szFileName)
{
    QFile file(szFileName);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "file open failed!";
        return false;
    }

    QSettings settings("QBitPlayer.ini", QSettings::IniFormat);
    settings.beginGroup("BitPlayer");
    settings.setValue("fileName", QFileInfo(szFileName).absolutePath());
    settings.endGroup();
    settings.sync();


    return parseTorrentData(file.readAll());
}

bool BenCodeParser::parseInteger(qint64 *iInterger)
{
    if(m_bytesData.at(m_iCurrentIndex) != 'i')
        return false;

    qint64 iDigtial = 0;
    //跳过i
    m_iCurrentIndex++;

    int iSign = 1;
    while(m_iCurrentIndex < m_iDataLen)
    {
        if(m_bytesData.at(m_iCurrentIndex) == 'e')
        {
            m_iCurrentIndex++;
            break;
        }

        //负数时
        if((iSign == 1) && (m_bytesData.at(m_iCurrentIndex) == '-'))
        {
            qDebug() << "negative number";
            iSign = -1;
            m_iCurrentIndex++;

            //-0为异常情况
            if(m_bytesData.at(m_iCurrentIndex) == '0')
            {
                m_szErrorString = QString("Integer Parse Error at %1 cause -0").arg(m_iCurrentIndex);
                qDebug() << m_szErrorString;
                return false;
            }
        }

        if(m_bytesData.at(m_iCurrentIndex) >= '0' && m_bytesData.at(m_iCurrentIndex) <= '9')
            iDigtial = iDigtial * 10 + (m_bytesData.at(m_iCurrentIndex) - '0');
        else
            return false;
        m_iCurrentIndex++;
    }

    *iInterger = iDigtial * iSign;

    return true;
}

bool BenCodeParser::parseString(QByteArray *bString)
{
    qint64 iDigtial = 0;

    while(m_iCurrentIndex < m_iDataLen)
    {
        if(m_bytesData.at(m_iCurrentIndex) == ':')
        {
            m_iCurrentIndex++;
            break;
        }

        if(m_bytesData.at(m_iCurrentIndex) >= '0' && m_bytesData.at(m_iCurrentIndex) <= '9')
            iDigtial = iDigtial * 10 + (m_bytesData.at(m_iCurrentIndex) - '0');
        else
            return false;
        m_iCurrentIndex++;
    }

    *bString = m_bytesData.mid(m_iCurrentIndex, iDigtial);
    m_iCurrentIndex += iDigtial;

    return true;
}

bool BenCodeParser::parseList(QList<QVariant> *list)
{
    if(m_bytesData.at(m_iCurrentIndex) != 'l')
        return false;

    m_iCurrentIndex++;

    QList<QVariant> tempList;

    while(m_iCurrentIndex < m_iDataLen)
    {
        if(m_bytesData.at(m_iCurrentIndex) == 'e')
        {
            m_iCurrentIndex++;
            break;
        }

        QByteArray byteString;
        qint64 iInteger = 0;
        QList <QVariant> temp;
        BenDictionary dict;

        if(parseString(&byteString))
        {
            tempList.append(byteString);
        }
        else if (parseInteger(&iInteger))
        {
            tempList.append(iInteger);
        }
        else if(parseList(&temp))
        {
            tempList.append(temp);
        }
        else if(parseDictionary(&dict))
        {
            tempList.append(QVariant::fromValue<QMap<QByteArray, QVariant> >(dict));
        }
        else
        {
            m_szErrorString = QString("List Parse Error at %1").arg(m_iCurrentIndex);
            qDebug() << m_szErrorString;

            return false;
        }
    }

    *list = tempList;

    return true;
}

bool BenCodeParser::parseDictionary(BenDictionary *dict)
{
    if(m_bytesData.at(m_iCurrentIndex) != 'd')
        return false;

    BenDictionary tempDict;

    m_iCurrentIndex++;

    while(m_iCurrentIndex < m_iDataLen)
    {
        if(m_bytesData.at(m_iCurrentIndex) == 'e')
        {
            m_iCurrentIndex++;
            break;
        }

        QByteArray key;
        if(!parseString(&key))
            return false;

        if(key == "info")
            m_iInfoSectionStart = m_iCurrentIndex;

        QByteArray byteString;
        BenDictionary temp;
        qint64 iInteger = 0;
        QList <QVariant> list;

        if(parseString(&byteString))
        {
            tempDict.insert(key, byteString);
        }
        else if (parseInteger(&iInteger))
        {
            tempDict.insert(key, iInteger);
        }
        else if(parseList(&list))
        {
            tempDict.insert(key, list);
        }
        else if(parseDictionary(&temp))
        {
            tempDict.insert(key, QVariant::fromValue<QMap<QByteArray, QVariant> >(temp));
        }
        else
        {
            m_szErrorString = QString("Dict Parse Error at %1").arg(m_iCurrentIndex);
            qDebug() << m_szErrorString;
        }

        if(key == "info")
            m_iInfoSectionLength = m_iCurrentIndex - m_iInfoSectionStart;

    }

    *dict = tempDict;
    return true;
}

bool BenCodeParser::parseTorrentData(const QByteArray data)
{
    if(data.size() <= 0)
    {
        m_szErrorString = QString("Torrent file is Empty");
        return false;
    }

    m_iCurrentIndex = 0;
    m_iDataLen = data.size();
    m_bytesData = data;
    m_iInfoSectionStart = 0;
    m_iInfoSectionLength = 0;

    return parseDictionary(&m_dictTorrent);
}

BenDictionary BenCodeParser::getDict()
{
    return m_dictTorrent;
}

QByteArray BenCodeParser::getInfoSection()
{
    return m_bytesData.mid(m_iInfoSectionStart, m_iInfoSectionLength);
}

QString BenCodeParser::getErrorString()
{
    return m_szErrorString;
}

QString BenCodeParser::getAnnounceUrl()
{
    assert(m_dictTorrent.contains("announce"));

    return m_dictTorrent.value("announce").toString();
}

QStringList BenCodeParser::getAnnounceListUrls()
{
    if(m_dictTorrent.contains("announce-list"))
        return m_dictTorrent.value("announce-list").toStringList();
    else
        return QStringList();
}

QDateTime BenCodeParser::getCreateTime()
{
    return QDateTime::fromSecsSinceEpoch(m_dictTorrent.value("creation date").toLongLong());
}

bool BenCodeParser::getFileInfoSection(FILE_INFO *fileInfoSection)
{
    if(!m_dictTorrent.contains("info"))
        return false;

    BenDictionary infoDict = qvariant_cast<BenDictionary>(m_dictTorrent.value("info"));
    //Dictionary infoDict = m_dictTorrent.value("info").value<Dictionary>();

    FILE_INFO fileInfo;
    //文件夹名字name
    fileInfo.szFolderName = infoDict.value("name").toString();
    fileInfo.iPieceLength = infoDict.value("iece length").toLongLong();
    fileInfo.bPieces = infoDict.value("pieces").toByteArray();

    if(infoDict.contains("files"))//多文件
    {
        QList <QVariant> fileList = infoDict.value("files").toList();

        for(int i = 0; i < fileList.size(); i++)
        {
            SINGLE_FILE_INFO singleFile;
            BenDictionary fileDict = qvariant_cast<BenDictionary>(fileList.at(i));
            singleFile.length = fileDict.value("length").toLongLong();
            singleFile.path = fileInfo.szFolderName;

            foreach (QString pathElement, fileDict.value("path").toStringList()) {
                singleFile.path += "/";
                singleFile.path += pathElement;
            }
            fileInfo.lFileList.append(singleFile);
        }
    }
    else//单文件
    {
        SINGLE_FILE_INFO singleFile;

        singleFile.length = infoDict.value("length").toLongLong();
        singleFile.path = fileInfo.szFolderName + infoDict.value("path").toString();
        fileInfo.lFileList.append(singleFile);
    }


    if(fileInfoSection)
        *fileInfoSection = fileInfo;

//    for(int i = 0; i < fileInfo.lFileList.size(); i++)
//        qDebug() << fileInfo.lFileList.at(i).path;
    return true;
    //qDebug() << fileInfo.bPieces.size();
    //m_dictTorrent.value("announce-list").toStringList();
}
