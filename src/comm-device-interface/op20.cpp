#include "op20.h"

OP20::OP20(QObject *parent, QString dllName, QString DeviceDesc, QString DeviceUniqueID) : j2534_interface(parent, dllName, DeviceDesc, DeviceUniqueID)
{
}

OP20::~OP20()
{
}
