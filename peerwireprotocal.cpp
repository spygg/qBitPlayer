#include "peerwireprotocal.h"

PeerWireProtocal::PeerWireProtocal(QObject *parent) : QObject(parent)
{

}

QByteArray PeerWireProtocal::handShake()
{
    //握手： <pstrlen><pstr><reserved><info_hash><peer_id>
    QByteArray data;
    data.append(19);
    data.append("BitTorrent protocol");
    data.append("00000000");


    return data;
}
