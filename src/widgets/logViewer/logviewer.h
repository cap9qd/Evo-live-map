#ifndef LOGVIEWER_H
#define LOGVIEWER_H

#include "../ecuManager.h"
#include "../../types.h"
#include "logplotter.h"

class LogViewer : public QWidget
{
    Q_OBJECT

public:
    LogViewer(QWidget *parent = nullptr, ecuManager *ecu_manager = nullptr);
    ~LogViewer();

private:
    LogPlotter *logWin;
    void showWin();
};
#endif // LOGVIEWER_H
