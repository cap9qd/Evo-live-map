#ifndef ECU_INTERFACE_H
#define ECU_INTERFACE_H

#include <QApplication>
#include <QObject>
#include <QDebug>
//#include <QTimer>
#include <QThread>
#include <QtEndian>
#include <QElapsedTimer>

#include <src/ecu/ecu-definition.h>

#include "../comm-device-interface/comm-device-interface.h"

#include "pollhelper.h"

enum class DMAcomand
{
    stockWrite,
    stockRead,
    stockInfo,
    directWrite = 0x04,
    directRead,
};

class DMA_proto : public pollHelper
{
    Q_OBJECT
public:
    comm_device_interface **devComm = nullptr;
    ecu_definition _ecu_definition;

    DMA_proto();
    //explicit DMA_proto(comm_device_interface **devComm = nullptr);
    virtual ~DMA_proto();

    void setCommDev(comm_device_interface **devComm = nullptr);
    bool getECU(QString romID);

    virtual bool connect() = 0;

    virtual QByteArray indirectDMAread(quint32 addr, int lenght) = 0;
    virtual QByteArray directDMAread(quint32 addr, int len) = 0;

public slots:
    virtual void directDMAwrite(quint32 addr, char *buf, int lenght) = 0;
    virtual void directDMAwrite(abstractMemoryScaled memory);

    virtual void startLog() = 0;
    virtual void stopLog() = 0;

private slots:
    virtual void poll() = 0;

private:

    //Protocol protocol;
    //ConnectFlag ConnectFlag;
    //uint baudRate;

//signals:
//    void Log(QString);

};

#endif // ECU_INTERFACE_H
