#include <QApplication>
#include <QLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFont>

#include "launcherdialog.hpp"

LauncherDialog::LauncherDialog(QWidget *parent) : QDialog(parent)
{
    // Set launcher original width
    this->setFixedWidth(width);

    // Set the dialog layout
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setAlignment(Qt::AlignCenter);
    this->setLayout(layout);

    // Populate the launcher Dialog
    QLabel *topLabel = new QLabel("AR Board Game");
    topLabel->setAlignment(Qt::AlignCenter);
    topLabel->setFont(QFont("Courier",25));
    layout->addWidget(topLabel);

    QSpacerItem *spacer = new QSpacerItem(0,20);
    layout->addItem(spacer);

    QPushButton *initializeBoardButton = new QPushButton("Initialize AR Board");
    layout->addWidget(initializeBoardButton);
    layout->setAlignment(initializeBoardButton,Qt::AlignCenter);
    initializeBoardButton->setFixedSize(initializeBoardButton->sizeHint().width(), initializeBoardButton->sizeHint().height());

    QPushButton *launchGameButton = new QPushButton("Launch game");
    layout->addWidget(launchGameButton);
    layout->setAlignment(launchGameButton,Qt::AlignCenter);
    launchGameButton->setFixedSize(launchGameButton->sizeHint().width(), launchGameButton->sizeHint().height());
    launchGameButton->setEnabled(false);


    QPushButton *quitButton = new QPushButton("Quit");
    layout->addWidget(quitButton);
    layout->setAlignment(quitButton,Qt::AlignCenter);
    quitButton->setFixedSize(quitButton->sizeHint().width(), quitButton->sizeHint().height());
    QObject::connect(quitButton, SIGNAL(clicked()), qApp, SLOT(quit()));
}

