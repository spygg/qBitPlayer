#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    BenCodeParser bencode;
    bencode.setTorrentFile("torrent/ubuntu.torrent");

    TrakerCommunicate *com = new TrakerCommunicate;
    com->setBenCodeParse(&bencode);
    com->commnicateTracker();
    bencode.getFileInfoSection(nullptr);
}

Widget::~Widget()
{
    delete ui;
}
