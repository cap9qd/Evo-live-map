#include "comm-device-interface.h"

//#include "src/test-map.h"


comm_device_interface::comm_device_interface(QString dllName, QString DeviceUniqueID) : QObject(), dllName(dllName), DeviceUniqueID(DeviceUniqueID)
{
    qDebug() << "comm_device_interface";
}

comm_device_interface::~comm_device_interface()
{
    qDebug() << "~comm_device_interface";
}

void comm_device_interface::setBaudRate(int baudRate)
{
    this->baudRate = baudRate;
}