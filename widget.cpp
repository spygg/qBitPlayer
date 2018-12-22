#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    m_pDownloadMan = new DownloadManager(this);

    m_pDownloadMan->setTorrentFile("torrent/ubuntu.torrent");
}

Widget::~Widget()
{
    delete ui;
}
