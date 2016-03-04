#include "communicator.h"
#include <QDataStream>

Communicator::Communicator() :
    mBlockSize(0),
    hiMessage(true)
{
    mSocket = new QTczpSocket(this);
    connect(mSocket, &QTcpSocket::readyRead, this, &Communicator::read);
    connect(mSocket, &QTcpSocket::connected, this, &Communicator::setConnection);
    connect(mSocket, &QTcpSocket::disconnected, this, &Communicator::abortConnection);
}

void Communicator::setConnection()
{
    emit newConnection();
    qDebug() << "Connecting to " << mIp;
}

void Communicator::abortConnection()
{
    emit lostConnection();
}

int Communicator::connectedState()
{
    return mSocket->ConnectedState;
}

void Communicator::connectToHost()
{
    mBlockSize = 0;
    mSocket->abort();
    mSocket->connectToHost(mIp, mPort);
    qDebug() << "Connected";
}

bool Communicator::isConnected()
{
    return (connectedState() == QTcpSocket::ConnectedState);
}

void Communicator::send(QString message)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_8);
    out << (quint16)0;
    out << message;
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));
    mSocket->write(block);
    qDebug() << "Message sent" << message;
}

void Communicator::read()
{
    if (hiMessage)
    {
        hiMessage = false;
        qDebug() << hiMessage;
        return;
    }

    QDataStream in(mSocket);
    in.setVersion(QDataStream::Qt_4_0);
    QString message;

    for (;;)
    {
        if (!mBlockSize)
        {
            if (mSocket->bytesAvailable() < sizeof(quint16))
            {
                break;
            }
            in >> mBlockSize;
        }
        if (mSocket->bytesAvailable() < mBlockSize)
        {
            break;
        }
        in >> message;
        mBlockSize = 0;
        qDebug() << message;
        emit recieveMessage(message);
    }
}
