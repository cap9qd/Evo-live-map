#include "logviewer.h"
#include "ui_logviewer.h"

LogViewer::LogViewer(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LogViewer)
{
    ui->setupUi(this);

    ui->statusBar->setText("");

    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
    if(widgetDebug) {
        configure();
        connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
        dataTimer.start(1000.0/refreshHz); // Interval 0 means to refresh as fast as possible
    } else {}
}

void LogViewer::realtimeDataSlot()
{
    static QTime time(QTime::currentTime());
    // calculate two new data points:
    double key = time.elapsed()/1000.0; // time elapsed since start of demo, in seconds
    static double lastPointKey = 0;
    static long totalSize = 0;
    // calculate frames per second:
    static double lastFpsKey;
    static int frameCount;

    if (!plotReady)
        return;

    if (key-lastPointKey > (0.002)) // at most add point every 2 ms
    {
        std::uniform_real_distribution<double> dist(0.001, 1.0);

        QCPRange xRange = ui->plot->xAxis->range();
        //qDebug() << tr("Current Plot X-Axis Range: [%1,%2] | Size: %3 sec").arg(xRange.lower).arg(xRange.upper).arg(xRange.upper-xRange.lower);
        // add data to lines:
        totalSize = 0;
        for(int i = 0; i < maxPlots; ++i)
        {
            double randAdd = dist(*QRandomGenerator::global());
            ui->plot->graph(i)->addData(key, qSin(key)+randAdd*randAmp[i]*qSin(key/randTime[i]));
            if(!pauseUpdate)
            {
                ui->plot->graph(i)->data()->removeBefore(key-(xRange.upper-xRange.lower)-(key-lastPointKey));
                ui->plot->graph(i)->rescaleAxes(true);
            } else {
                ui->plot->graph(i)->data()->removeBefore(key-(xRange.upper-xRange.lower)*10.0);
            }
            totalSize += ui->plot->graph(i)->data()->size();

            //enable/disable based on checkbox...
            ui->plot->graph(i)->setVisible(plotVisibleCB[i]->isChecked());
        }
        //ui->plot->graph(0)->addData(key, qSin(key)+qrand()/(double)RAND_MAX*1*qSin(key/0.3843));
        //ui->plot->graph(1)->addData(key, qCos(key)+qrand()/(double)RAND_MAX*0.5*qSin(key/0.4364));
        // rescale value (vertical) axis to fit the current data:
        //ui->customPlot->graph(0)->rescaleValueAxis();
        //ui->customPlot->graph(1)->rescaleValueAxis(true);
        qDebug() << tr("PlotTime = %1 sec (%2 Hz)").arg(key-lastPointKey).arg(1.0/(key-lastPointKey));
        lastPointKey = key;
        ++frameCount;

        // make key axis range scroll with the data (at a constant range size of 8):
        ui->plot->xAxis->setRange(key, xSpan, Qt::AlignRight);

        if(!pauseUpdate)
            ui->plot->replot();

        if (key-lastFpsKey > 2) // average fps over 2 seconds
        {
            ui->statusBar->setText(
                QString("%1 FPS, Total Data points: %2")
                .arg(frameCount/(key-lastFpsKey), 0, 'f', 3)
                .arg(totalSize));
          lastFpsKey = key;
          frameCount = 0;
        }
    }
}

LogViewer::~LogViewer()
{
    delete ui;
    plotReady = false;
}

void LogViewer::on_btn_pause_clicked()
{
    pauseUpdate = !pauseUpdate;
    updatePause();
}

void LogViewer::updatePause()
{
    if(pauseUpdate)
        ui->label_pause->setText("PAUSED");
    else
        ui->label_pause->setText("RUNNING");
}

void LogViewer::on_rate_sb_valueChanged(double arg1)
{}

void LogViewer::on_span_sb_valueChanged(double arg1)
{}

void LogViewer::on_span_sb_editingFinished()
{
    xSpan = ui->span_sb->value();
}


void LogViewer::on_rate_sb_editingFinished()
{
    refreshHz = ui->rate_sb->value();
    if(widgetDebug)
    {
        dataTimer.setInterval(1000.0/refreshHz); // Interval 0 means to refresh as fast as possible
    }
}

