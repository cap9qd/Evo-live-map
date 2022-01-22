#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//#include <time.h>
#include <QMainWindow>
#include <QPushButton>
#include <QTimer>
#include <QFileDialog>
#include <QLCDNumber>
#include <QTreeWidgetItem>

//#include <QMessageBox>
#include <QStatusBar>



#include "types.h"

//#include "comm-device-interface/comm-device-interface.h"
//#include "comm-device-interface/op13.h"
//#include "comm-device-interface/op20.h"

#include "deviceNativeFilter.h"

#include "widgets/commParamWidget.h"
#include "widgets/mapWidget/mapwidget.h"
#include "widgets/hexEditor/qhexedit/qhexedit.h"
#include "widgets/hexEditor/hexeditor.h"
#include "widgets/gauge_widget.h"


#include "widgets/ecuManager.h"
#include "comm-device-interface/devicemanager.h"
#include "DMA-proto/proto-manager.h"
#include "wideband/wb-manager.h"


#include "widgets/logViewer/logviewer.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QHash<QString, mapWidget*> ptrRAMtables;

    ~MainWindow();
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *event);

public slots:
    void setUSBfilter(deviceNativeFilter *usbFilter);
    void Log(QString str);

private slots:
    void deviceEvent(comm_device_interface *devComm);
    void ecu_connected();
    void disConnectECUaction();
    void createMap(mapDefinition *dMap);
    void itemChecks(QTreeWidgetItem *item, int column);

private:
    Ui::MainWindow *ui;

    void setCPW();
    void createMapTree(Map *tab);
    void freeMapTree();

    void colorFromFile(QString filename);

    //======================== widget's =================================
    ecuManager _ecuManager;
    commParamWidget cpW;
    hexEditor *hexEdit;
    gaugeWidget wbWgt{"           = Wideband =           ", 4};

    //======================== widget lists =================================
    QSet<gaugeWidget*> gauge_widget_set;
    QToolBar *loggerWidgetBar = nullptr;
    QVector<QColor> colormap;
    void create_gauge(QString name, mutParam *param);
    void gaugeDelete();

signals:
    void _exit();

};
#endif // MAINWINDOW_H
