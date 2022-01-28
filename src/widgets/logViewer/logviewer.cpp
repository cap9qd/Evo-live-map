#include "logviewer.h"

LogViewer::LogViewer(QWidget *parent, ecuManager *ecu_manager)
    : QWidget(nullptr)
{
    logWin = new LogPlotter(nullptr, ecu_manager);

    //===============================================================
    //For menu
    //--------------------------------------------------------------
    QToolButton *menuButton = new QToolButton(this);
    QBoxLayout  *menuLayout = new QBoxLayout(QBoxLayout::TopToBottom);

    menuButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    menuButton->setIcon(QIcon(":ico/logview.png"));
    menuButton->setIconSize(QSize(150, 150));
    connect(menuButton, &QPushButton::clicked, this, &LogViewer::showWin);
    QLabel *menuLabel = new QLabel("LogPlotter");
    menuLabel->setAlignment(Qt::AlignCenter);

    menuLayout->addWidget(menuLabel);
    menuLayout->addWidget(menuButton);
    setLayout(menuLayout);
    //--------------------------------------------------------------
    //===============================================================
}

LogViewer::~LogViewer()
{

}

void LogViewer::showWin()
{
    logWin->show();
}
