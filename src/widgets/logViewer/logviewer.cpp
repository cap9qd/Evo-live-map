#include "logviewer.h"
#include "ui_logviewer.h"

LogViewer::LogViewer(QWidget *parent, ecuManager *ecu_manager)
    : QDialog(parent)
    , ui(new Ui::LogViewer)
{

    connect(ecu_manager, &ecuManager::logReady,     this, &LogViewer::logReady);
    connect(ecu_manager, &ecuManager::setRamMut,    this, &LogViewer::ecuRamMut);
    connect(ecu_manager, &ecuManager::ecuConnected, this, &LogViewer::ecuConnected);

    /*
    //=============
    // for menubox
    //------------
    QHBoxLayout *layout = new QHBoxLayout;
    QPushButton *pb = new QPushButton("LogView");
    layout->addWidget(pb,0);
    setLayout(layout);
    setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );
    resize(200, 200);
    connect(pb, &QPushButton::clicked, this, &LogViewer::setupWindow);
    //---------
    //===========
    */

    menuButton = new QPushButton("LogView");
    connect(menuButton, &QPushButton::clicked, this, &LogViewer::show);

    ui->setupUi(this);

    ui->statusBar->setText("");

    ui->span_sb->setValue(xSpan);
    ui->span_sb->setMinimum(5.0);
    ui->span_sb->setMaximum(90.0);

    ui->rate_sb->setValue(refreshHz);
    ui->rate_sb->setMinimum(1.0);
    ui->rate_sb->setMaximum(30.0);

    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
    if(widgetDebug) {
        configure();
        connect(&dataTimer, &QTimer::timeout, this, &LogViewer::realtimeDataSlot);
        dataTimer.start(1000.0/refreshHz); // Interval 0 means to refresh as fast as possible
    } else {}
    connect(ui->hsb_xRange, &QScrollBar::valueChanged, this, &LogViewer::horzScrollBarChanged);
    //connect(ui->plot->xAxis, &QCPAxis::rangeChanged, this, &LogViewer::xAxisChanged);

    updatePause();
    ui->hsb_xRange->setEnabled(pauseUpdate);

    ui->hsb_xRange->setRange(0, xSpan*10.0);
}

void LogViewer::ecuConnected()
{
    qDebug() << "LogViewer::ecuConnected!";
}



double LogViewer::scaleDouble(double in, double iMin, double iMax, double oMin, double oMax)
{
    return((oMax-oMin)/(iMax-iMin)*(in-iMin)+oMin);
}

void LogViewer::realtimeDataSlot()
{
    QVector<float> scaledValues;
    static double t = 0.0;
    t += 1.0/refreshHz;

    double oMax = 0.0;
    double oMin = 0.0;

    std::uniform_real_distribution<double> dist(-1.0, 1.0);

    double randAdd = 0.0;

    for(int i = 0; i < RAM_MUT.size(); ++i)
    {
        oMax = RAM_MUT.at(i).scaling.max;
        oMin = RAM_MUT.at(i).scaling.min;
        randAdd = scaleDouble(dist(*QRandomGenerator::global()), -1.0, 1.0, oMin, oMax)*0.05;

        scaledValues.insert(i, scaleDouble(qCos(t), -1.0, 1.0, oMin, oMax) + randAdd);
    }
    logReady(scaledValues);
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
    ui->hsb_xRange->setEnabled(pauseUpdate);
    if(pauseUpdate)
    {
        if ((lastKey-firstKey) < xSpan)
            ui->hsb_xRange->setRange(ui->plot->xAxis->range().lower*10.0, ui->plot->xAxis->range().upper*10.0);
        else
            ui->hsb_xRange->setRange(firstKey*10.0, lastKey*10.0);
        ui->hsb_xRange->setValue(ui->plot->xAxis->range().center()*10.0);
    }
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

}

void LogViewer::showPlot(int state)
{
    for(int i = 0; i<plotVisibleCB.size(); ++i)
    {
        //enable/disable based on checkbox...
        ui->plot->graph(i)->setVisible(plotVisibleCB.at(i)->isChecked());
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

    //qDebug() << tr("LogViewer::logReady");

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
            lcdNumbers.at(i)->display(scaledValues.at(i));
            ui->plot->graph(i)->addData(key, scaleDouble(scaledValues.at(i), RAM_MUT.at(i).scaling.min, RAM_MUT.at(i).scaling.max, 0.0, 100.0));
            if(!pauseUpdate)
            {
                lastKey = key;
                firstKey = qMax(0.0, key-(xRange.upper-xRange.lower)*10.0-(key-lastPointKey));

                //Delete data out of view and rescale the axes
                ui->plot->graph(i)->data()->removeBefore(key-(xRange.upper-xRange.lower)*10.0-(key-lastPointKey));
                ui->plot->graph(i)->rescaleAxes(true);
            } else {
                //Keep more data just in case we want to do statistics later on the visible data
                ui->plot->graph(i)->data()->removeBefore(key-(xRange.upper-xRange.lower)*50.0);
            }
            totalSize += ui->plot->graph(i)->data()->size();
        }

        //qDebug() << tr("PlotTime = %1 sec (%2 Hz)").arg(key-lastPointKey).arg(1.0/(key-lastPointKey));
        lastPointKey = key;
        ++frameCount;

        if(!pauseUpdate)
        {
            // make key axis range scroll with the data:
            ui->plot->xAxis->setRange(key, xSpan, Qt::AlignRight);
            ui->plot->replot();
        }

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
    qDebug() << "LogViewer::ecuRamMut";
    RAM_MUT = ramMut;
    configureMut();
}

