#include "mainwindow.h"
#include "viewwidget.h"
#include "volumewidget.h"
#include "dicom.h"
#include <QGridLayout>
#include <QDir>
#include <QFileDialog>

MainWindow::MainWindow()
{
    setWindowTitle(tr("Graphics"));
    setStyleSheet("background-color:black;");

    QString dirName = QFileDialog::getExistingDirectory(this, "Select Dicom Folder", "./");
    QDir dir(dirName);
    //QDir dir("Head");
    QFileInfoList files = dir.entryInfoList();

/*
    // 2D DICOM Viewer.
    QString file = files.at(2).filePath();
    Dicom dicom(file.toLocal8Bit().data());
    ViewWidget *view = new ViewWidget(this, dicom.image());
*/

    // Volume Rnedering Viewer.
    VolumeWidget *view = new VolumeWidget(this);
    for (int i = 2; i < files.length(); i++)
    {
        QString file = files.at(i).filePath();
        Dicom dicom(file.toLocal8Bit().data());
        view->addSliceImage(dicom.image());
        if (i == 2) view->setResolution(dicom.row(), dicom.column());
    }
    view->animate();

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(view, 0, 0);
    setLayout(layout);

}
