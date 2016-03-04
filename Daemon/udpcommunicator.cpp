#include "udpcommunicator.h"
#include <QTcpSocket>

UdpCommunicator::UdpCommunicator(QObject *parent) :
    QObject(parent),
    port(1221),
    blockSize(0)
{
}

void UdpCommunicator::listen()
{
    tcpServer = new QTcpServer(this);
    if (!tcpServer->listen(QHostAddress::Any, port)) {
        qDebug()<<"Unable to start the server: " << tcpServer->errorString();
        tcpServer->close();
    }
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(bindSocket()));
}

void UdpCommunicator::setPort(int numPort)
{
    port = numPort;
}

void UdpCommunicator::bindSocket()
{
    QTcpSocket *bufferSocket = tcpServer->nextPendingConnection();
    hostAddr = bufferSocket->peerAddress();
    qDebug() << "Get new connection " << hostAddr.toString();
    bufferSocket->deleteLater();
    udpSocket = new QUdpSocket(this);
    udpSocket->bind(hostAddr, port);
    blockSize = 0;
    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(readNextDatagram()));
    connect(udpSocket, SIGNAL(disconnected()), this, SLOT(abortConnection()));
    emit newConnection();
}

void UdpCommunicator::abortConnection()
{
    udpSocket->disconnectFromHost();
    emit lostConnection();
}

void UdpCommunicator::send(QString message)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_8);
    out << (quint16)0;
    out << message;
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));
    udpSocket->writeDatagram(block, hostAddr, port);
}

void UdpCommunicator::readNextDatagram()
{
    while (udpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(), datagram.size());
        QString message(datagram.data());
        qDebug() << message;
        emit recieveMessage(message);
    }
    /*QDataStream in(udpSocket);
    in.setVersion(QDataStream::Qt_4_0);
    QString message;

    for (;;)
    {
        if (!blockSize)
        {
            if (udpSocket->bytesAvailable() < sizeof(quint16))
            {
                break;
            }
            in >> blockSize;
        }
        if (udpSocket->bytesAvailable() < blockSize)
        {
            break;
        }
        in >> message;
        blockSize = 0;
        emit recieveMessage(message);
    }*/
}