void LogViewer::logReady(QVector<float> scaledValues)
{
    static QTime time(QTime::currentTime());
    // calculate two new data points:
    double key = 0.0;
    static double lastPointKey = 0;
    static long totalSize = 0;
    // calculate frames per second:
    static double lastFpsKey;
    static int frameCount;

    if(!plotReady)
        return;

    if(!plotTimer.isValid())
    {
        plotTimer.start();
        return;
    } else {
        key = plotTimer.elapsed()/1000.0;
    }

    if (key-lastPointKey > (1.0/refreshHz))
    {
        QCPRange xRange = ui->plot->xAxis->range();
        totalSize = 0;
        for(int i = 0; i < scaledValues.size(); ++i)
        {
            ui->plot->graph(i)->addData(key, scaledValues.at(i));
            if(!pauseUpdate)
            {
                //Delete data out of view and rescale the axes
                ui->plot->graph(i)->data()->removeBefore(key-(xRange.upper-xRange.lower)-(key-lastPointKey));
                ui->plot->graph(i)->rescaleAxes(true);
            } else {
                //Keep more data just in case we want to do statistics later on the visible data
                ui->plot->graph(i)->data()->removeBefore(key-(xRange.upper-xRange.lower)*10.0);
            }
            totalSize += ui->plot->graph(i)->data()->size();

            //enable/disable based on checkbox...
            ui->plot->graph(i)->setVisible(plotVisibleCB[i]->isChecked());
        }

        qDebug() << tr("PlotTime = %1 sec (%2 Hz)").arg(key-lastPointKey).arg(1.0/(key-lastPointKey));
        lastPointKey = key;
        ++frameCount;

        // make key axis range scroll with the data:
        ui->plot->xAxis->setRange(key, xSpan, Qt::AlignRight);

        if(!pauseUpdate)
            ui->plot->replot();

        if (key-lastFpsKey > 2) // average fps over 2 seconds
        {
            ui->statusBar->setText(
                QString("%1 FPS, Total Data points: %2")
                .arg(frameCount/(key-lastFpsKey), 0, 'f', 3)
                .arg(totalSize));
          lastFpsKey = key;
          frameCount = 0;
        }
    }
}

void LogViewer::ecuRamMut(QVector<mutParam> ramMut)
{
    RAM_MUT = ramMut;
    configureMut();
}

void LogViewer::showWin()
{
    this->show();
}

void LogViewer::configureMut()
{
    QVBoxLayout *gbLayout = new QVBoxLayout();

    for(int i = 0; i < RAM_MUT.size(); ++i)
    {
        ui->plot->addGraph(); // blue line
        //ui->plot->graph(i)->setPen(QPen(QColor(40, 110, 255)));
        ui->plot->graph(i)->setPen(QPen( QColor( qrand() % 256, qrand() % 256, qrand() % 256 )));

        ui->plot->graph(i)->setVisible(true);

        plotVisibleCB[i] = new QCheckBox(tr("%1 (%2)").arg(RAM_MUT.at(i).name).arg(RAM_MUT.at(i).scaling.units));
        plotVisibleCB[i]->setChecked(1);
        plotVisibleCB[i]->setCheckable(1);
        gbLayout->addWidget(plotVisibleCB[i],i);
    }
    ui->gb_params->setLayout(gbLayout);

    qDebug() << "";

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    ui->plot->xAxis->setTicker(timeTicker);
    ui->plot->axisRect()->setupFullAxesBox();
    ui->plot->yAxis->setRange(-1.2, 1.2);

    updatePause();

    ui->gb_params->setMaximumWidth(200);
    ui->gb_params->setMinimumWidth(200);

    ui->span_sb->setMinimum(5.0);
    ui->span_sb->setMaximum(90.0);
    ui->span_sb->setValue(xSpan);

    ui->rate_sb->setMinimum(1.0);
    ui->rate_sb->setMaximum(30.0);
    ui->rate_sb->setValue(refreshHz);

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->plot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->plot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->plot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->plot->yAxis2, SLOT(setRange(QCPRange)));

    plotReady = true;
    pauseUpdate = false;
    updatePause();
}


void LogViewer::configure()
{
    QVBoxLayout *gbLayout = new QVBoxLayout();

    std::uniform_real_distribution<double> distAmp(0.5,2.5);
    std::uniform_real_distribution<double> distTime(0.001,0.5);

    for(int i = 0; i < maxPlots; ++i)
    {
        randAmp[i]  = distAmp(*QRandomGenerator::global());
        randTime[i] = distTime(*QRandomGenerator::global());

        qDebug() << tr("randAmp[%1]  = ").arg(i) << tr("%1").arg(randAmp[i]);
        qDebug() << tr("randTime[%1] = ").arg(i) << tr("%1").arg(randTime[i]);

        ui->plot->addGraph(); // blue line
        //ui->plot->graph(i)->setPen(QPen(QColor(40, 110, 255)));
        ui->plot->graph(i)->setPen(QPen( QColor( qrand() % 256, qrand() % 256, qrand() % 256 )));
        plotVisibleCB[i] = new QCheckBox(tr("PLOT %1").arg(i));
        plotVisibleCB[i]->setChecked(1);
        plotVisibleCB[i]->setCheckable(1);
        gbLayout->addWidget(plotVisibleCB[i],i);
    }
    ui->gb_params->setLayout(gbLayout);

    qDebug() << "";

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    ui->plot->xAxis->setTicker(timeTicker);
    ui->plot->axisRect()->setupFullAxesBox();
    ui->plot->yAxis->setRange(-1.2, 1.2);

    updatePause();

    ui->gb_params->setMaximumWidth(200);
    ui->gb_params->setMinimumWidth(200);

    ui->span_sb->setMinimum(5.0);
    ui->span_sb->setMaximum(90.0);
    ui->span_sb->setValue(xSpan);

    ui->rate_sb->setMinimum(1.0);
    ui->rate_sb->setMaximum(30.0);
    ui->rate_sb->setValue(refreshHz);

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->plot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->plot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->plot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->plot->yAxis2, SLOT(setRange(QCPRange)));

    plotReady = true;
    pauseUpdate = false;
    updatePause();
}
