#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <vector>
#include "dicom.h"

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow();

private:
    std::vector<Dicom> sliceList;
};

#endif // MAINWINDOW_H