QString LogViewer::SearchFiles(QString path, QString CalID)       // Для поиска файлов в каталоге
{
    // Пытаемся найти правильные файлы, в текущем каталоге
    QStringList listFiles = QDir(path).entryList((CalID + "*.xml ").split(" "), QDir::Files);  //выборка файлов по маскам
    if (listFiles.size()  == 0)            // если файл не найдем вернем егог
        return "";
    //return QFileDialog::getOpenFileName(nullptr,  tr("Open xml"), path, tr("xml files (*.xml)"));
    else
        return path + listFiles.at(0);
}

void LogViewer::forceTestRamMut()
{
    ecuDefinition *ecuDef = new ecuDefinition;
    QString romID = ui->le_ecuid->text();

    if (!ecuDef->fromFile(SearchFiles(QApplication::applicationDirPath() + "/xml/", romID)))
    {
        delete ecuDef;
        qDebug() << "XML NOT FOUND!";
        return;
    }
    //Send RAM_MUT to self
    ecuRamMut(ecuDef->RAM_MUT);

    //Start fake data based on RAM_MUT
    connect(&dataTimer, &QTimer::timeout, this, &LogViewer::realtimeDataSlot);
    dataTimer.start(1000.0/refreshHz); // Interval 0 means to refresh as fast as possible

}

void LogViewer::showWin()
{
    this->show();
}

