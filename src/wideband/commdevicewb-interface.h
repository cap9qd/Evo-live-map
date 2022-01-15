#ifndef COMMDEVICEWB_INTERFACE_H
#define COMMDEVICEWB_INTERFACE_H

#include <QString>

class commDeviceWB
{
public:
    QString DeviceDesc;// + " / " + cdWB->DeviceUniqueID
    explicit commDeviceWB();

    virtual bool isClosed() = 0;
    virtual bool openWB(uint baudRate) = 0;

    virtual bool connectWB(unsigned int baudRate) = 0;
    virtual bool closeWB() = 0;
    virtual QByteArray readWB() = 0;

};

#endif // COMMDEVICEWB_INTERFACE_H
