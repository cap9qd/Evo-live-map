#ifndef LOGVIEWER_H
#define LOGVIEWER_H

#include <QMainWindow>
#include <QTimer>
#include "qcustomplot.h"
#include <QElapsedTimer>
#include <QDialog>

#include "../../controller.h"
#include "../../types.h"

using namespace std;

QT_BEGIN_NAMESPACE
namespace Ui { class LogViewer; }
QT_END_NAMESPACE

class LogViewer : public QDialog
{
    Q_OBJECT

public:
    LogViewer(QWidget *parent = nullptr);
    ~LogViewer();

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

public slots:
    void logReady(QVector<float> scaledValues);
    void ecuRamMut(QVector<mutParam> ramMut);
    void showWin();
    void horzScrollBarChanged(int value);

private:
    void forceTestRamMut();
    enum{maxPlots = 16};
    Ui::LogViewer *ui;
    QTimer dataTimer;
    //double randAmp[maxPlots];
    //double randTime[maxPlots];
    QVector<QCheckBox*> plotVisibleCB;
    QVector<QLCDNumber*> lcdNumbers;
    double refreshHz = 10.0;
    bool pauseUpdate = true;
    void updatePause();
    double xSpan = 30.0;
    //Set to true to run timed test via main.cpp
    bool widgetDebug = false;
    bool plotReady = false;
    QVector<mutParam> RAM_MUT;
    quint32 RAM_MUT_count = 0;
    void configure();
    void configureMut();
    QElapsedTimer plotTimer;
    QString LogViewer::SearchFiles(QString path, QString CalID);
    double scaleDouble(double in, double iMin, double iMax, double oMin, double oMax);
    double lastKey;
    double firstKey;

};
#endif // LOGVIEWER_H
