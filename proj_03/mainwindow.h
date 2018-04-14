#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "svmgraphicsview.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionExit_triggered();

    void on_actionOpen_triggered();

    void on_actionRescale_triggered();

    void on_actionXLines_triggered();

    void on_actionYLines_triggered();

    void on_actionZLines_triggered();

    void on_actionXVPoint_triggered();

    void on_actionYVPoint_triggered();

    void on_actionZVPoint_triggered();

    void on_actionHomography_triggered();

    //void on_actionRPlane_triggered();

    //void on_actionRHeight_triggered();

    //void on_actionAlpha_triggered();

    void on_actionScene2Image_triggered();

    void on_actionPtPool_triggered();

    void on_actionPolygon_triggered();

    //**********
    void on_actionChooseOrigin_triggered();
    void on_actionChooseXRef_triggered();
    void on_actionChooseYRef_triggered();
    void on_actionChooseZRef_triggered();

    //void on_actionTexture_triggered();

    void on_actionSave_triggered();

    void on_actionXY_Plane_triggered();

    void on_actionXZ_Plane_triggered();

    void on_actionYZ_Plane_triggered();

    void on_actionShowPlane_triggered();

private:
    Ui::MainWindow *ui;
    SVMGraphicsView *svmView;
};

#endif // MAINWINDOW_H
