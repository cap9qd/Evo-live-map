#ifndef jcsbanksDMA_H
#define jcsbanksDMA_H

#include <QObject>
#include <QDebug>

#include "ECU-interface.h"

class jcsbanksDMA : public ECU_interface
{
    Q_OBJECT
public:
    explicit jcsbanksDMA(comm_device_interface **devComm);
    ~jcsbanksDMA();

public slots:
    bool connect();
    QByteArray indirectDMAread(quint32 addr, int lenght);
    QByteArray directDMAread(quint32 addr, int lenght);
    void directDMAwrite(quint32 addr, char *buf, int lenght);

private slots:

private:
    int delay_after_command = 5;

private:
    void sendDMAcomand(char command, unsigned long addr, unsigned long count, char* buf = nullptr);

    void setHeader(DMAcomand command, uchar count, quint32 addr);

    void getChckSmm();


signals:
    //void readyInterface(bool);
    //void readyRead(QByteArray);

    //void ECUready();
    //void AFR(float);
    //void Log(QString);
};

#endif // jcsbanksDMA_H
