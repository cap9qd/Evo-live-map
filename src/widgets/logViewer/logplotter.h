#ifndef LOGPLOTTER_H
#define LOGPLOTTER_H

#include <QMainWindow>
#include <QTimer>
#include "qcustomplot.h"
#include <QElapsedTimer>
//#include <QDialog>

//#include "../../controller.h"
#include "../ecuManager.h"
#include "../../types.h"

using namespace std;

QT_BEGIN_NAMESPACE
namespace Ui { class LogPlotter; }
QT_END_NAMESPACE

class LogPlotter : public QWidget
{
    Q_OBJECT

public:
    LogPlotter(QWidget *parent = nullptr, ecuManager *ecu_manager = nullptr);
    ~LogPlotter();


private slots:
    void realtimeDataSlot();
    void on_btn_pause_clicked();
    void on_rate_sb_valueChanged(double arg1);
    void on_span_sb_valueChanged(double arg1);
    void on_span_sb_editingFinished();
    void on_rate_sb_editingFinished();


    void on_pb_frcEcuId_clicked();

    void on_pb_spanApply_clicked();

    void on_pb_rateApply_clicked();

    void on_hsb_xRange_sliderReleased();

    void on_hsb_xRange_sliderPressed();
    void showPlot(int state);
    void setupWindow();

//From ecuManager
public slots:
    void logReady(QVector<float> scaledValues);
    void ecuConnected();
    void ecuDisconnect();

private slots:
    void horzScrollBarChanged(int value);

signals:
    void Log(QString);

private:
    Ui::LogPlotter *ui;
    bool widgetDebug = false;
    void configure();
    void forceTest();
    double scaleDouble(double in, double iMin, double iMax, double oMin, double oMax);
    void updatePause();

private:
    QTimer dataTimer;
    QVector<QCheckBox*> plotVisibleCB;
    QVector<QLCDNumber*> lcdNumbers;
    double refreshHz = 10.0;
    bool pauseUpdate = true;
    double xSpan = 30.0;
    bool plotReady = false;
    QVector<mutParam> *ecuDef_ramMut;
    QElapsedTimer plotTimer;
    double lastKey;
    double firstKey;

};



#endif // LOGPLOTTER_H
