/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.11.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionOpen;
    QAction *actionSave;
    QAction *actionExit;
    QAction *actionXLines;
    QAction *actionYLines;
    QAction *actionZLines;
    QAction *actionRPlane;
    QAction *actionRHeight;
    QAction *actionPolygon;
    QAction *actionRescale;
    QAction *actionReset;
    QAction *actionXVPoint;
    QAction *actionYVPoint;
    QAction *actionZVPoint;
    QAction *actionHomography;
    QAction *actionAlpha;
    QAction *actionScene2Image;
    QAction *actionPtPool;
    QAction *actionChooseXRef;
    QAction *actionChooseYRef;
    QAction *actionChooseZRef;
    QAction *actiontest;
    QAction *actionChooseOrigin;
    QAction *actionXY_Plane;
    QAction *actionXZ_Plane;
    QAction *actionYZ_Plane;
    QAction *actionShowPlane;
    QWidget *centralWidget;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuEdit;
    QMenu *menuChooseRefPt;
    QMenu *menuCalculation;
    QMenu *menuTexture;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(400, 300);
        actionOpen = new QAction(MainWindow);
        actionOpen->setObjectName(QStringLiteral("actionOpen"));
        actionSave = new QAction(MainWindow);
        actionSave->setObjectName(QStringLiteral("actionSave"));
        actionExit = new QAction(MainWindow);
        actionExit->setObjectName(QStringLiteral("actionExit"));
        actionXLines = new QAction(MainWindow);
        actionXLines->setObjectName(QStringLiteral("actionXLines"));
        actionYLines = new QAction(MainWindow);
        actionYLines->setObjectName(QStringLiteral("actionYLines"));
        actionZLines = new QAction(MainWindow);
        actionZLines->setObjectName(QStringLiteral("actionZLines"));
        actionRPlane = new QAction(MainWindow);
        actionRPlane->setObjectName(QStringLiteral("actionRPlane"));
        actionRHeight = new QAction(MainWindow);
        actionRHeight->setObjectName(QStringLiteral("actionRHeight"));
        actionPolygon = new QAction(MainWindow);
        actionPolygon->setObjectName(QStringLiteral("actionPolygon"));
        actionRescale = new QAction(MainWindow);
        actionRescale->setObjectName(QStringLiteral("actionRescale"));
        actionReset = new QAction(MainWindow);
        actionReset->setObjectName(QStringLiteral("actionReset"));
        actionXVPoint = new QAction(MainWindow);
        actionXVPoint->setObjectName(QStringLiteral("actionXVPoint"));
        actionXVPoint->setShortcutVisibleInContextMenu(true);
        actionYVPoint = new QAction(MainWindow);
        actionYVPoint->setObjectName(QStringLiteral("actionYVPoint"));
        actionYVPoint->setShortcutVisibleInContextMenu(true);
        actionZVPoint = new QAction(MainWindow);
        actionZVPoint->setObjectName(QStringLiteral("actionZVPoint"));
        actionZVPoint->setShortcutVisibleInContextMenu(true);
        actionHomography = new QAction(MainWindow);
        actionHomography->setObjectName(QStringLiteral("actionHomography"));
        actionHomography->setShortcutVisibleInContextMenu(true);
        actionAlpha = new QAction(MainWindow);
        actionAlpha->setObjectName(QStringLiteral("actionAlpha"));
        actionScene2Image = new QAction(MainWindow);
        actionScene2Image->setObjectName(QStringLiteral("actionScene2Image"));
        actionPtPool = new QAction(MainWindow);
        actionPtPool->setObjectName(QStringLiteral("actionPtPool"));
        actionChooseXRef = new QAction(MainWindow);
        actionChooseXRef->setObjectName(QStringLiteral("actionChooseXRef"));
        actionChooseYRef = new QAction(MainWindow);
        actionChooseYRef->setObjectName(QStringLiteral("actionChooseYRef"));
        actionChooseZRef = new QAction(MainWindow);
        actionChooseZRef->setObjectName(QStringLiteral("actionChooseZRef"));
        actiontest = new QAction(MainWindow);
        actiontest->setObjectName(QStringLiteral("actiontest"));
        actionChooseOrigin = new QAction(MainWindow);
        actionChooseOrigin->setObjectName(QStringLiteral("actionChooseOrigin"));
        actionXY_Plane = new QAction(MainWindow);
        actionXY_Plane->setObjectName(QStringLiteral("actionXY_Plane"));
        actionXZ_Plane = new QAction(MainWindow);
        actionXZ_Plane->setObjectName(QStringLiteral("actionXZ_Plane"));
        actionYZ_Plane = new QAction(MainWindow);
        actionYZ_Plane->setObjectName(QStringLiteral("actionYZ_Plane"));
        actionShowPlane = new QAction(MainWindow);
        actionShowPlane->setObjectName(QStringLiteral("actionShowPlane"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 400, 22));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QStringLiteral("menuFile"));
        menuEdit = new QMenu(menuBar);
        menuEdit->setObjectName(QStringLiteral("menuEdit"));
        menuChooseRefPt = new QMenu(menuEdit);
        menuChooseRefPt->setObjectName(QStringLiteral("menuChooseRefPt"));
        menuCalculation = new QMenu(menuBar);
        menuCalculation->setObjectName(QStringLiteral("menuCalculation"));
        menuTexture = new QMenu(menuCalculation);
        menuTexture->setObjectName(QStringLiteral("menuTexture"));
        MainWindow->setMenuBar(menuBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindow->setStatusBar(statusBar);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuEdit->menuAction());
        menuBar->addAction(menuCalculation->menuAction());
        menuFile->addAction(actionOpen);
        menuFile->addAction(actionSave);
        menuFile->addSeparator();
        menuFile->addAction(actionExit);
        menuEdit->addAction(actionXLines);
        menuEdit->addAction(actionYLines);
        menuEdit->addAction(actionZLines);
        menuEdit->addAction(menuChooseRefPt->menuAction());
        menuEdit->addAction(actionPtPool);
        menuEdit->addAction(actionPolygon);
        menuEdit->addSeparator();
        menuEdit->addAction(actionRescale);
        menuEdit->addAction(actionReset);
        menuChooseRefPt->addAction(actionChooseXRef);
        menuChooseRefPt->addAction(actionChooseYRef);
        menuChooseRefPt->addAction(actionChooseZRef);
        menuChooseRefPt->addAction(actionChooseOrigin);
        menuCalculation->addAction(actionXVPoint);
        menuCalculation->addAction(actionYVPoint);
        menuCalculation->addAction(actionZVPoint);
        menuCalculation->addAction(actionHomography);
        menuCalculation->addAction(menuTexture->menuAction());
        menuCalculation->addAction(actionScene2Image);
        menuCalculation->addAction(actionShowPlane);
        menuTexture->addAction(actionXY_Plane);
        menuTexture->addAction(actionXZ_Plane);
        menuTexture->addAction(actionYZ_Plane);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", nullptr));
        actionOpen->setText(QApplication::translate("MainWindow", "Open", nullptr));
        actionSave->setText(QApplication::translate("MainWindow", "Save", nullptr));
        actionExit->setText(QApplication::translate("MainWindow", "Exit", nullptr));
        actionXLines->setText(QApplication::translate("MainWindow", "XLines", nullptr));
        actionYLines->setText(QApplication::translate("MainWindow", "YLines", nullptr));
        actionZLines->setText(QApplication::translate("MainWindow", "ZLines", nullptr));
        actionRPlane->setText(QApplication::translate("MainWindow", "RPlane", nullptr));
        actionRHeight->setText(QApplication::translate("MainWindow", "RHeight", nullptr));
        actionPolygon->setText(QApplication::translate("MainWindow", "Polygon", nullptr));
        actionRescale->setText(QApplication::translate("MainWindow", "Rescale", nullptr));
        actionReset->setText(QApplication::translate("MainWindow", "Reset", nullptr));
        actionXVPoint->setText(QApplication::translate("MainWindow", "XVPoint", nullptr));
        actionYVPoint->setText(QApplication::translate("MainWindow", "YVPoint", nullptr));
        actionZVPoint->setText(QApplication::translate("MainWindow", "ZVPoint", nullptr));
        actionHomography->setText(QApplication::translate("MainWindow", "Homography", nullptr));
        actionAlpha->setText(QApplication::translate("MainWindow", "Alpha", nullptr));
        actionScene2Image->setText(QApplication::translate("MainWindow", "Scene2Image", nullptr));
        actionPtPool->setText(QApplication::translate("MainWindow", "PtPool", nullptr));
        actionChooseXRef->setText(QApplication::translate("MainWindow", "ChooseXRef", nullptr));
        actionChooseYRef->setText(QApplication::translate("MainWindow", "ChooseYRef", nullptr));
        actionChooseZRef->setText(QApplication::translate("MainWindow", "ChooseZRef", nullptr));
        actiontest->setText(QApplication::translate("MainWindow", "test", nullptr));
        actionChooseOrigin->setText(QApplication::translate("MainWindow", "ChooseOrigin", nullptr));
        actionXY_Plane->setText(QApplication::translate("MainWindow", "XY Plane", nullptr));
        actionXZ_Plane->setText(QApplication::translate("MainWindow", "XZ Plane", nullptr));
        actionYZ_Plane->setText(QApplication::translate("MainWindow", "YZ Plane", nullptr));
        actionShowPlane->setText(QApplication::translate("MainWindow", "ShowPlane", nullptr));
        menuFile->setTitle(QApplication::translate("MainWindow", "File", nullptr));
        menuEdit->setTitle(QApplication::translate("MainWindow", "Edit", nullptr));
        menuChooseRefPt->setTitle(QApplication::translate("MainWindow", "ChooseRefPt", nullptr));
        menuCalculation->setTitle(QApplication::translate("MainWindow", "Calculation", nullptr));
        menuTexture->setTitle(QApplication::translate("MainWindow", "Texture", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
