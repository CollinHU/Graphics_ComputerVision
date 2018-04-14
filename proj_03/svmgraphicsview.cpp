#include "svmgraphicsview.h"

#include <QtWidgets>
#include <QFileDialog>
#include <iostream>
#include <cmath>
#include "eigen.h"


using namespace std;
using namespace Eigen;

SVMGraphicsView::SVMGraphicsView(QWidget *parent) :
    QGraphicsView(parent)
{
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    Scene = new QGraphicsScene(this);
    Scene->setBackgroundBrush(QBrush(Qt::darkGray, Qt::SolidPattern));
    setScene(Scene);

    setDragMode(ScrollHandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    viewport()->setCursor(Qt::ArrowCursor);
    _pan = false;
    _hasimage = false;

}

void SVMGraphicsView::wheelEvent(QWheelEvent *event)
{
    /*
    double scaleFactor = 1.15;
    if(event->delta() > 0)
    {
        scale(scaleFactor, scaleFactor);
    } else {
        scale(1.0 / scaleFactor, 1.0 / scaleFactor);
    }*/
}

void SVMGraphicsView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        _pan = true;
        _panStartX = event->x();
        _panStartY = event->y();
        viewport()->setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
    else if (event->button() == Qt::LeftButton)
    {
        if (_hasimage)
        {
            QPointF imgCursorPos = mapToScene(event->pos());
            double x = imgCursorPos.x();
            double y = imgCursorPos.y();

            QPoint imgPix = imgCursorPos.toPoint();
            if (imgPix.x() >=0 && imgPix.x() < image.size().width() && imgPix.y() >=0 && imgPix.y() < image.size().height())
            {

                if (QApplication::keyboardModifiers() && Qt::ControlModifier)
                {
                    QGraphicsItem* itemAtCur = Scene->itemAt(imgCursorPos, QGraphicsView::transform());
                    if (itemAtCur->type() == QGraphicsEllipseItem::Type)
                    {
                        x = itemAtCur->boundingRect().center().x();
                        y = itemAtCur->boundingRect().center().y();
                    }
                //    cout << itemAtCur->type() << endl;
                }

                switch(state)
                {
                case _START:
                    cout << "Not Start Yet ?" << endl;
                    break;
                case _XLines:
                    pushLines(XLines, x, y);
                    break;
                case _YLines:
                    pushLines(YLines, x, y);
                    break;
                case _ZLines:
                    pushLines(ZLines, x, y);
                    break;
                //case _RPlane:
                //    pushRefPoints(x, y);
                //    break;
                //case _RHeight:
                //    if (RHpoints.size() < 2)
                //        pushRefPoints(x, y);
                //    break;
                case _Polygon:
                    pushPolyPool(x, y);
                    break;
                case _PtPool:
                    pushPtPool(x, y);
                    break;
                case _ChooseOrigin:
                    assignRef(x, y);
                    break;
                case _RefX:
                    assignRef(x, y);
                    break;
                case _RefY:
                    assignRef(x, y);
                    break;
                case _RefZ:
                    assignRef(x, y);
                    break;
                case _Hxy:
                    pushBxPool(x, y);
                    break;
                case _Hxz:
                    pushBxPool(x, y);
                    break;
                case _Hyz:
                    pushBxPool(x, y);
                    break;
                default:
                    cout << "Already Finished ?" << endl;
                    break;
                }
            }
        }

    }
    event->ignore();
}

void SVMGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        _pan = false;
        viewport()->setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }
    event->ignore();
}

void SVMGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    if (_pan)
    {
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - (event->x() - _panStartX));
        verticalScrollBar()->setValue(verticalScrollBar()->value() - (event->y() - _panStartY));
        _panStartX = event->x();
        _panStartY = event->y();
        event->accept();
        return;
    }

    QGraphicsView::mouseMoveEvent(event);

    if (_hasimage)
    {
        QPointF imgCursorPos = mapToScene(event->pos());
        QPoint imgPix = imgCursorPos.toPoint();
        if(pointCache != NULL)
        {
            drawScene();
            if (imgPix.x() >=0 && imgPix.x() < image.size().width() && imgPix.y() >=0 && imgPix.y() < image.size().height())
            {
                double endx = pointCache->x +  (imgCursorPos.x() - pointCache->x);
                double endy = pointCache->y +  (imgCursorPos.y() - pointCache->y);
                Scene->addLine(pointCache->x, pointCache->y, endx, endy, QPen(Qt::white, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            }
        }
//        cout << imgCursorPos.x() << ", " << imgCursorPos.y() << endl;

        if(_zVP_exist && (state == _RHeight || state == _PtPool) && imgPix.x() >=0 && imgPix.x() < image.size().width() && imgPix.y() >=0 && imgPix.y() < image.size().height())
        {
            if(pointCache == NULL)
                drawScene();

            double endx = zVP->x + (1-gap) * (imgCursorPos.x() - zVP->x);
            double endy = zVP->y + (1-gap) * (imgCursorPos.y() - zVP->y);
            Scene->addLine(zVP->x, zVP->y, endx, endy, QPen(Qt::white, 0.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            endx = zVP->x + (1+gap) * (imgCursorPos.x() - zVP->x);
            endy = zVP->y + (1+gap) * (imgCursorPos.y() - zVP->y);
            double endx2 = zVP->x + 1.1 * (imgCursorPos.x() - zVP->x);
            double endy2 = zVP->y + 1.1 * (imgCursorPos.y() - zVP->y);
            Scene->addLine(endx, endy, endx2, endy2, QPen(Qt::white, 0.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        }
    }
    event->ignore();
}

void SVMGraphicsView::svmInit()
{
    XLines.clear();
    YLines.clear();
    ZLines.clear();
    RPoints.clear();
    RHpoints.clear();
    PtPool.clear();
    PolyPool.clear();
    Scene->clear();
    state = _START;
    pointCache = NULL;
    poolCache = NULL;
    xVP = NULL;
    yVP = NULL;
    zVP = NULL;

    origin = NULL;
    X_Ref = NULL;
    Y_Ref = NULL;
    Z_Ref = NULL;

    _xVP_exist = false;
    _yVP_exist = false;
    _zVP_exist = false;
    _refHomo_exist = false;
    _getGammaZ = false;
    _method = 0;
    _textureMethod = 0;
    gap = 0.005;
    refHomoI2S << 0,0,0,
                  0,0,0,
                  0,0,0;
    refHomoS2I << 0,0,0,
                  0,0,0,
                  0,0,0;
    Proj << 0, 0, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0;
    patchConfirmDlg = new patchConfirmDialog(this);
}

void SVMGraphicsView::openAct()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::currentPath());

    if (!fileName.isEmpty())
    {
        image = QImage(fileName);
        if (image.isNull()) {
            QMessageBox::information(this, tr("Single View Modeling"), tr("Cannot load %1.").arg(fileName));
            return;
        }
        svmInit();
        Scene->addPixmap(QPixmap::fromImage(image));
        resetTransform();
        horizontalScrollBar()->setValue(0);
        verticalScrollBar()->setValue(0);
        _pan = false;
        _hasimage = true;
    }
}

void SVMGraphicsView::rescaleAct()
{
    if (_hasimage)
    {
        setTransform(QTransform::fromScale(1, 1));
    }
}

void SVMGraphicsView::pickXLines()
{
    if (_hasimage)
    {
        cout << "Please pick up endpoints of lines pointing in X direction" << endl;
        state = _XLines;
        if (pointCache != NULL)
        {
            pointCache = NULL;
            drawScene();
        }
    } else {
        QMessageBox::warning(this, tr("Warning"), tr("Please load a picture first !"));
    }
}

void SVMGraphicsView::pickYLines()
{
    if (_hasimage)
    {
        cout << "Please pick up endpoints of lines pointing in Y direction" << endl;
        state = _YLines;
        if (pointCache != NULL)
        {
            pointCache = NULL;
            drawScene();
        }
    } else {
        QMessageBox::warning(this, tr("Warning"), tr("Please load a picture first !"));
    }
}

void SVMGraphicsView::pickZLines()
{
    if (_hasimage)
    {
        cout << "Please pick up endpoints of lines pointing in Z direction" << endl;
        state = _ZLines;
        if (pointCache != NULL)
        {
            pointCache = NULL;
            drawScene();
        }
    } else {
        QMessageBox::warning(this, tr("Warning"), tr("Please load a picture first !"));
    }
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void SVMGraphicsView::chooseOrigin()
{
    if (_hasimage)
    {
        if(origin == NULL)
            origin = new connPoint;
        state = _ChooseOrigin;
    } else {
        QMessageBox::warning(this, tr("Warning"), tr("Please load a picture first !"));
    }
}

//void SVMGraphicsView::assignOrigin(double x ,double y)


void SVMGraphicsView::chooseXRef()
{
    if (_hasimage)
    {
        if(X_Ref == NULL)
            X_Ref = new connPoint;
        state = _RefX;
    } else {
        QMessageBox::warning(this, tr("Warning"), tr("Please load a picture first !"));
    }
}

//void SVMGraphicsView::assignXRef(double x ,double y)


void SVMGraphicsView::chooseYRef()
{
    if (_hasimage)
    {
        if(Y_Ref == NULL)
            Y_Ref = new connPoint;
        state = _RefY;
    } else {
        QMessageBox::warning(this, tr("Warning"), tr("Please load a picture first !"));
    }
}

//void SVMGraphicsView::assignYRef(double x ,double y)

void SVMGraphicsView::chooseZRef()
{
    if (_hasimage)
    {
        if(Z_Ref == NULL)
            Z_Ref = new connPoint;
        state = _RefZ;
    } else {
        QMessageBox::warning(this, tr("Warning"), tr("Please load a picture first !"));
    }
}

void SVMGraphicsView::assignRef(double x ,double y)
{
    popRefInDialog(x, y);
    drawScene();
}

void SVMGraphicsView::calculateProj()
{
    if(xVP == NULL || yVP == NULL || zVP == NULL || X_Ref == NULL || Y_Ref == NULL || Z_Ref == NULL)
    {
        cout << "Please calculate all staff first!" << endl;
    return;
    }
    double a_scale = subtractImgP(X_Ref->imgPt, origin->imgPt) / subtractImgP(*xVP, X_Ref->imgPt ) / X_Ref->scnPt.x;
    double b_scale = subtractImgP(Y_Ref->imgPt, origin->imgPt) / subtractImgP(*yVP, Y_Ref->imgPt ) / Y_Ref->scnPt.y;
    double c_scale = subtractImgP(Z_Ref->imgPt, origin->imgPt) / subtractImgP(*zVP, Z_Ref->imgPt ) / Z_Ref->scnPt.z;
    Proj << a_scale * imgPointToVector3d(*xVP), b_scale * imgPointToVector3d(*yVP), c_scale * imgPointToVector3d(*zVP),
            imgPointToVector3d(origin->imgPt);
    gammaZ = c_scale;
    _getGammaZ = true;
}

double SVMGraphicsView::subtractImgP(imgPoint a, imgPoint b)
{
    return (a.x - b.x + a.y- b.y)/2;
}


bool SVMGraphicsView::isLinesReady()
{
    if (_hasimage)
    {
        if (XLines.size() < 2)
        {
            QMessageBox::warning(this, tr("Warning"), tr("Please specify more lines in X direction so i can calculate X vanish point :)"));
        } else if (YLines.size() < 2) {
            QMessageBox::warning(this, tr("Warning"), tr("Please specify more lines in Y direction so i can calculate Y vanish point :)"));
        } else if (ZLines.size() < 2) {
            QMessageBox::warning(this, tr("Warning"), tr("Please specify more lines in Z direction so i can calculate Z vanish point :)"));
        } else {
            return true;
        }
    }
    return false;
}

void SVMGraphicsView::pushLines(std::vector<imgLine> &lineContainer, double x, double y)
{
    if (pointCache == NULL)
    {
        pointCache = new imgPoint;
        pointCache->x = x;
        pointCache->y = y;
        pointCache->w = 1;
    } else {
        imgLine newLine;
        newLine.p0 = *pointCache;
        newLine.p1.x = x;
        newLine.p1.y = y;
        newLine.p1.w = 1;
        lineContainer.push_back(newLine);
        pointCache = NULL;
    }
    drawScene();
}

bool SVMGraphicsView::popRefInDialog(double x, double y)
{
    QDialog dialog(this);
    // Use a layout allowing to have a label next to each field
    QFormLayout form(&dialog);

    // Add some text above the fields
    form.addRow(new QLabel("Please input the 3D coordinates:"));

    // Add the lineEdits with their respective labels
    QList<QLineEdit *> fields;
    for(int i = 0; i < 3; ++i) {
        QLineEdit *lineEdit = new QLineEdit(&dialog);
        QString label;
        switch(i)
        {
            case 0:
                label = QString("X: ");
                if (state == _RefY || state == _RefZ || state == _ChooseOrigin)
                {
                    lineEdit->setText("0");
                    lineEdit->setReadOnly(true);
                    lineEdit->setStyleSheet(QString( "background-color: gray"));
                }
                else {
                    lineEdit->setValidator(new QDoubleValidator());
                }
                break;
            case 1:
                label = QString("Y: ");
                if (state == _RefX || state == _RefZ || state == _ChooseOrigin)
                {
                    lineEdit->setText("0");
                    lineEdit->setReadOnly(true);
                    lineEdit->setStyleSheet(QString( "background-color: gray"));
                }
                else {
                    lineEdit->setValidator(new QDoubleValidator());
                }
                break;
            default:
                label = QString("Z: ");
                /*
                if (state == _RPlane)
                {
                    lineEdit->setText("0");
                    lineEdit->setReadOnly(true);
                    lineEdit->setStyleSheet(QString( "background-color: gray"));
                }
                */
                if(state == _RefX || state == _RefY || state == _ChooseOrigin) {
                    lineEdit->setText("0");
                    lineEdit->setReadOnly(true);
                    lineEdit->setStyleSheet(QString( "background-color: gray"));
                }
                else {
                    lineEdit->setValidator(new QDoubleValidator());
                }
                break;
        }
        form.addRow(label, lineEdit);
        fields << lineEdit;
    }

    // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    // Show the dialog as modal
    if (dialog.exec() == QDialog::Accepted) {

        connPoint newRefPt;
        newRefPt.imgPt.w = 1;
        newRefPt.imgPt.x = x;
        newRefPt.imgPt.y = y;
        newRefPt.scnPt.w = 1;
        newRefPt.scnPt.z = 0;
 //       newRefPt.homoMatrix = NULL;
        newRefPt.homoMatrixEig3 << 0, 0, 0,
                                   0, 0, 0,
                                   0, 0, 0;

        int i = 0;
        // If the user didn't dismiss the dialog, do something with the fields
        foreach(QLineEdit * lineEdit, fields) {
            switch(i)
            {
                case 0:
                    newRefPt.scnPt.x = lineEdit->text().toDouble();
                    i++;
                    break;
                case 1:
                    newRefPt.scnPt.y = lineEdit->text().toDouble();
                    i++;
                    break;
                case 2:
                    newRefPt.scnPt.z = lineEdit->text().toDouble();
                    i++;
                    break;
                default:
                    break;
            }
        }

        switch(state)
        {
            /*
            case _RPlane:
                RPoints.push_back(newRefPt);
                break;
            case _RHeight:
                RHpt_cache = newRefPt;
                RHpoints.push_back(RHpt_cache);
                break;
            */
            case _ChooseOrigin:
                 origin->imgPt.w = 1;
                 origin->imgPt.x = x;
                 origin->imgPt.y = y;
                 origin->scnPt.w = 1;
                 origin->scnPt.x = newRefPt.scnPt.x;
                 origin->scnPt.y = newRefPt.scnPt.y;
                 origin->scnPt.z = newRefPt.scnPt.z;
                 break;
            case _RefX:
                 X_Ref->imgPt.w = 1;
                 X_Ref->imgPt.x = x;
                 X_Ref->imgPt.y = y;
                 X_Ref->scnPt.w = 1;
                 X_Ref->scnPt.x = newRefPt.scnPt.x;
                 X_Ref->scnPt.y = newRefPt.scnPt.y;
                 X_Ref->scnPt.z = newRefPt.scnPt.z;
                 break;
            case _RefY:
                 Y_Ref->imgPt.w = 1;
                 Y_Ref->imgPt.x = x;
                 Y_Ref->imgPt.y = y;
                 Y_Ref->scnPt.w = 1;
                 Y_Ref->scnPt.x = newRefPt.scnPt.x;
                 Y_Ref->scnPt.y = newRefPt.scnPt.y;
                 Y_Ref->scnPt.z = newRefPt.scnPt.z;
                 break;
            case _RefZ:
                 Z_Ref->imgPt.w = 1;
                 Z_Ref->imgPt.x = x;
                 Z_Ref->imgPt.y = y;
                 Z_Ref->scnPt.w = 1;
                 Z_Ref->scnPt.x = newRefPt.scnPt.x;
                 Z_Ref->scnPt.y = newRefPt.scnPt.y;
                 Z_Ref->scnPt.z = newRefPt.scnPt.z;
                 break;
            default:
                break;
        }

        return true;
    }
    return false;
}

void SVMGraphicsView::drawPoint(double x, double y, int size, const QPen & pen, const QBrush & brush)
{
    QGraphicsEllipseItem* pt = new QGraphicsEllipseItem;
    pt->setPen(pen);
    pt->setBrush(brush);
    pt->setRect(x-size/2, y-size/2, size, size);
    Scene->addItem(pt);
}

void SVMGraphicsView::drawScene()
{
    Scene->clear();
    Scene->addPixmap(QPixmap::fromImage(image));

    std::vector<imgLine>::iterator iterLINE;
    for(iterLINE = XLines.begin(); iterLINE != XLines.end(); iterLINE++)
    {
        imgLine line = *iterLINE;

        Scene->addLine(line.p0.x, line.p0.y, line.p1.x, line.p1.y, QPen(Qt::red, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

        drawPoint(line.p0.x, line.p0.y, 4, QPen(Qt::red), QBrush(Qt::red, Qt::SolidPattern));
        drawPoint(line.p1.x, line.p1.y, 4, QPen(Qt::red), QBrush(Qt::red, Qt::SolidPattern));

    }

    for(iterLINE = YLines.begin(); iterLINE != YLines.end(); iterLINE++)
    {
        imgLine line = *iterLINE;

        Scene->addLine(line.p0.x, line.p0.y, line.p1.x, line.p1.y, QPen(Qt::green, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

        drawPoint(line.p0.x, line.p0.y, 4, QPen(Qt::green), QBrush(Qt::green, Qt::SolidPattern));
        drawPoint(line.p1.x, line.p1.y, 4, QPen(Qt::green), QBrush(Qt::green, Qt::SolidPattern));

    }

    for(iterLINE = ZLines.begin(); iterLINE != ZLines.end(); iterLINE++)
    {
        imgLine line = *iterLINE;

        Scene->addLine(line.p0.x, line.p0.y, line.p1.x, line.p1.y, QPen(Qt::blue, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

        drawPoint(line.p0.x, line.p0.y, 4, QPen(Qt::blue), QBrush(Qt::blue, Qt::SolidPattern));
        drawPoint(line.p1.x, line.p1.y, 4, QPen(Qt::blue), QBrush(Qt::blue, Qt::SolidPattern));

    }

    std::vector<connPoint>::iterator iterRPLANE;
    for(iterRPLANE = RPoints.begin(); iterRPLANE != RPoints.end(); iterRPLANE++)
    {
        connPoint connPt = *iterRPLANE;

        drawPoint(connPt.imgPt.x, connPt.imgPt.y, 6, QPen(Qt::black), QBrush(Qt::black, Qt::SolidPattern));
    }

    if (pointCache != NULL)
    {   
        drawPoint(pointCache->x, pointCache->y, 4, QPen(Qt::magenta), QBrush(Qt::magenta, Qt::SolidPattern));
    }

    if (origin != NULL)
    {
        drawPoint(origin->imgPt.x, origin->imgPt.y, 4, QPen(Qt::white), QBrush(Qt::white, Qt::SolidPattern));
    }

    if (X_Ref != NULL)
    {
        //cout << "uu" << endl;
        drawPoint(X_Ref->imgPt.x, X_Ref->imgPt.y, 4, QPen(Qt::white), QBrush(Qt::white, Qt::SolidPattern));
    }

    if (Y_Ref != NULL)
    {
        drawPoint(Y_Ref->imgPt.x, Y_Ref->imgPt.y, 4, QPen(Qt::white), QBrush(Qt::white, Qt::SolidPattern));
    }
    if (Z_Ref != NULL)
    {
        drawPoint(Z_Ref->imgPt.x, Z_Ref->imgPt.y, 4, QPen(Qt::white), QBrush(Qt::white, Qt::SolidPattern));
    }


    if (xVP != NULL)
    {
        drawPoint(xVP->x, xVP->y, 4, QPen(QColor(205, 79, 86,255)), QBrush(QColor(205, 79, 86,255), Qt::SolidPattern));
    }

    if (yVP != NULL)
    {
        drawPoint(yVP->x, yVP->y, 4, QPen(QColor(205, 79, 86,255)), QBrush(QColor(205, 79, 86,255), Qt::SolidPattern));
    }

    if (zVP != NULL)
    {
        drawPoint(zVP->x, zVP->y, 4, QPen(QColor(205, 79, 86,255)), QBrush(QColor(205, 79, 86,255), Qt::SolidPattern));
    }

    std::vector<connPoint>::iterator iterRHpt;
    for(iterRHpt = RHpoints.begin(); iterRHpt != RHpoints.end(); iterRHpt++)
    {
        connPoint connPt = *iterRHpt;
        drawPoint(connPt.imgPt.x, connPt.imgPt.y, 6, QPen(QColor(163, 3, 168,255)), QBrush(QColor(163, 3, 168,255), Qt::SolidPattern));
    }

    std::vector<connPoint>::iterator iterPoolpt;
    for(iterPoolpt = PtPool.begin(); iterPoolpt != PtPool.end(); iterPoolpt++)
    {
        connPoint connPt = *iterPoolpt;
        drawPoint(connPt.imgPt.x, connPt.imgPt.y, 6, QPen(QColor(5, 179, 250,255)), QBrush(QColor(5, 179, 250,255), Qt::SolidPattern));
    }

    if (state == _Polygon)
    {
        for(iterPoolpt =  PolyPool.begin(); iterPoolpt != PolyPool.end(); iterPoolpt++)
        {
            QGraphicsEllipseItem* polyPt;
            connPoint connPt = *iterPoolpt;
            polyPt = Scene->addEllipse(connPt.imgPt.x-10, connPt.imgPt.y-10, 20, 20, QPen(QColor(5, 179, 250,255)), QBrush(QColor(5, 179, 250,255), Qt::SolidPattern));
            QGraphicsOpacityEffect* dual_Eff = new QGraphicsOpacityEffect(this);
            dual_Eff->setOpacity(0.5);
            polyPt->setGraphicsEffect(dual_Eff);
        }
    }

    if (poolCache != NULL)
    {
        QGraphicsEllipseItem* prePoolpt;
        if (!_method)
        {
            prePoolpt = Scene->addEllipse(poolCache->imgPt.x-10, poolCache->imgPt.y-10, 20, 20, QPen(QColor(5, 179, 250,255)), QBrush(QColor(5, 179, 250,255), Qt::SolidPattern));
        } else {
            prePoolpt = Scene->addEllipse(poolCache->imgPt.x-10, poolCache->imgPt.y-10, 20, 20, QPen(Qt::white), QBrush(Qt::white, Qt::SolidPattern));
        }
        QGraphicsOpacityEffect* dual_Eff = new QGraphicsOpacityEffect(this);
        dual_Eff->setOpacity(0.5);
        prePoolpt->setGraphicsEffect(dual_Eff);
    }

    std::vector<connPoint>::iterator iterPoolbx;
    for(iterPoolbx = BxPool.begin(); iterPoolbx != BxPool.end(); iterPoolbx++)
    {
        connPoint connPt = *iterPoolbx;
        drawPoint(connPt.imgPt.x, connPt.imgPt.y, 6, QPen(QColor(5, 179, 250,255)), QBrush(QColor(5, 179, 250,255), Qt::SolidPattern));
    }
}

void SVMGraphicsView::keyPressEvent(QKeyEvent *event)
{
    if (_hasimage)
    {
        if (QApplication::keyboardModifiers() && Qt::AltModifier)
        {
            if (event->key() == Qt::Key_D)
            {
                switch(state)
                {
                    case _XLines:
                        reWindDel(XLines);
                        break;
                    case _YLines:
                        reWindDel(YLines);
                        break;
                    case _ZLines:
                        reWindDel(ZLines);
                        break;
                    /*
                    case _RPlane:
                        if(RPoints.size())
                        {
                            RPoints.pop_back();
                            drawScene();
                        }
                        break;
                    case _RHeight:
                        if(RHpoints.size())
                        {
                            RHpoints.pop_back();
                            drawScene();
                        }
                        break;
                    */
                    case _BxPool:
                        if(BxPool.size())
                        {
                            BxPool.pop_back();
                            drawScene();
                        }
                        break;
                    case _PtPool:
                        if(PtPool.size())
                        {
                            connPoint lstPlPt = PtPool.back();
                            if(poolCache != NULL && poolCache->imgPt.x == lstPlPt.imgPt.x && poolCache->imgPt.y == lstPlPt.imgPt.y)
                            {
                                poolCache = NULL;
                            }
                            PtPool.pop_back();
                            drawScene();
                        }
                        break;
                    case _Polygon:
                        if(PolyPool.size())
                        {
                            PolyPool.pop_back();
                            drawScene();
                        }
                        break;
                    default:
                        break;
                }
            }
            if (event->key() == Qt::Key_M)
            {
                popMoveDialog();
            }
            if (event->key() == Qt::Key_S)
            {
                if (state == _PtPool)
                {
                    _method = 1 - _method;
                    drawScene();
                } else if (state == _Polygon) {
                    _textureMethod = 1 - _textureMethod;
                }
            }
            /*if (event->key() == Qt::Key_P)
            {
                calPatch();
            }*/
            if (event->key() == Qt::Key_X)
            {
                calXVP();
            }
            if (event->key() == Qt::Key_Y)
            {
                calYVP();
            }
            if (event->key() == Qt::Key_Z)
            {
                calZVP();
            }
            if (event->key() == Qt::Key_H)
            {
                calHomography_Public();
            }

        }
        if (event->key() == Qt::Key_Equal)
        {
            gap *= 1.1;
        }
        if (event->key() == Qt::Key_Minus)
        {
            gap /= 1.1;
        }
    }
}

void SVMGraphicsView::popMoveDialog()
{

    QDialog dialog(this);
    // Use a layout allowing to have a label next to each field
    QFormLayout form(&dialog);

    // Add some text above the fields
    form.addRow(new QLabel("Cursor's new coordinate:"));

    // Add the lineEdits with their respective labels
    QList<QLineEdit *> fields;
    for(int i = 0; i < 2; ++i) {
        QLineEdit *lineEdit = new QLineEdit(&dialog);
        QString label;
        switch(i)
        {
            case 0:
                label = QString("X: ");
                lineEdit->setValidator(new QDoubleValidator());
                break;
            case 1:
                label = QString("Y: ");
                lineEdit->setValidator(new QDoubleValidator());
                break;
        }
        form.addRow(label, lineEdit);
        fields << lineEdit;
    }

    // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    // Show the dialog as modal
    double newx, newy;
    if (dialog.exec() == QDialog::Accepted) {
        int i = 0;
        // If the user didn't dismiss the dialog, do something with the fields
        foreach(QLineEdit * lineEdit, fields)
        {
            switch(i)
            {
                case 0:
                    newx = lineEdit->text().toDouble();
                    i++;
                    break;
                case 1:
                    newy = lineEdit->text().toDouble();
                    break;
            }
        }
        this->cursor().setPos(mapToGlobal(mapFromScene(newx, newy)));
    }
}

void SVMGraphicsView::reWindDel(std::vector<imgLine> &lineConatiner)
{
    if (pointCache != NULL)
    {
        pointCache = NULL;
    } else {
        int size = lineConatiner.size();
        if (size)
        {
            imgLine lstLine = lineConatiner[size-1];
            imgPoint newPtCache = lstLine.p0;
            pointCache = new imgPoint(newPtCache);
            lineConatiner.pop_back();
        }
    }
    drawScene();
}

void SVMGraphicsView::calXVP()
{
    if (_hasimage)
    {
        xVP = calVP(XLines);
        _xVP_exist = true;
        drawScene();
    } else {
        QMessageBox::warning(this, tr("Warning"), tr("Please load a picture and specify more than 2 lines in X direction !"));
    }
}

void SVMGraphicsView::calYVP()
{
    if (_hasimage)
    {
        yVP = calVP(YLines);
        _yVP_exist = true;
        drawScene();
    } else {
        QMessageBox::warning(this, tr("Warning"), tr("Please load a picture and specify more than 2 lines in Y direction !"));
    }
}

void SVMGraphicsView::calZVP()
{
    if (_hasimage)
    {
        zVP = calVP(ZLines);
        _zVP_exist = true;
        drawScene();
    } else {
        QMessageBox::warning(this, tr("Warning"), tr("Please load a picture and specify more than 2 lines in Z direction !"));
    }
}

imgPoint* SVMGraphicsView::calVP(std::vector<imgLine> &lineConatiner)
{
    int size = lineConatiner.size();
    if (size >= 2)
    {
        imgPoint* VP = new imgPoint;

        double *eig_val = (double *) calloc (3,sizeof(double));
        double **eig_vec = (double**) calloc (3,sizeof(double*));
        double **m = (double**) calloc (3,sizeof(double*));
        for (int i=0;i<3;i++) {
            m[i] = (double *) calloc (3,sizeof(double));
            eig_vec[i] = (double *) calloc (3, sizeof(double));
        }

        std::vector<imgLine>::iterator iterLINE;
        for(iterLINE = lineConatiner.begin(); iterLINE != lineConatiner.end(); iterLINE++)
        {
            imgLine line = *iterLINE;
            imgPoint *linePt = new imgPoint;
            linePt = crossProduct(line.p0, line.p1);
            double ptArray[3];
            ptArray[0] = linePt->x;
            ptArray[1] = linePt->y;
            ptArray[2] = linePt->w;
            for (int i=0; i<3; i++)
            {
                for (int j=0; j<3; j++)
                {
                    m[i][j] += ptArray[i] * ptArray[j];
                }
            }
        }

        cout << endl << "------------------------------------------------" << endl;
        for (int i=0; i<3; i++)
        {
            for (int j=0; j<3; j++)
            {
                cout << m[i][j] << "    ";
            }
            cout << endl;
        }

        Matrix3d mEig3;
        for (int i=0; i<3; i++)
            for (int j=0; j<3; j++)
                mEig3(i, j) = m[i][j];

        SelfAdjointEigenSolver<Matrix3d> es;
        es.compute(mEig3);
        cout << "\n Eigen Values: \n" << es.eigenvalues() << endl;
        cout << "\n Eigen Vectors: \n" << es.eigenvectors() << endl;

        Vector3d v = es.eigenvectors().col(0);
        VP->x = v(0)/v(2);
        VP->y = v(1)/v(2);
        VP->w = 1;

// ----------------------------------------------------------------------------


        for (int i=0;i<3;i++) {
            free(m[i]);
            free(eig_vec[i]);
        }
        free(m);
        free(eig_vec);
        free(eig_val);

        cout << endl << "Vanishing point position: (" << VP->x << ", " << VP->y << ")" << endl << "------------------------------------------------" << endl << endl;
        return VP;
    } else {
        QMessageBox::warning(this, tr("Warning"), tr("Please specify more than 2 lines in this direction !"));
    }
    return NULL;
}

imgPoint* SVMGraphicsView::crossProduct(imgPoint p0, imgPoint p1)
{
    imgPoint* lnPoint = new imgPoint;
    lnPoint->x = p0.y * p1.w - p0.w * p1.y;
    lnPoint->y = p1.x * p0.w - p0.x * p1.w;
    lnPoint->w = p0.x * p1.y - p0.y * p1.x;
    homoFy(lnPoint);
//    cout << p0.x << ", "<< p0.y << ", " << p0.w << ", " << p1.x << ", " << p1.y << ", " << p1.w << ", " << lnPoint->x << ", " << lnPoint->y << ", " << lnPoint->w << endl << "+++++++" << endl;
    return lnPoint;
}

void SVMGraphicsView::normaLize(imgPoint* pt)
{
    double mod = sqrt(pt->x * pt->x + pt->y * pt->y + pt->w * pt->w);
    pt->x /= mod;
    pt->y /= mod;
    pt->w /= mod;
}

void SVMGraphicsView::homoFy(imgPoint* pt)
{
    pt->x /= pt->w;
    pt->y /= pt->w;
    pt->w = 1;
}

void SVMGraphicsView::eigenJacobianTest()
{
    double *eig_val = (double *) calloc (3,sizeof(double));
    double **eig_vec = (double**) calloc (3,sizeof(double*));
    double **m = (double**) calloc (3,sizeof(double*));
    for (int i=0;i<3;i++) {
        m[i] = (double *) calloc (3,sizeof(double));
        eig_vec[i] = (double *) calloc (3, sizeof(double));
    }
    m[0][0] = 2; m[0][1] = 1; m[0][2] = 1;
    m[1][0] = 1; m[1][1] = 2; m[1][2] = 1;
    m[2][0] = 1; m[2][1] = 1; m[2][2] = 2;
    eig_sys (3,m,eig_vec,eig_val);
    cout << "eigenvalues " ;
    for (int i=0;i<3;i++) cout << eig_val[i] << " ";
    cout << endl;
    cout << "eigenvectors";
    for (int i=0;i<3;i++) {
        cout << endl;
        for (int j=0;j<3;j++)
            cout << eig_vec[i][j] << " " ;
    }
    cout << endl;

    for (int i=0;i<3;i++) {
        free(m[i]);
        free(eig_vec[i]);
    }
    free(m);
    free(eig_vec);
    free(eig_val);
}

void SVMGraphicsView::eigen3Test()
{
    MatrixXd A = MatrixXd::Random(100,100);
    MatrixXd b = MatrixXd::Random(100,50);
    MatrixXd x = A.fullPivLu().solve(b);
    double relative_error = (A*x - b).norm() / b.norm(); // norm() is L2 norm
    cout << "The relative error is:\n" << relative_error << endl;
}

void SVMGraphicsView::calHomography_Public()
{
    if(_hasimage)
    {
        calculateProj();
        cout << Proj << endl;

        Vector4d orgCord;
        orgCord(0) = 0.0;
        orgCord(1) = 0.0;
        orgCord(2) = 0.0;
        orgCord(3) = 1.0;
        Vector3d imgOrgCord = Proj * orgCord;
        cout << "Img cord of origin: " << imgOrgCord << endl;

        int new_x = Proj(9);//-imgOrgCord(0);
        int new_y = Proj(10);//+imgOrgCord(1);
        int new_x_0 = Proj(9);//+imgOrgCord(0);
        //added by Sophia
        Hxy = (cv::Mat_<double>(3, 3) << Proj(0), Proj(3), new_x,
                                         Proj(1), Proj(4), new_y,
                                         Proj(2), Proj(5), Proj(11));
        Hxz = (cv::Mat_<double>(3, 3) << Proj(0), Proj(6), new_x,
                                         Proj(1), Proj(7), new_y,
                                         Proj(2), Proj(8), Proj(11));
        Hyz = (cv::Mat_<double>(3, 3) << Proj(3), Proj(6), new_x_0,
                                         Proj(4), Proj(7), new_y,
                                         Proj(5), Proj(8), Proj(11));


        refHomoS2I << Proj(0), Proj(3), Proj(9),
                      Proj(1), Proj(4), Proj(10),
                      Proj(2), Proj(5), Proj(11);
        refHomoI2S = refHomoS2I.inverse();


        /*
        popIfLoadVPDialog();
        // calHomography(RPoints);   // use self programmed LU solver, performance not good
        calHomographyEig3(RPoints); // use eigen3 package, very good performance
        */
    } else {
        QMessageBox::warning(this, tr("Warning"), tr("Please load a picture and specify a plane first!"));
    }
}

void SVMGraphicsView::popIfLoadVPDialog()
{

    QDialog dialog(this);
    // Use a layout allowing to have a label next to each field
    QFormLayout form(&dialog);

    // Add some text above the fields
    form.addRow(new QLabel("Take X and Y vanishing points into consideration at this step if they can be and already been calculated?"));

    // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    _extraRPt = 0;
    if (dialog.exec() == QDialog::Accepted) {
        if (_xVP_exist) _extraRPt++;
        if (_yVP_exist) _extraRPt++;
    }

}

void SVMGraphicsView::calHomographyEig3(std::vector<connPoint> &planePts)
{
    int refPtN =  planePts.size() + _extraRPt;
    if (refPtN < 4)
    {
        QMessageBox::warning(this, tr("Warning"), tr("Please specify at least 4 points (including vanishing points on this plane) of a POLYGON on this plane !"));
    } else {
        MatrixXd A(2*refPtN, 8);
        VectorXd b(2*refPtN);
        std::vector<connPoint>::iterator iterRPt;
        int i = 0;
        for (iterRPt=planePts.begin(); iterRPt!=planePts.end(); iterRPt++, i++)
        {
            connPoint refPt = *iterRPt;
            b(2*i) = refPt.scnPt.x;
            b(2*i+1) = refPt.scnPt.y;

            A(2*i, 0) = refPt.imgPt.x;
            A(2*i, 1) = refPt.imgPt.y;
            A(2*i, 2) = 1;
            A(2*i, 3) = 0;
            A(2*i, 4) = 0;
            A(2*i, 5) = 0;
            A(2*i, 6) = -refPt.imgPt.x * refPt.scnPt.x;
            A(2*i, 7) = -refPt.imgPt.y * refPt.scnPt.x;

            A(2*i+1, 0) = 0;
            A(2*i+1, 1) = 0;
            A(2*i+1, 2) = 0;
            A(2*i+1, 3) = refPt.imgPt.x;
            A(2*i+1, 4) = refPt.imgPt.y;
            A(2*i+1, 5) = 1;
            A(2*i+1, 6) = -refPt.imgPt.x * refPt.scnPt.y;
            A(2*i+1, 7) = -refPt.imgPt.y * refPt.scnPt.y;
        }


        if (_extraRPt)
        {
            if (_xVP_exist)
            {
                b(2*i) = 0;
                b(2*i+1) = -1;

                A(2*i, 0) = 0;
                A(2*i, 1) = 0;
                A(2*i, 2) = 0;
                A(2*i, 3) = xVP->x;
                A(2*i, 4) = xVP->y;
                A(2*i, 5) = 1;
                A(2*i, 6) = 0;
                A(2*i, 7) = 0;

                A(2*i+1, 0) = 0;
                A(2*i+1, 1) = 0;
                A(2*i+1, 2) = 0;
                A(2*i+1, 3) = 0;
                A(2*i+1, 4) = 0;
                A(2*i+1, 5) = 0;
                A(2*i+1, 6) = xVP->x;
                A(2*i+1, 7) = xVP->y;
            }

            i++;

            if (_yVP_exist)
            {
                b(2*i) = 0;
                b(2*i+1) = -1;

                A(2*i, 0) = yVP->x;
                A(2*i, 1) = yVP->y;
                A(2*i, 2) = 1;
                A(2*i, 3) = 0;
                A(2*i, 4) = 0;
                A(2*i, 5) = 0;
                A(2*i, 6) = 0;
                A(2*i, 7) = 0;

                A(2*i+1, 0) = 0;
                A(2*i+1, 1) = 0;
                A(2*i+1, 2) = 0;
                A(2*i+1, 3) = 0;
                A(2*i+1, 4) = 0;
                A(2*i+1, 5) = 0;
                A(2*i+1, 6) = yVP->x;
                A(2*i+1, 7) = yVP->y;
            }
        }



        VectorXd x = A.fullPivLu().solve(b);
        cout << "\n A: \n" << A << endl;
        cout << "\n b: \n" << b << endl;
        Eigen::MatrixXd Ab(2*refPtN, 9);

        for (int i=0; i<2*refPtN; i++)
        {
            for (int j=0; j<8; j++)
                Ab(i, j) = A(i, j);
            Ab(i, 8) = b(i);
        }

        FullPivLU<MatrixXd> lu(Ab);
        lu.setThreshold(1e-5);
        cout << "\n Rank of [A b]: \n" << lu.rank() << endl;
        cout << "\n x: \n" << x << endl;
        cout << "\n A*x: \n" << A*x << endl;

        Matrix3d homo;
        for (int i=0; i<8; i++) homo(i/3, i%3) = x(i);
        homo(2,2) = 1;
        refHomoI2S = homo;
        refHomoS2I = homo.inverse();
        _refHomo_exist = true;

        cout << "\n Homography matrix (from image to scene): \n" << refHomoI2S << endl;

        cout << "\n Homography matrix (from scene to image): \n" << refHomoS2I << endl;

        i = 1;
        for (iterRPt=planePts.begin(); iterRPt!=planePts.end(); iterRPt++, i++)
        {
            (*iterRPt).homoMatrixEig3 = homo;
            Vector3d imgPtVec = imgPointToVector3d((*iterRPt).imgPt);
            cout << "------------Point " << i <<" ------------- \n ImageCoord: \n" << imgPtVec << endl;
            Vector3d scnPtFromImg = homo * imgPtVec;
            scnPtFromImg /= scnPtFromImg(2);
            cout << "\n HomoMat * ImageCoord: \n" << scnPtFromImg << endl;
            cout << "\n SceneCoord: \n" << scnPointToVector3d((*iterRPt).scnPt) << endl;
        }
        if (_extraRPt)
        {
            if (_xVP_exist)
            {
                Vector3d imgPtVec = imgPointToVector3d(*xVP);
                cout << "------------Point " << i <<" ------------- \n ImageCoord: \n" << imgPtVec << endl;
                Vector3d scnPtFromImg = homo * imgPtVec;
            //    scnPtFromImg /= scnPtFromImg(2);
                cout << "\n HomoMat * ImageCoord: \n" << scnPtFromImg << endl;
                Vector3d xVPScn;
                xVPScn << 1, 0, 0;
                cout << "\n SceneCoord: \n" << xVPScn << endl;
            }
            i++;
            if (_yVP_exist)
            {
                Vector3d imgPtVec = imgPointToVector3d(*yVP);
                cout << "------------Point " << i <<" ------------- \n ImageCoord: \n" << imgPtVec << endl;
                Vector3d scnPtFromImg = homo * imgPtVec;
            //    scnPtFromImg /= scnPtFromImg(2);
                cout << "\n HomoMat * ImageCoord: \n" << scnPtFromImg << endl;
                Vector3d yVPScn;
                yVPScn << 0, 1, 0;
                cout << "\n SceneCoord: \n" << yVPScn << endl;
            }
        }

    }
}

Vector3d SVMGraphicsView::imgPointToVector3d(imgPoint imgPt)
{
    Vector3d vecPt;
    vecPt(0) = imgPt.x;
    vecPt(1) = imgPt.y;
    vecPt(2) = imgPt.w;
    return vecPt;
}

Vector3d SVMGraphicsView::scnPointToVector3d(scnPoint scnPt)
{
    Vector3d vecPt;
    vecPt(0) = scnPt.x;
    vecPt(1) = scnPt.y;
    vecPt(2) = scnPt.w;
    return vecPt;
}

void SVMGraphicsView::calGamma()
{
    if (_hasimage)
    {
        if (_refHomo_exist && (RHpoints.size() == 2) && _zVP_exist)
        {
            Vector3d P1, P2, O, Vz, b, t;
            P1 = refHomoS2I.col(0);
            P2 = refHomoS2I.col(1);
            O = refHomoS2I.col(2);
            Vz = imgPointToVector3d(*zVP);

            connPoint bConn = RHpoints[1];
            b = imgPointToVector3d(bConn.imgPt);
            connPoint tConn = RHpoints[0];
            t = imgPointToVector3d(tConn.imgPt);
            double z = bConn.scnPt.z;
            double deltaz = tConn.scnPt.z - z;

            cout << "++++++++++++ All 4 Check ++++++++++++++";
            cout << "\n P1: \n" << P1 << "\n\n P2: \n" << P2 << "\n\n O: \n" << O << "\n\n Vz: \n" << Vz << "\n\n b: \n" << b << "\n\n t: \n" << t << "\n\n z: " << z << " deltaz: " << deltaz << endl;

            int sign = 1;
            if ((b.cross(t).dot(Vz.cross(t))) > 0) sign = -1;
            gammaZ = sign * ((P1.cross(P2)).dot(O)) * (b.cross(t)).norm() / (deltaz * ((P1.cross(P2)).dot(b)) * (Vz.cross(t)).norm() + z * ((P1.cross(P2)).dot(Vz)) * (b.cross(t)).norm());
            gammaZ *= sign;

            cout << "\n gammaZ: \n " << gammaZ << endl;
            _getGammaZ = true;

            Proj << P1, P2, gammaZ * Vz, O;
            cout << "\n Projection Matrix from scene to image: \n" << Proj << endl;

        } else {
            QMessageBox::warning(this, tr("Warning"), tr("Please first calculate Z vanishing point, homography matrix of XY plane and give a reference off-plane point (x, y ,z) or 2 reference z-colinear points with only z !"));
        }
    } else {
        QMessageBox::warning(this, tr("Warning"), tr("Please load a picture, specify a plane and an off-plane height first!"));
    }
}

void SVMGraphicsView::scnMapToImg()
{
    if (_hasimage)
    {
        if (_getGammaZ)
        {
            drawScene();

            QDialog dialog(this);
            // Use a layout allowing to have a label next to each field
            QFormLayout form(&dialog);

            // Add some text above the fields
            form.addRow(new QLabel("Please input the Image coordinates:"));

            // Add the lineEdits with their respective labels
            QList<QLineEdit *> fields;
            for(int i = 0; i < 3; ++i) {
                QLineEdit *lineEdit = new QLineEdit(&dialog);
                QString label;
                switch(i)
                {
                    case 0:
                        label = QString("X: ");
                        lineEdit->setValidator(new QDoubleValidator());
                        break;
                    case 1:
                        label = QString("Y: ");
                        lineEdit->setValidator(new QDoubleValidator());
                        break;
                    default:
                        label = QString("Z: ");
                        lineEdit->setValidator(new QDoubleValidator());
                        break;
                }
                form.addRow(label, lineEdit);
                fields << lineEdit;
            }

            // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
            QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                       Qt::Horizontal, &dialog);
            form.addRow(&buttonBox);
            QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
            QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

            // Show the dialog as modal
            if (dialog.exec() == QDialog::Accepted) {
                Vector4d scnCord;

                int i = 0;
                foreach(QLineEdit * lineEdit, fields)
                {
                    scnCord(i) = lineEdit->text().toDouble();
                    i++;
                }
                scnCord(i) = 1;
                Vector3d imgCord = Proj * scnCord;
                imgCord /= imgCord(2);
                cout << "\n Scene coordinates: \n" << scnCord << "\n Image coordinates: \n" << imgCord << endl;

                QGraphicsEllipseItem* imgTarget;
                imgTarget = Scene->addEllipse(imgCord(0)-3, imgCord(1)-3, 6, 6, QPen(QColor(5, 179, 250,255)), QBrush(QColor(5, 179, 250,255), Qt::SolidPattern));
                QGraphicsOpacityEffect* dual_Eff = new QGraphicsOpacityEffect(this);
                dual_Eff->setOpacity(0.5);
                imgTarget->setGraphicsEffect(dual_Eff);
            }
        } else {
            QMessageBox::warning(this, tr("Warning"), tr("Please calculate all the staffs first!"));
        }
    } else {
        QMessageBox::warning(this, tr("Warning"), tr("Please load a picture and calculate all the staffs first!"));
    }
}

void SVMGraphicsView::make3DPtPool()
{
    if (_hasimage)
    {
        if (_getGammaZ)
        {
            state = _PtPool;
            drawScene();
            cout << "\n Please pick up all the points which will be used to form polygons later, start from one point in the reference plane. \n"
                 << "You can change method state afterwards. Begin with co-plane methods. \"Alt + S\" could switch between co-plane and vertical methods." << endl;
            poolCache = NULL;
            _method = 0;
        } else {
            QMessageBox::warning(this, tr("Warning"), tr("Please calculate all the staffs first!"));
        }
    } else {
        QMessageBox::warning(this, tr("Warning"), tr("Please load a picture first !"));
    }
}

//for warping img
void SVMGraphicsView::make3DBxPool()
{
    if (_hasimage)
    {
        if (_getGammaZ)
        {
            state = _BxPool;
            drawScene();
            cout << "\n Please pick up two points (Top-left and Bottom-right) which will be used to wrap your target area later." << endl;
            poolCache = NULL;
            //_method = 0;
        } else {
            QMessageBox::warning(this, tr("Warning"), tr("Please calculate all the staffs first!"));
        }
    } else {
        QMessageBox::warning(this, tr("Warning"), tr("Please load a picture first !"));
    }
}

void SVMGraphicsView::pushBxPool(double x, double y)//modified
{
    connPoint* newPollPt;
    newPollPt = new connPoint;
    newPollPt->imgPt.x = x;
    newPollPt->imgPt.y = y;
    newPollPt->imgPt.w = 1;
    newPollPt->scnPt.w = 1;

    if (!BxPool.size())
    {

        newPollPt->homoMatrixEig3 = refHomoI2S;
        newPollPt->scnPt.z = 0;
        Vector3d scnCord = newPollPt->homoMatrixEig3 * imgPointToVector3d(newPollPt->imgPt);
        //Vector3d scnCord = Proj * imgPointToVector3d(newPollPt->imgPt);
        scnCord /= scnCord(2);
        newPollPt->scnPt.x = scnCord(0);
        newPollPt->scnPt.y = scnCord(1);

        cout << "Image Coordinates: (" << newPollPt->imgPt.x << ", " << newPollPt->imgPt.y <<")" << endl;
        cout << "Scene Coordinates: (" << newPollPt->scnPt.x << ", " << newPollPt->scnPt.y << ", " << 0 << ")" << endl;
        BxPool.push_back(*newPollPt);
        poolCache = newPollPt;
    } else {
        if (poolCache == NULL)
        {
            std::vector<connPoint>::iterator iterPoolpt;
            for(iterPoolpt = BxPool.begin(); iterPoolpt != BxPool.end(); iterPoolpt++)
            {
                connPoint connPt = *iterPoolpt;
                if (x == connPt.imgPt.x && y == connPt.imgPt.y)
                {
                    newPollPt->scnPt.x = connPt.scnPt.x;
                    newPollPt->scnPt.y = connPt.scnPt.y;
                    newPollPt->scnPt.z = connPt.scnPt.z;
                    newPollPt->homoMatrixEig3 = connPt.homoMatrixEig3;
                    poolCache = newPollPt;
                    break;
                }
            }
            cout<< "Please select a previous point (Ctrl + LClick) first !" << endl;
        } else {
            if (!_method)    // 0 coplane 1 vertical
            {
                newPollPt->scnPt.z = poolCache->scnPt.z;
                newPollPt->homoMatrixEig3 = poolCache->homoMatrixEig3;
                Vector3d scnCord = newPollPt->homoMatrixEig3 * imgPointToVector3d(newPollPt->imgPt);
                //Vector3d scnCord = Proj * imgPointToVector3d(newPollPt->imgPt);
                scnCord /= scnCord(2);
                newPollPt->scnPt.x = scnCord(0);
                newPollPt->scnPt.y = scnCord(1);
            } else {
                newPollPt->scnPt.x = poolCache->scnPt.x;
                newPollPt->scnPt.y = poolCache->scnPt.y;

                Vector3d P1, P2, P3, O, Vz, b, bscn, t;
                P1 = Proj.col(0);
                P2 = Proj.col(1);
                P3 = Proj.col(2);
                O = Proj.col(3);
                Vz = imgPointToVector3d(*zVP);
                bscn << newPollPt->scnPt.x, newPollPt->scnPt.y, 1;
                b = refHomoS2I * bscn;
                b /= b(2);
                t = imgPointToVector3d(newPollPt->imgPt);
                int sign = 1;
                if ((b.cross(t).dot(Vz.cross(t))) > 0) sign = -1;
                double z;

                z = sign * ((P1.cross(P2)).dot(O)) * (b.cross(t)).norm() / (gammaZ * ((P1.cross(P2)).dot(b)) * (Vz.cross(t)).norm());

                newPollPt->scnPt.z = z;

                Matrix3d P;
                P << P1, P2, O + P3 * z;
                Matrix3d zHomo = P.inverse();
                newPollPt->homoMatrixEig3 = zHomo;

                _method = 0;
            }
            cout << "Image Coordinates: (" << newPollPt->imgPt.x << ", " << newPollPt->imgPt.y <<")" << endl;
            cout << "Scene Coordinates: (" << newPollPt->scnPt.x << ", " << newPollPt->scnPt.y << ", " << newPollPt->scnPt.z << ")" << endl;
            BxPool.push_back(*newPollPt);
            poolCache = newPollPt;
        }
    }

    drawScene();
}

void SVMGraphicsView::pushPtPool(double x, double y)//modified
{
    connPoint* newPollPt;
    newPollPt = new connPoint;
    newPollPt->imgPt.x = x;
    newPollPt->imgPt.y = y;
    newPollPt->imgPt.w = 1;
    newPollPt->scnPt.w = 1;

    if (!PtPool.size())
    {

        newPollPt->homoMatrixEig3 = refHomoI2S;
        newPollPt->scnPt.z = 0;
        Vector3d scnCord = newPollPt->homoMatrixEig3 * imgPointToVector3d(newPollPt->imgPt);
        //Vector3d scnCord = Proj * imgPointToVector3d(newPollPt->imgPt);
        scnCord /= scnCord(2);
        newPollPt->scnPt.x = scnCord(0);
        newPollPt->scnPt.y = scnCord(1);

        cout << "Image Coordinates: (" << newPollPt->imgPt.x << ", " << newPollPt->imgPt.y <<")" << endl;
        cout << "Scene Coordinates: (" << newPollPt->scnPt.x << ", " << newPollPt->scnPt.y << ", " << 0 << ")" << endl;
        PtPool.push_back(*newPollPt);
        poolCache = newPollPt;
    } else {
        if (poolCache == NULL)
        {
            std::vector<connPoint>::iterator iterPoolpt;
            for(iterPoolpt = PtPool.begin(); iterPoolpt != PtPool.end(); iterPoolpt++)
            {
                connPoint connPt = *iterPoolpt;
                if (x == connPt.imgPt.x && y == connPt.imgPt.y)
                {
                    newPollPt->scnPt.x = connPt.scnPt.x;
                    newPollPt->scnPt.y = connPt.scnPt.y;
                    newPollPt->scnPt.z = connPt.scnPt.z;
                    newPollPt->homoMatrixEig3 = connPt.homoMatrixEig3;
                    poolCache = newPollPt;
                    break;
                }
            }
            cout<< "Please select a previous point (Ctrl + LClick) first !" << endl;
        } else {
            if (!_method)    // 0 coplane 1 vertical
            {
                newPollPt->scnPt.z = poolCache->scnPt.z;
                newPollPt->homoMatrixEig3 = poolCache->homoMatrixEig3;
                Vector3d scnCord = newPollPt->homoMatrixEig3 * imgPointToVector3d(newPollPt->imgPt);
                //Vector3d scnCord = Proj * imgPointToVector3d(newPollPt->imgPt);
                scnCord /= scnCord(2);
                newPollPt->scnPt.x = scnCord(0);
                newPollPt->scnPt.y = scnCord(1);
            } else {
                newPollPt->scnPt.x = poolCache->scnPt.x;
                newPollPt->scnPt.y = poolCache->scnPt.y;

                Vector3d P1, P2, P3, O, Vz, b, bscn, t;
                P1 = Proj.col(0);
                P2 = Proj.col(1);
                P3 = Proj.col(2);
                O = Proj.col(3);
                Vz = imgPointToVector3d(*zVP);
                bscn << newPollPt->scnPt.x, newPollPt->scnPt.y, 1;
                b = refHomoS2I * bscn;
                b /= b(2);
                t = imgPointToVector3d(newPollPt->imgPt);
                int sign = 1;
                if ((b.cross(t).dot(Vz.cross(t))) > 0) sign = -1;
                double z;

                z = sign * ((P1.cross(P2)).dot(O)) * (b.cross(t)).norm() / (gammaZ * ((P1.cross(P2)).dot(b)) * (Vz.cross(t)).norm());

                newPollPt->scnPt.z = z;

                Matrix3d P;
                P << P1, P2, O + P3 * z;
                Matrix3d zHomo = P.inverse();
                newPollPt->homoMatrixEig3 = zHomo;

                _method = 0;
            }
            cout << "Image Coordinates: (" << newPollPt->imgPt.x << ", " << newPollPt->imgPt.y <<")" << endl;
            cout << "Scene Coordinates: (" << newPollPt->scnPt.x << ", " << newPollPt->scnPt.y << ", " << newPollPt->scnPt.z << ")" << endl;
            PtPool.push_back(*newPollPt);
            poolCache = newPollPt;
        }
    }

    drawScene();
}

void SVMGraphicsView::pickPolygon()
{
    if (_hasimage)
    {
        if (_getGammaZ && PtPool.size() >= 3)
        {
            poolCache = NULL;
            drawScene();
            cout << "\n Please select at least 3 points for each plane and make patch by press \"Alt + P\" or click \"Texture\" in \"Calculation\" menu, you can change TextureMethod (BoundingBox method or Polygon method) with \"Alt + S\", default: BoundingBox method" << endl;
            state = _Polygon;
            _textureMethod = 0;
            PolyPool.clear();
        } else {
            QMessageBox::warning(this, tr("Warning"), tr("Please calculate all the staffs and make point pool ready (at least 3 points) !"));
        }
    } else {
        QMessageBox::warning(this, tr("Warning"), tr("Please load a picture first !"));
    }

}

void SVMGraphicsView::pushPolyPool(double x, double y)
{
    std::vector<connPoint>::iterator iterPoolpt;
    for(iterPoolpt = PtPool.begin(); iterPoolpt != PtPool.end(); iterPoolpt++)
    {
        connPoint connPt = *iterPoolpt;
        if (x == connPt.imgPt.x && y == connPt.imgPt.y)
        {
            PolyPool.push_back(connPt);
            drawScene();
            break;
        }
    }
}

void SVMGraphicsView::calHxy()
{
    if (_hasimage)
    {
        state = _Hxy;
        //makeSubH(Hxy, false);
    }
    else
        QMessageBox::warning(this, tr("Warning"), tr("Please load a picture, calculate all the stuff and pick up some points on a plane first!"));
}

void SVMGraphicsView::calHxz()
{
    if (_hasimage)
    {
        state = _Hxz;
        //makeSubH(Hxz, false);
    }
    else
        QMessageBox::warning(this, tr("Warning"), tr("Please load a picture, calculate all the stuff and pick up some points on a plane first!"));

}

void SVMGraphicsView::calHyz()
{
    if (_hasimage)
    {
        state = _Hyz;
        //makeSubH(Hyz, true);
    }
    else
        QMessageBox::warning(this, tr("Warning"), tr("Please load a picture, calculate all the stuff and pick up some points on a plane first!"));

}

void SVMGraphicsView::calH()
{
    if(state == _Hxy)
        makeSubH(Hxy, false);
    else if(state == _Hxz)
        makeSubH(Hxz, false);
    else if(state == _Hyz)
        makeSubH(Hyz, true);
}
void SVMGraphicsView::makeSubH(cv::Mat H, bool flip)
{

    drawScene();
    patchConfirmDlg->scnPoints.clear();
    patchConfirmDlg->texPoints.clear();

    std::vector<connPoint>::iterator iter;
    QPolygonF region;
    double bBoxXmin, bBoxXmax, bBoxYmin, bBoxYmax;
    std::vector<Vector3d> rotatedPts;
    for(iter = PolyPool.begin(); iter != PolyPool.end(); iter++)
    {
        connPoint pt = *iter;

        Vector3d pt3DCord;
        pt3DCord << pt.scnPt.x, pt.scnPt.y, pt.scnPt.z;
        patchConfirmDlg->scnPoints.push_back(pt3DCord);

        Vector3d rPt;
        if(state == _Hxy)
        {
            rPt(0) = pt.scnPt.x;
            rPt(1) = pt.scnPt.y;
        }
        else if(state == _Hxz)
        {
            rPt(0) = pt.scnPt.x;
            rPt(1) = pt.scnPt.z;
        }
        else if(state == _Hyz)
        {
            rPt(0) = pt.scnPt.y;
            rPt(1) = pt.scnPt.z;
        }
        rPt(2) = 0;
        //cout << "rPt"  << pt.scnPt.x << endl;
        //cout << "rPt"  << pt.scnPt.y << endl;
        //cout << "rPt"  << pt.scnPt.z << endl;
        //cout << endl << rPt(0) << endl;
        //rPt = rotate(pt.scnPt.x, pt.scnPt.y, pt.scnPt.z, R, theta);
        rotatedPts.push_back(rPt);

        QPointF qpt(rPt(0), rPt(1));
        region << qpt;

        if(iter == PolyPool.begin())
        {
            bBoxXmin = rPt(0);
            bBoxXmax = rPt(0);
            bBoxYmin = rPt(1);
            bBoxYmax = rPt(1);
        } else {
            bBoxXmin = min(bBoxXmin, rPt(0));
            bBoxXmax = max(bBoxXmax, rPt(0));
            bBoxYmin = min(bBoxYmin, rPt(1));
            bBoxYmax = max(bBoxYmax, rPt(1));
        }
    }

    double scale = 1;
    int oX = floor(bBoxXmin/scale);
    int oY = floor(bBoxYmin/scale);
    int ow = ceil(bBoxXmax/scale) - oX + 1;
    int oh = ceil(bBoxYmax/scale) - oY + 1;
    //cout << "width"  << ow << endl;
    //cout << "height" << oh << endl;
    std::vector<Vector3d>::iterator iterR;
    for(iterR = rotatedPts.begin(); iterR != rotatedPts.end(); iterR++)
    {
        Vector3d patchPt = *iterR;
        //cout << patchPt(0) << endl;
        //cout << patchPt(1) << endl;
        patchPt(0) = (patchPt(0) - oX * scale) / ((ow - 1) * scale);
        patchPt(1) = ((oh - 1) * scale - (patchPt(1) - oY * scale)) / ((oh - 1) * scale);
        Vector2d texCord;
        texCord << patchPt(0), patchPt(1);
        patchConfirmDlg->texPoints.push_back(texCord);
    }

    //Get subimage range
    int width = image.width();
    int height = image.height();
    std::vector<connPoint>::iterator iterBx;
    double subImgXmin , subImgXmax , subImgYmin , subImgYmax;
    if(BxPool.size() >= 2){
        for(iterBx = BxPool.begin(); iterBx != BxPool.end(); iterBx++){
            connPoint pt = *iterBx;
            double pt1 = pt.imgPt.x;
            double pt2 = pt.imgPt.y;
            if(iterBx == BxPool.begin())
            {
                subImgXmin = pt1;
                subImgXmax = pt1;
                subImgYmin = pt2;
                subImgYmax = pt2;
            } else {
                subImgXmin = min(subImgXmin, pt1);
                subImgXmax = max(subImgXmax, pt1);
                subImgYmin = min(subImgYmin, pt2);
                subImgYmax = max(subImgYmax, pt2);
            }
        }
    }
    else{
        subImgXmin = 0;
        subImgXmax = width;
        subImgYmin = 0;
        subImgYmax = height;
    }
    cout << "Current bounding Point #: " << BxPool.size() << endl;
    cout << subImgXmin << " " << subImgXmax << " " << subImgYmin << " " << subImgYmax << endl;
    BxPool.clear();

    QImage patchImg(width, height, QImage::Format_ARGB32_Premultiplied);
    patchImg.fill(Qt::transparent);

    QImage image_copy = image.convertToFormat(QImage::Format_RGB32);
    cv::Mat src = ASM::QImageToCvMat(image_copy);
    if(src.empty()){
        cout << "can't open image" << endl;
    }
    else{
        cout << H << endl;
        cv::Mat corner, new_corner,translationMatrix, new_H;
        cv::Rect box;
        double bx, by, bwidth, bheight;
        //new_height = height - 1;
        //new_width = width - 1;
        std::vector<cv::Point2f> corners, new_corners;
        cv::Point2f pt1(subImgXmin,subImgYmin),pt2(subImgXmin,subImgYmax-1),pt3(subImgXmax-1, subImgYmax-1), pt4(subImgXmax-1, subImgYmin);
        corners.push_back(pt1);
        corners.push_back(pt2);
        corners.push_back(pt3);
        corners.push_back(pt4);


        cv::perspectiveTransform(corners, new_corners, H.inv());
        box = cv::boundingRect(new_corners);

        bx = box.x;
        bwidth = box.width;
        by = box.y;
        bheight = box.height;
        cout << bx << " " << by << " " << bheight << " " << bwidth << endl;
        translationMatrix = (cv::Mat_<double>(3,3) << 1, 0, -bx,
                                                      0, 1, -by,
                                                      0, 0, 1);
        new_H = (translationMatrix * H.inv()).inv();

        cv::perspectiveTransform(corners, new_corners, new_H.inv());
        box = cv::boundingRect(new_corners);
        bx = box.x;
        bwidth = box.width;
        by = box.y;
        bheight = box.height;
        cout << bx << " " << by << " " << bheight << " " << bwidth << endl;

        cv::Mat dst;//, flip_dst;
        cv::warpPerspective(src, dst, new_H, cv::Size(bwidth, bheight), cv::WARP_INVERSE_MAP);

        //if(flip)
        //    cv::flip(dst,flip_dst, -1);
        //else
        //    cv::flip(dst, flip_dst, 0);
        //patchImg = ASM::cvMatToQImage(flip_dst);
        patchImg = ASM::cvMatToQImage(dst);
    }

    patchConfirmDlg->patchImg = patchImg;
    patchConfirmDlg->popout();
}

void SVMGraphicsView::saveModel()
{
    if(_hasimage)
    {
        QString sourcePath = QDir::currentPath() + "/patches/wrl.tmp";
        if(QFile::exists(sourcePath))
        {

            QByteArray fileFormat("wrl");

            QString destPath = QDir::currentPath() + "/patches/untitled." + fileFormat;

            QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),
                                                            destPath,
                                                            tr("%1 Files (*.%2);;")
                                                            .arg(QString::fromLatin1(fileFormat.toUpper()))
                                                            .arg(QString::fromLatin1(fileFormat)));
            if (!fileName.isEmpty()) {
                QFile::copy(sourcePath, destPath);
                cout << "\nSave model: " << fileName.toStdString() << endl;
            }
        } else {
            QMessageBox::warning(this, tr("Warning"), tr("Please build all the patches first!"));
        }
    } else {
        QMessageBox::warning(this, tr("Warning"), tr("Please load a picture first!"));
    }
}
