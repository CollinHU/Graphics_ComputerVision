#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QMainWindow>
#include <QImage>
#ifndef QT_NO_PRINTER
#include <QPrinter>
#endif

#include "node.h"
#include <QVector>
#include <QShortcut>

class QAction;
class QLabel;
class QMenu;
class QScrollArea;
class QScrollBar;

class ImageViewer : public QMainWindow
{
    Q_OBJECT

public:
    ImageViewer();
    bool loadFile(const QString &);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
   // void paintEvent(QPaintEvent *event) override;
    //void resizeEvent(QResizeEvent *event) override;

//private:

private slots:
    void open();
    //void saveAs();
    void print();
    void copy();
    void paste();
    void zoomIn();
    void zoomOut();
    void normalSize();
    void fitToWindow();
    void about();

    void imageOnly();
    void imageWithContour();
    void saveContour();
    void saveMask();
    //void undo();
    void costGraph();
    void minPath();
    void drawMinPath(QPoint point);
    void pathTree();
    void pixelNode();

    void addFollowingSeedPoint();
    void addFirstSeedPoint();
    void finishCurrentContour();
    void finishCurrentContourClose();
    void undo();

private:
    void createActions();
    void createMenus();
    void updateActions();
    bool saveFile(const QString &fileName, const QRect &Rect);
    void setImage(const QImage &newImage);
    void scaleImage(double factor);
    void adjustScrollBar(QScrollBar *scrollBar, double factor);

    void drawLineWithNode();
    bool atImage(QPoint point);

    QPoint convert_position(QPoint point);
    QPoint cursorSnap(QPoint point);
    void shortestPath(QPoint point);

    void computeCost();
    void delay();

    //void addSeedPoint();
    //void addFirstSeedPoint();


    bool scribbling;
    bool moveEnable;

    QVector<QPoint> seedPoints;
    QVector<QPoint> wirePoints;

    QPoint endPoint;

    QImage image;
    QLabel *imageLabel;
    QScrollArea *scrollArea;
    QPixmap qpixmap;//new add
    double scaleFactor;

#ifndef QT_NO_PRINTER
    QPrinter printer;
#endif

    //QAction *saveAsAct;
    QAction *printAct;
    QAction *copyAct;
    QAction *zoomInAct;
    QAction *zoomOutAct;
    QAction *normalSizeAct;
    QAction *fitToWindowAct;

    QAction *imageOnlyAct;
    QAction *imageWithContourAct;
    QAction *saveContourAct;
    //QAction *undoAct;
    QAction *costGraphAct;
    QAction *minPathAct;
    QAction *pathTreeAct;
    QAction *pixelNodeAct;
    QAction *saveMaskAct;

    QShortcut *addFirstSeedPointSC;
    QShortcut *addFollowingSeedPointSC;
    QShortcut *finishCurrentContourSC;
    QShortcut *finishCurrentContourCloseSC;
    QShortcut *undoSC;
};

#endif