void LogViewer::configureMut()
{
    QVBoxLayout *gbLayout = new QVBoxLayout();

    // make left and bottom axes transfer their ranges to right and top axes:

    connect(ui->plot->xAxis, static_cast<void (QCPAxis::*)(const QCPRange &)>(&QCPAxis::rangeChanged), ui->plot->xAxis2, static_cast<void (QCPAxis::*)(const QCPRange &)>(&QCPAxis::setRange));
    //connect(ui->plot->xAxis, &QCPAxis::rangeChanged, ui->plot->xAxis2, &QCPAxis::setRange);

    //connect(ui->plot->yAxis, &QCPAxis::rangeChanged, ui->plot->yAxis2, &QCPAxis::setRange);
    connect(ui->plot->yAxis, static_cast<void (QCPAxis::*)(const QCPRange &)>(&QCPAxis::rangeChanged), ui->plot->yAxis2, static_cast<void (QCPAxis::*)(const QCPRange &)>(&QCPAxis::setRange));

    ui->plot->xAxis->setRange(xSpan, xSpan, Qt::AlignRight);
    ui->plot->replot();

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    ui->plot->xAxis->setTicker(timeTicker);
    ui->plot->axisRect()->setupFullAxesBox();
    ui->plot->yAxis->setRange(-1.2, 1.2);

    int gBoxSize = 200;
    for(int i = 0; i < RAM_MUT.size(); ++i)
    {
        //qDebug() << tr("%1: [%2:%3]").arg(RAM_MUT.at(i).name).arg(RAM_MUT.at(i).scaling.min).arg(RAM_MUT.at(i).scaling.max);

        /*
        //===========================================
        //Tried each on own axis...didnt work well
        //-------------------------------------------
        //Give each signal it's own y-axis for scaling
        QCPAxis *addAxis = ui->plot->axisRect()->addAxis(QCPAxis::AxisType::atBottom);//atLeft);
        //Add graph with new axis
        addAxis->setVisible(false);
        connect(ui->plot->yAxis, SIGNAL(rangeChanged(QCPRange)), addAxis, SLOT(setRange(QCPRange)));
        ui->plot->addGraph(addAxis, ui->plot->xAxis);
        //--------------------------------------------
        //============================================
        */
        ui->plot->addGraph();

        //Set pen color to random...
        QColor lineColor = QColor( qrand() % 256, qrand() % 256, qrand() % 256 );
        ui->plot->graph(i)->setPen(QPen(lineColor));

        //Make visible...
        ui->plot->graph(i)->setVisible(true);

        //Set brush to color area under curve; (more of a thought for Knock)
        QColor brushColor = QColor(lineColor);
        brushColor.setAlpha(0);
        ui->plot->graph(i)->setBrush(QBrush(brushColor));

        QHBoxLayout *hLayout = new QHBoxLayout();
        //Setup hide plot checkboxes
        plotVisibleCB.insert(i, new QCheckBox(tr("%1 (%2)").arg(RAM_MUT.at(i).name).arg(RAM_MUT.at(i).scaling.units)));
        plotVisibleCB.at(i)->setChecked(1);
        plotVisibleCB.at(i)->setCheckable(1);

        connect(plotVisibleCB.at(i), &QCheckBox::stateChanged, this, &LogViewer::showPlot);

        //add to group box on left
        hLayout->addWidget(plotVisibleCB.at(i), 0);
        lcdNumbers.insert(i, new QLCDNumber());
        lcdNumbers.at(i)->setMaximumWidth(50);
        lcdNumbers.at(i)->setDecMode();
        lcdNumbers.at(i)->setSegmentStyle(QLCDNumber::Flat);

        QFrame* line = new QFrame;
        line->setFrameShape(QFrame::HLine);

        line->setMaximumWidth(25);
        line->setMinimumWidth(25);
        line->setLineWidth(15);
        line->setMidLineWidth(15);

        QPalette palette = line->palette();
        palette.setColor(QPalette::WindowText, lineColor);
        line->setPalette(palette);
        hLayout->addWidget(line, 1);

        // get the palette
        palette = lcdNumbers.at(i)->palette();

        // foreground color
        palette.setColor(palette.WindowText, QColor(85, 85, 255));
        // background color
        palette.setColor(palette.Background, QColor(0, 170, 255));
        // "light" border
        palette.setColor(palette.Light, QColor(255, 0, 0));
        // "dark" border
        palette.setColor(palette.Dark, QColor(0, 255, 0));

        // set the palette
        lcdNumbers.at(i)->setPalette(palette);

        hLayout->addWidget(lcdNumbers.at(i), 2);
        gbLayout->addLayout(hLayout);
        //qDebug() << tr("%1 : %2 : %3").arg(plotVisibleCB.at(i)->sizeHint().width()).arg(line->sizeHint().width()).arg(lcdNumbers.at(i)->sizeHint().width());
        gBoxSize = qMax(gBoxSize, plotVisibleCB.at(i)->sizeHint().width() + 25 + lcdNumbers.at(i)->sizeHint().width() + 20);
    }

    ui->gb_params->setLayout(gbLayout);

    ui->gb_params->setMaximumWidth(gBoxSize);
    ui->gb_params->setMinimumWidth(gBoxSize);

    ui->plot->axisRect()->setRangeDragAxes(ui->plot->xAxis, nullptr);

    qDebug() << "";

    updatePause();

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
        /*
        randAmp[i]  = distAmp(*QRandomGenerator::global());
        randTime[i] = distTime(*QRandomGenerator::global());

        qDebug() << tr("randAmp[%1]  = ").arg(i) << tr("%1").arg(randAmp[i]);
        qDebug() << tr("randTime[%1] = ").arg(i) << tr("%1").arg(randTime[i]);
        */
        ui->plot->addGraph(); // blue line
        //ui->plot->graph(i)->setPen(QPen(QColor(40, 110, 255)));
        ui->plot->graph(i)->setPen(QPen( QColor( qrand() % 256, qrand() % 256, qrand() % 256 )));
        plotVisibleCB.insert(i, new QCheckBox(tr("PLOT %1").arg(i)));
        plotVisibleCB.at(i)->setChecked(1);
        plotVisibleCB.at(i)->setCheckable(1);

        gbLayout->addWidget(plotVisibleCB.at(i), i);
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
    connect(ui->plot->xAxis, static_cast<void (QCPAxis::*)(const QCPRange &)>(&QCPAxis::rangeChanged), ui->plot->xAxis2, static_cast<void (QCPAxis::*)(const QCPRange &)>(&QCPAxis::setRange));
    //connect(ui->plot->xAxis, &QCPAxis::rangeChanged, ui->plot->xAxis2, &QCPAxis::setRange);

    //connect(ui->plot->yAxis, &QCPAxis::rangeChanged, ui->plot->yAxis2, &QCPAxis::setRange);
    connect(ui->plot->yAxis, static_cast<void (QCPAxis::*)(const QCPRange &)>(&QCPAxis::rangeChanged), ui->plot->yAxis2, static_cast<void (QCPAxis::*)(const QCPRange &)>(&QCPAxis::setRange));

    plotReady = true;
    pauseUpdate = false;
    updatePause();
}

void LogViewer::on_pb_frcEcuId_clicked()
{
    forceTestRamMut();
}


void LogViewer::on_pb_spanApply_clicked()
{
        xSpan = ui->span_sb->value();
}


void LogViewer::on_pb_rateApply_clicked()
{
    refreshHz = ui->rate_sb->value();
    if(widgetDebug)
    {
        dataTimer.setInterval(1000.0/refreshHz); // Interval 0 means to refresh as fast as possible
    }
}


void LogViewer::on_hsb_xRange_sliderReleased()
{
    ui->plot->replot();
}

void LogViewer::horzScrollBarChanged(int value)
{
    double dValue = value / 10.0;
    double xMove = ui->plot->xAxis->range().center()-dValue;

    qDebug() << tr("xMove = %1").arg(xMove);

    if (qAbs(xMove) > 0.1) // if user is dragging plot, we don't want to replot twice
    {
        ui->plot->xAxis->setRange(dValue, ui->plot->xAxis->range().size(), Qt::AlignCenter);
    }
    if (qAbs(xMove) >= 1.0)
    {
        ui->plot->replot();
    }
}

void LogViewer::on_hsb_xRange_sliderPressed()
{

}

