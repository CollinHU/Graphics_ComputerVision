#include <QtWidgets>
#if defined(QT_PRINTSUPPORT_LIB)
#include <QtPrintSupport/qtprintsupportglobal.h>
#if QT_CONFIG(printdialog)
#include <QPrintDialog>
#endif
#endif

#include <QtGlobal>
//#inlcude <QShortcut>

#include "imageviewer.h"
#include "node.h"
//#include "utility.h"
#include <vector>
#include <queue>
using namespace  std;

//#include <QCursor>
#include <QPainter>
#include <QPen>
//#define MAX 10000.0

Node graphNode[2048][2048];
double costMax = 0;

static void initialGraphNode(const QImage *loadImage){
    costMax = 0;
    for(int i = 0;i < loadImage->width();i++)
    {
        for(int j = 0; j < loadImage->height();j++)
        {
            graphNode[i][j].row = i;
            graphNode[i][j].column = j;
            graphNode[i][j].state = 0;
            graphNode[i][j].prevNode = NULL;
            graphNode[i][j].totalCost = MAX;
            graphNode[i][j].maxDeriv = 0;
        }
    }
}



ImageViewer::ImageViewer()
   :  scribbling(false)
   , moveEnable(false)
   , imageLabel(new QLabel)
   , scrollArea(new QScrollArea)
   , scaleFactor(1)
{
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);
    //imageLabel->setMargin(15);
    this->setMouseTracking(true);
    imageLabel->setMouseTracking(true);
    scrollArea->setMouseTracking(true);

    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    scrollArea->setVisible(false);
    setCentralWidget(scrollArea);

    createActions();

    resize(QGuiApplication::primaryScreen()->availableSize() * 3 / 5);
}

bool ImageViewer::atImage(QPoint point)
{
    int tmpX = point.x();
    int tmpY = point.y();

    if(tmpX >= 0
      && tmpX < imageLabel->pixmap()->width()
      && tmpY >= 0
      && tmpY < imageLabel->pixmap()->height())
        return true;
    else
        return false;
}

bool ImageViewer::loadFile(const QString &fileName)
{
    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    const QImage newImage = reader.read();
    if (newImage.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot load %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        return false;
    }
    initialGraphNode(&newImage);
    setImage(newImage);

    setWindowFilePath(fileName);

    const QString message = tr("Opened \"%1\", %2x%3, Depth: %4")
        .arg(QDir::toNativeSeparators(fileName)).arg(image.width()).arg(image.height()).arg(image.depth());
    statusBar()->showMessage(message);

    return true;
}

void ImageViewer::setImage(const QImage &newImage)
{
    image = newImage;

    qpixmap = QPixmap::fromImage(image);
    imageLabel->setPixmap(qpixmap);

    scaleFactor = 1.0;

    scrollArea->setVisible(true);
    printAct->setEnabled(true);
    fitToWindowAct->setEnabled(true);
    updateActions();

    if (!fitToWindowAct->isChecked())
        imageLabel->adjustSize();

    double factor = 400.0/image.width();
    scaleImage(factor);
    computeCost();
    endPoint = QPoint(0,0);
    seedPoints.clear();
    wirePoints.clear();
}


bool ImageViewer::saveFile(const QString &fileName, const QRect &Rect)
{
    QImageWriter writer(fileName);
    QImage imageToSave = imageLabel->pixmap()->copy(Rect).toImage();

    if (!writer.write(imageToSave)){
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot write %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName)), writer.errorString());
        return false;
    }
    const QString message = tr("Wrote \"%1\"").arg(QDir::toNativeSeparators(fileName));
    statusBar()->showMessage(message);
    return true;
}


static void initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode)
{
    static bool firstDialog = true;

    if (firstDialog) {
        firstDialog = false;
        const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    }

    QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen
        ? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
    foreach (const QByteArray &mimeTypeName, supportedMimeTypes)
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/jpeg");
    if (acceptMode == QFileDialog::AcceptSave)
        dialog.setDefaultSuffix("jpg");
}

void ImageViewer::open()
{
    scribbling = false;
    moveEnable = false;

    QFileDialog dialog(this, tr("Open File"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

    while (dialog.exec() == QDialog::Accepted && !loadFile(dialog.selectedFiles().first())) {}
}

/*
void ImageViewer::saveAs()
{
    QFileDialog dialog(this, tr("Save File As"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptSave);

    while (dialog.exec() == QDialog::Accepted && !saveFile(dialog.selectedFiles().first())) {}
}
*/

void ImageViewer::print()
{
    scribbling = false;
    moveEnable = false;

    Q_ASSERT(imageLabel->pixmap());
#if QT_CONFIG(printdialog)
    QPrintDialog dialog(&printer, this);
    if (dialog.exec()) {
        QPainter painter(&printer);
        QRect rect = painter.viewport();
        QSize size = imageLabel->pixmap()->size();
        size.scale(rect.size(), Qt::KeepAspectRatio);
        painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
        painter.setWindow(imageLabel->pixmap()->rect());
        painter.drawPixmap(0, 0, *imageLabel->pixmap());
    }
#endif
}

void ImageViewer::copy()
{
    scribbling = false;
    moveEnable = false;

#ifndef QT_NO_CLIPBOARD
    QGuiApplication::clipboard()->setImage(image);
#endif // !QT_NO_CLIPBOARD
}

#ifndef QT_NO_CLIPBOARD
static QImage clipboardImage()
{
    if (const QMimeData *mimeData = QGuiApplication::clipboard()->mimeData()) {
        if (mimeData->hasImage()) {
            const QImage image = qvariant_cast<QImage>(mimeData->imageData());
            if (!image.isNull())
                return image;
        }
    }
    return QImage();
}
#endif // !QT_NO_CLIPBOARD

void ImageViewer::paste()
{
    scribbling = false;
    moveEnable = false;

#ifndef QT_NO_CLIPBOARD
    const QImage newImage = clipboardImage();
    if (newImage.isNull()) {
        statusBar()->showMessage(tr("No image in clipboard"));
    } else {
        setImage(newImage);
        setWindowFilePath(QString());
        const QString message = tr("Obtained image from clipboard, %1x%2, Depth: %3")
            .arg(newImage.width()).arg(newImage.height()).arg(newImage.depth());
        statusBar()->showMessage(message);
    }
#endif // !QT_NO_CLIPBOARD
}

void ImageViewer::zoomIn()
{
    //scribbling = false;
    //moveEnable = false;
    scaleImage(1.25);
}

void ImageViewer::zoomOut()
{
    //scribbling = false;
    scaleImage(0.8);
}

void ImageViewer::normalSize()
{
    //scribbling = false;
    imageLabel->adjustSize();
    scaleFactor = 1.0;
}

void ImageViewer::fitToWindow()
{
    //scribbling = false;

    bool fitToWindow = fitToWindowAct->isChecked();
    scrollArea->setWidgetResizable(fitToWindow);
    if (!fitToWindow)
        normalSize();
    updateActions();
}

void ImageViewer::about()
{
    QMessageBox::about(this, tr("About Image Viewer"),
            tr("<p>The <b>Image Viewer</b> example shows how to combine QLabel "
               "and QScrollArea to display an image. QLabel is typically used "
               "for displaying a text, but it can also display an image. "
               "QScrollArea provides a scrolling view around another widget. "
               "If the child widget exceeds the size of the frame, QScrollArea "
               "automatically provides scroll bars. </p><p>The example "
               "demonstrates how QLabel's ability to scale its contents "
               "(QLabel::scaledContents), and QScrollArea's ability to "
               "automatically resize its contents "
               "(QScrollArea::widgetResizable), can be used to implement "
               "zooming and scaling features. </p><p>In addition the example "
               "shows how to use QPainter to print an image.</p>"));
}

void ImageViewer::imageOnly()
{
   scribbling = false;
   moveEnable = false;
   int width = imageLabel->pixmap()->width();
   imageLabel->setPixmap(QPixmap::fromImage(image));
   scaleImage(double(width/imageLabel->pixmap()->width()));
}

void ImageViewer::imageWithContour()
{
    imageLabel->setPixmap(qpixmap);
}

void ImageViewer::undo(){

    if(seedPoints.isEmpty())
        return;
    seedPoints.pop_back();
    if(!seedPoints.isEmpty())
    {
        while((wirePoints.last() != seedPoints.last()  && !(wirePoints.isEmpty())))
        {
            wirePoints.pop_back();
            //statusBar()->showMessage(QString("%1,%2").arg(wirePoints.size()).arg(seedPoints.size()));
            //statusBar()->showMessage(QString("%1,%2,%3,%4").arg(lastSeed.x()).arg(lastWirePoint.x()).arg(lastSeed.y()).arg(lastWirePoint.y()));
        }
    }
    else
        wirePoints.clear();

    if(seedPoints.size() == 1)
    {
              wirePoints.clear();
              seedPoints.clear();
    }

    if(!seedPoints.isEmpty())
        shortestPath(seedPoints.last());

    qpixmap = QPixmap::fromImage(image);
    drawLineWithNode();
    statusBar()->showMessage(QString("%1,%2").arg(wirePoints.size()).arg(seedPoints.size()));
}

void ImageViewer::costGraph(){
    scribbling = false;
    moveEnable = false;

    int org_width = image.width();
    int org_height = image.height();
    int width = org_width * 3;
    int height = org_height * 3;
    int value;


    //QSize newSize(width, height);
    QImage tmp(width, height, QImage::Format_RGB32);
    for(int x = 0; x < org_width; x++){
        for(int y = 0; y < org_height; y++){
            //QPoint b(x*3, y*3);
            //id = y * org_width + x;
            value = 255;//qGray(image.pixel(x, y));
            tmp.setPixel(x*3+1, y*3+1,QColor(value, value, value).rgb());

            value = std::min<unsigned long>(graphNode[x][y].linkCost[0], 255);
            tmp.setPixel(x*3+2, y*3+1, QColor(value, value, value).rgb());

            value = std::min<unsigned long>(graphNode[x][y].linkCost[1], 255);
            tmp.setPixel(x*3+2, y*3,   QColor(value, value, value).rgb());

            value = std::min<unsigned long>(graphNode[x][y].linkCost[2], 255);
            tmp.setPixel(x*3+1, y*3,   QColor(value, value, value).rgb());

            value = std::min<unsigned long>(graphNode[x][y].linkCost[3], 255);
            tmp.setPixel(x*3,   y*3, QColor(value, value, value).rgb());

            value = std::min<unsigned long>(graphNode[x][y].linkCost[4], 255);
            tmp.setPixel(x*3, y*3+1, QColor(value, value, value).rgb());

            value =  std::min<unsigned long>(graphNode[x][y].linkCost[5], 255);
            tmp.setPixel(x*3,   y*3+2, QColor(value, value, value).rgb());

            value = std::min<unsigned long>(graphNode[x][y].linkCost[6], 255);
            tmp.setPixel(x*3+1, y*3+2, QColor(value, value, value).rgb());

            value = std::min<unsigned long>(graphNode[x][y].linkCost[7], 255);
            tmp.setPixel(x*3+2, y*3+2, QColor(value, value, value).rgb());
        }
    }
    //imageLabel->resize(QPixmap::fromImage(tmp).size());
    //setImage(tmp);
    imageLabel->setPixmap(QPixmap::fromImage(tmp));
    statusBar()->showMessage(QString("%1, %2").arg(tmp.width()).arg(tmp.height()));
    //scaleImage(3.0);
}

void ImageViewer::pixelNode(){
    scribbling = false;
    moveEnable = false;

    int org_width = image.width();
    int org_height = image.height();
    int width = org_width * 3;
    int height = org_height * 3;
    //int value;


    //QSize newSize(width, height);
    QImage tmp(width, height, QImage::Format_RGB32);
    tmp.fill(Qt::black);

    for(int x = 0; x < org_width; x++){
        for(int y = 0; y < org_height; y++){
            //QPoint b(x*3, y*3);
            //id = y * org_width + x;
            tmp.setPixel(x*3+1, y*3+1,image.pixelColor(x, y).rgb());
        }
    }
    //imageLabel->resize(QPixmap::fromImage(tmp).size());
    //setImage(tmp);
    imageLabel->setPixmap(QPixmap::fromImage(tmp));
    statusBar()->showMessage(QString("%1, %2").arg(tmp.width()).arg(tmp.height()));
    //scaleImage(3.0);
}


void ImageViewer::minPath(){
    scribbling = true;
    moveEnable = true;

}

void ImageViewer::pathTree(){

    if(seedPoints.isEmpty())
        return;
    shortestPath(seedPoints.last());

    int tmpWidth = 3 * image.width();
    int tmpHeight = 3 * image.height();

    QImage tmp(tmpWidth, tmpHeight, QImage::Format_RGB32);
    tmp.fill(Qt::black);
    imageLabel->setPixmap(QPixmap::fromImage(tmp));

    int count = 0;

    priority_queue<Node*, vector<Node*>, greaterNode> treeq;
    //treeq.push(&graphNode[seedPoints.x()][seedPoints.y()]);
    for(int x = 0; x < image.width(); x++){
        for(int y = 0; y < image.height(); y++){
            treeq.push(&graphNode[x][y]);
        }
    }
    int a_x, a_y;
    while(!treeq.empty()){
        Node* a = treeq.top();
        a_x = a->row;
        a_y = a->column;
        treeq.pop();
        int value = int(255 * a->totalCost /(costMax * 2) + 125);
        tmp.setPixel(3 * a_x + 1, 3 * a_y + 1, QColor(value, value,0).rgb());
        if(graphNode[a_x][a_y].prevNode != NULL)
        {
            int i = graphNode[a_x][a_y].prevNode->row - graphNode[a_x][a_y].row;
            int j = graphNode[a_x][a_y].prevNode->column - graphNode[a_x][a_y].column;

           //image.pixelColor(x,y).rgb());
            tmp.setPixel(3 * a_x + 1 + i, 3 * a_y + 1 + j, QColor(value, value,0).rgb());
            tmp.setPixel(3 * a_x + 1 + 2 * i, 3 * a_y + 1 + 2 * j, QColor(value, value,0).rgb());
         }
        count++;
        if(count % 5000 == 0){
            imageLabel->setPixmap(QPixmap::fromImage(tmp));
            delay();
        }
    }

    imageLabel->setPixmap(QPixmap::fromImage(tmp));
}

void ImageViewer::delay(){
    QTime dieTime= QTime::currentTime().addMSecs(50);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}


void ImageViewer::drawMinPath(QPoint mousePoint){

    if(seedPoints.isEmpty() || !moveEnable)
        return;
    imageLabel->setPixmap(qpixmap);

    QPixmap tmpPixmap = qpixmap.copy();
    QPainter painter(&tmpPixmap);

    painter.setPen(QPen(Qt::green, 2, Qt::SolidLine, Qt::RoundCap,
                        Qt::RoundJoin));


    Node *node = &graphNode[mousePoint.x()][mousePoint.y()];

    while(node != NULL)
    {
        painter.drawPoint(QPoint(node->row,node->column));
        node = node->prevNode;
    }
    imageLabel->setPixmap(tmpPixmap);
    statusBar()->showMessage(QString("%1,%2").arg(wirePoints.size()).arg(seedPoints.size()));

}

void ImageViewer::saveContour()
{
    imageLabel->setPixmap(qpixmap);
    QImage tmp = qpixmap.toImage();
    QString fileName = QFileDialog::getSaveFileName(this,"Save",QDir::currentPath(),"JPG-Files (*.jpg)");
    if (!fileName.isEmpty()) {
         if (tmp.isNull()) {
             QMessageBox::information(this, tr("Error"),
                                      tr("Please Load an Image First!"));
             return;
         }
        tmp.save(fileName);
    }



}

void ImageViewer::saveMask()
{
    scribbling = false;
    moveEnable = false;

    if(wirePoints.isEmpty())
        return;

    int tmpWidth = image.width();
    int tmpHeight = image.height();

    QImage tmp(tmpWidth, tmpHeight, QImage::Format_ARGB32);
    tmp.fill(QColor(255,255,255,0).rgba());

    bool *maskMatrix = (bool*)malloc(tmpWidth * tmpHeight * sizeof(bool));

    for(int j = 0; j < tmpHeight;j++){
        for(int i = 0;i < tmpWidth;i++){
            maskMatrix[j * tmpWidth + i] = false;
        }
    }

    for(int i = 0;i < wirePoints.size();i++){
       maskMatrix[wirePoints.at(i).y() * tmpWidth + wirePoints.at(i).x()] = true;
    }

    QVector<QPoint> expandMaskVec;
    expandMaskVec.append(QPoint(0,0));
    //maskMatrix[0] = true;
    while(!expandMaskVec.isEmpty())
    {
        QPoint point = expandMaskVec.last();
        expandMaskVec.pop_back();
        int tmpX = point.x();
        int tmpY = point.y();

        if(tmpX >= 0 && tmpX <= tmpWidth && tmpY >= 0 && tmpY <= tmpHeight)
        {
            int index = tmpY * tmpWidth + tmpX;
            if(!maskMatrix[index])
            {
                maskMatrix[index] = true;
                expandMaskVec.append(QPoint(tmpX - 1,tmpY));
                expandMaskVec.append(QPoint(tmpX + 1,tmpY));
                expandMaskVec.append(QPoint(tmpX,tmpY - 1));
                expandMaskVec.append(QPoint(tmpX,tmpY + 1));
            }
        }
    }

    for(int i = 0;i < tmpWidth * tmpHeight;i++)
    {
        if(!maskMatrix[i])
        {
            int x = i % tmpWidth;
            int y = (int)(i / tmpWidth);
            QPoint tmpP(x,y);
            tmp.setPixel(tmpP,image.pixel(tmpP));
        }
    }

/*
    for(int i = 0;i < wirePoints.size();i++){
       QPoint tmpP(wirePoints.at(i).x(), wirePoints.at(i).y());
       tmp.setPixel(tmpP,image.pixel(tmpP));
       statusBar()->showMessage(QString("%1").arg(tmpWidth));
    }
*/
    delete maskMatrix;
    //statusBar()->showMessage(QString("%1, %2").arg(tmpWidth).arg(tmpHeight));
    imageLabel->setPixmap(QPixmap::fromImage(tmp));

    QString fileName = QFileDialog::getSaveFileName(this,"Save",QDir::currentPath(),"JPG-Files (*.jpg)");
    if (!fileName.isEmpty()) {
         if (tmp.isNull()) {
             QMessageBox::information(this, tr("Error"),
                                      tr("Please Load an Image First!"));
             return;
         }
        tmp.save(fileName);
    }

}

void ImageViewer::addFirstSeedPoint()
{
    if(scribbling || image.isNull())
        return;

    endPoint = cursorSnap(QWidget::mapFromGlobal(QCursor::pos()));
    scribbling = true;
    addFollowingSeedPoint();
}

void ImageViewer::addFollowingSeedPoint()
{
    //endPoint = cursorSnap(event->pos());
    if(image.isNull() || !scribbling)
        return;

    if(!atImage(endPoint))
        return;

    //statusBar()->showMessage(QString("%1, %2, %3").arg(endPoint.x()).arg(graphNode[endPoint.x()][endPoint.y()].linkCost[0]).arg(endPoint.y()));

    Node *node = &graphNode[endPoint.x()][endPoint.y()];
    if(seedPoints.isEmpty())
    {
        wirePoints.append(QPoint(endPoint.x(), endPoint.y()));
        //drawLineWithNode();
        //statusBar()->showMessage(QString("%1").arg(wirePoints.size()));
    }
    else
    {
        QPoint lastSeed = seedPoints.last();
        QVector<QPoint> tmpVec;
        while(node != NULL)
        {
            if(node->row == lastSeed.x() && node->column == lastSeed.y())
            {
                break;
                //statusBar()->showMessage(QString("%1, %2").arg(wirePoints.size()).arg(seedPoints.size()));
             }

            tmpVec.prepend(QPoint(node->row,node->column));
            node = node->prevNode;
        }
        wirePoints += tmpVec;
        //statusBar()->showMessage(QString("%1, %2").arg(wirePoints.size()).arg(seedPoints.size()));
        //statusBar()->showMessage(QString("%1, %2").arg(wirePoints.size()).arg(seedPoints.size()));
    }
    drawLineWithNode();
    seedPoints.append(QPoint(endPoint.x(),endPoint.y()));
    shortestPath(seedPoints.last());
    //statusBar()->showMessage(QString("%1, %2").arg(wirePoints.size()).arg(seedPoints.size()));
    statusBar()->showMessage(QString("%1, %2").arg(wirePoints.size()).arg(seedPoints.size()));

}

void ImageViewer::finishCurrentContourClose()
{
    //seedPoints.clear();
    scribbling = false;
    moveEnable = false;
    imageLabel->setPixmap(qpixmap);
}

void ImageViewer::finishCurrentContour()
{
    //seedPoints.clear();
    scribbling = false;
    moveEnable = false;
    imageLabel->setPixmap(qpixmap);
}



void ImageViewer::createActions()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    QAction *openAct = fileMenu->addAction(tr("&Open..."), this, &ImageViewer::open);
    openAct->setShortcut(QKeySequence::Open);

    //saveAsAct = fileMenu->addAction(tr("&Save As..."), this, &ImageViewer::saveAs);
    //saveAsAct->setEnabled(false);

    printAct = fileMenu->addAction(tr("&Print..."), this, &ImageViewer::print);
    printAct->setShortcut(QKeySequence::Print);
    printAct->setEnabled(false);

    saveContourAct = fileMenu->addAction(tr("&Save Contour"), this, &ImageViewer::saveContour);
    saveContourAct->setEnabled(false);

    saveMaskAct = fileMenu->addAction(tr("&Save Mask"), this, &ImageViewer::saveMask);
    saveMaskAct->setEnabled(false);

    fileMenu->addSeparator();

    QAction *exitAct = fileMenu->addAction(tr("&Exit"), this, &QWidget::close);
    exitAct->setShortcut(tr("Ctrl+Q"));

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));

    copyAct = editMenu->addAction(tr("&Copy"), this, &ImageViewer::copy);
    copyAct->setShortcut(QKeySequence::Copy);
    copyAct->setEnabled(false);

    QAction *pasteAct = editMenu->addAction(tr("&Paste"), this, &ImageViewer::paste);
    pasteAct->setShortcut(QKeySequence::Paste);

    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));

    zoomInAct = viewMenu->addAction(tr("Zoom &In (25%)"), this, &ImageViewer::zoomIn);
    zoomInAct->setShortcut(QKeySequence::ZoomIn);
    zoomInAct->setEnabled(false);

    zoomOutAct = viewMenu->addAction(tr("Zoom &Out (25%)"), this, &ImageViewer::zoomOut);
    zoomOutAct->setShortcut(QKeySequence::ZoomOut);
    zoomOutAct->setEnabled(false);

    normalSizeAct = viewMenu->addAction(tr("&Normal Size"), this, &ImageViewer::normalSize);
    normalSizeAct->setShortcut(tr("Ctrl+S"));
    normalSizeAct->setEnabled(false);

    viewMenu->addSeparator();

    fitToWindowAct = viewMenu->addAction(tr("&Fit to Window"), this, &ImageViewer::fitToWindow);
    fitToWindowAct->setEnabled(false);
    fitToWindowAct->setCheckable(true);
    fitToWindowAct->setShortcut(tr("Ctrl+F"));

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

    helpMenu->addAction(tr("&About"), this, &ImageViewer::about);
    helpMenu->addAction(tr("About &Qt"), &QApplication::aboutQt);

    QMenu *toolMenu = menuBar()->addMenu(tr("&Tool"));
    QMenu *workMode = toolMenu->addMenu(tr("&Work Mode"));

    imageOnlyAct = workMode->addAction(tr("&Image Only"), this, &ImageViewer::imageOnly);
    imageOnlyAct->setEnabled(false);

    imageWithContourAct = workMode->addAction(tr("&Image With Contour"), this, &ImageViewer::imageWithContour);
    imageWithContourAct->setEnabled(false);

    QMenu *debugMode = toolMenu->addMenu(tr("&Debug Mode"));

    //undoAct = debugMode->addAction(tr("&Undo"), this, &ImageViewer::undo);
   // undoAct->setEnabled(false);

    costGraphAct = debugMode->addAction(tr("&Cost Graph"), this, &ImageViewer::costGraph);
    costGraphAct->setEnabled(false);

    minPathAct = debugMode->addAction(tr("&Min Path"), this, &ImageViewer::minPath);
    minPathAct->setEnabled(false);

    pathTreeAct = debugMode->addAction(tr("&Path Tree"), this, &ImageViewer::pathTree);
    pathTreeAct->setEnabled(false);

    pixelNodeAct = debugMode->addAction(tr("&Pixel Node"), this, &ImageViewer::pixelNode);
    pixelNodeAct->setEnabled(false);

    addFirstSeedPointSC = new QShortcut(QKeySequence("Ctrl+Left"), this);
    QObject::connect(addFirstSeedPointSC, SIGNAL(activated()), this, SLOT(addFirstSeedPoint()));
    addFirstSeedPointSC->setEnabled(false);

    addFollowingSeedPointSC = new QShortcut(QKeySequence("Left"), this);
    QObject::connect(addFollowingSeedPointSC, SIGNAL(activated()), this, SLOT(addFollowingSeedPoint()));
    addFollowingSeedPointSC->setEnabled(false);

    finishCurrentContourSC = new QShortcut(QKeySequence("Return"), this);
    QObject::connect(finishCurrentContourSC, SIGNAL(activated()), this, SLOT(finishCurrentContour()));
    finishCurrentContourSC->setEnabled(false);

    finishCurrentContourCloseSC = new QShortcut(QKeySequence("Ctrl+Return"), this);
    QObject::connect(finishCurrentContourCloseSC, SIGNAL(activated()), this, SLOT(finishCurrentContourClose()));
    finishCurrentContourCloseSC->setEnabled(false);

    undoSC = new QShortcut(QKeySequence("Backspace"), this);
    QObject::connect(undoSC, SIGNAL(activated()), this, SLOT(undo()));
    undoSC->setEnabled(false);
}

void ImageViewer::updateActions()
{
    saveContourAct->setEnabled(!image.isNull());
    copyAct->setEnabled(!image.isNull());
    zoomInAct->setEnabled(!fitToWindowAct->isChecked());
    zoomOutAct->setEnabled(!fitToWindowAct->isChecked());
    normalSizeAct->setEnabled(!fitToWindowAct->isChecked());

    imageOnlyAct->setEnabled(!fitToWindowAct->isChecked());
    imageWithContourAct->setEnabled(!fitToWindowAct->isChecked());
    //undoAct->setEnabled(!fitToWindowAct->isChecked());
    costGraphAct->setEnabled(!fitToWindowAct->isChecked());
    minPathAct->setEnabled(!fitToWindowAct->isChecked());
    pathTreeAct->setEnabled(!fitToWindowAct->isChecked());
    pixelNodeAct->setEnabled(!fitToWindowAct->isChecked());
    saveMaskAct->setEnabled(!fitToWindowAct->isChecked());

    addFirstSeedPointSC->setEnabled(!fitToWindowAct->isChecked());
    addFollowingSeedPointSC->setEnabled(!fitToWindowAct->isChecked());
    finishCurrentContourSC->setEnabled(!fitToWindowAct->isChecked());
    finishCurrentContourCloseSC->setEnabled(!fitToWindowAct->isChecked());
    undoSC->setEnabled(!fitToWindowAct->isChecked());

}

void ImageViewer::scaleImage(double factor)
{
    Q_ASSERT(imageLabel->pixmap());
    scaleFactor *= factor;
    imageLabel->resize(scaleFactor * imageLabel->pixmap()->size());

    adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(scrollArea->verticalScrollBar(), factor);

    zoomInAct->setEnabled(scaleFactor < 3.0);
    zoomOutAct->setEnabled(scaleFactor > 0.333);
}

void ImageViewer::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}


void ImageViewer::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && scribbling) {
        endPoint = cursorSnap(event->pos());
        //scribbling = true;
    }
}

void ImageViewer::mouseMoveEvent(QMouseEvent *event)
{
    if(image.isNull())
    {
        endPoint = convert_position(event->pos());
    }
    else
        endPoint = cursorSnap(event->pos());

    drawMinPath(endPoint);
    statusBar()->showMessage(QString("%1, %2").arg(event->pos().x()).arg(event->pos().y()));

}


void ImageViewer::mouseReleaseEvent(QMouseEvent *event)
{
     statusBar()->showMessage(QString("%1, %2").arg(event->pos().x()).arg(event->pos().y()));
     //statusBar()->showMessage(QString("%1, %2").arg(wirePoints.size()).arg(seedPoints.size()));
}

void ImageViewer::drawLineWithNode()
{
    QPainter painter(&qpixmap);

    painter.setPen(QPen(Qt::red, 2, Qt::SolidLine, Qt::RoundCap,
                        Qt::RoundJoin));

    for(int i = wirePoints.size() - 1;i >= 0;i--)
    {
        painter.drawPoint(wirePoints.at(i));
        //if(seedPoints.size() > 1 && wirePoints.at(i) == seedPoints.at(seedPoints.size() - 2))
            //break;
    }

    imageLabel->setPixmap(qpixmap);
}

QPoint ImageViewer::convert_position(QPoint point)
{
    QPoint converted_p = point;// - QPoint(15,15);
    converted_p /= scaleFactor;//QPoint(imageLabel->width(), imageLabel->height());
    //point = imageLabel->mapFromGlobal(point) + QPoint(138,43);
    //converted_p -= QPoint(15,15);
    return converted_p;
}

QPoint ImageViewer::cursorSnap(QPoint point)
{
    int range = 4;
    QPoint converted_p = point;// - QPoint(15,15);
    converted_p /= scaleFactor;//QPoint(imageLabel->width(), imageLabel->height());
    //point = imageLabel->mapFromGlobal(point) + QPoint(138,43);
    //converted_p -= QPoint(15,15);
    int newX = converted_p.x() - range;
    int newY = converted_p.y() - range;
    int relocX = converted_p.x(), relocY = converted_p.y();
    double localMax = 0.0;
    for(int i = 0;i < range * 2 + 1;i++)
    {
        for(int j = 0;j < range * 2 + 1;j++)
        {
            int tmpX = newX + i;
            int tmpY = newY + j;

            //make sure mouse location is not at the edge of the picture;
            if(atImage(QPoint(tmpX,tmpY)))
            {
                if(localMax < graphNode[tmpX][tmpY].maxDeriv)
                {
                    localMax = graphNode[tmpX][tmpY].maxDeriv;
                    relocX = tmpX;
                    relocY = tmpY;
                }

            }
        }
    }

    if(relocX == 0)
        relocX += 1;
    else if(relocX == imageLabel->pixmap()->height() - 1)
        relocX -= 1;

    if(relocY == 0)
        relocY += 1;
    else if(relocY == imageLabel->pixmap()->height() - 1)
        relocY -= 1;

    converted_p = QPoint(relocX,relocY);
    return converted_p;
}

void ImageViewer::shortestPath(QPoint st)
{
    costMax = 0;

    for(int i = 0;i < image.width();i++)
    {
        for(int j = 0; j < image.height();j++)
        {
           graphNode[i][j].prevNode = NULL;
           graphNode[i][j].state  = 0;
           graphNode[i][j].totalCost = MAX;
        }
    }

    graphNode[st.x()][st.y()].totalCost = 0.0;
    graphNode[st.x()][st.y()].prevNode = NULL;
    priority_queue<Node*, vector<Node*>, greaterNode> pq;
    pq.push(&graphNode[st.x()][st.y()]);

    while(!pq.empty()){
        Node* a = pq.top();
        pq.pop();
        a->state = 2;
        //a->totalCost = 0;
        int a_x = a->row;
        int a_y = a->column;
        if(a->totalCost > costMax)
            costMax = a->totalCost;

        for(int i = 0; i < 8; i++){
            //for each direction, get the offset in x and y
            int off_x, off_y;
            a->graph(off_x, off_y, i);
            int new_x = a_x + off_x;
            int new_y = a_y + off_y;
            if (new_x < 0 || new_y  < 0){
                continue;
            }
            int new_state = graphNode[new_x][new_y].state;
            if (new_state == 0){
                graphNode[new_x][new_y].prevNode= &graphNode[a_x][a_y];
                graphNode[new_x][new_y].totalCost= graphNode[a_x][a_y].totalCost + graphNode[a_x][a_y].linkCost[i];
                graphNode[new_x][new_y].state=1;
                pq.push(&graphNode[new_x][new_y]);
            }
            else if(new_state == 1){
                int current_cost = graphNode[a_x][a_y].totalCost + graphNode[a_x][a_y].linkCost[i];
                if(current_cost < graphNode[new_x][new_y].totalCost){
                    graphNode[new_x][new_y].prevNode = &graphNode[a_x][a_y];
                    graphNode[a_x][a_y].totalCost = current_cost;
                }
            }
        }
    }
}

void ImageViewer::computeCost(){
    //declare var
    int w = image.width();
    int h = image.height();
    //int id;
    int rDR, gDR, bDR;
    //double Derivative[8];
    double max = 0;
    //get rgb data
    //cv::Mat rgb = qimage_to_mat_ref(image, QImage::Format_RGB32);
    for(int y = 0; y < h; y++){
        for(int x = 0; x < w; x++){
            //initialize
            //id = y * w + x;
            //graphNode[x][y].column = y;
            //graphNode[x][y].row = x;
            //graphNode[x][y].state = 0;
            if(y>=1 && x<(w-1) && y<(h-1)){
                //D(link0)
                rDR = abs((image.pixelColor(x,y-1).red()+image.pixelColor(x+1,y-1).red())-(image.pixelColor(x,y+1).red()+image.pixelColor(x+1,y+1).red()))/4;
                gDR = abs((image.pixelColor(x,y-1).green()+image.pixelColor(x+1,y-1).green())-(image.pixelColor(x,y+1).green()+image.pixelColor(x+1,y+1).green()))/4;
                bDR = abs((image.pixelColor(x,y-1).blue()+image.pixelColor(x+1,y-1).blue())-(image.pixelColor(x,y+1).blue()+image.pixelColor(x+1,y+1).blue()))/4;
                graphNode[x][y].linkCost[0] = sqrt((rDR*rDR+gDR*gDR+bDR*bDR)/3);

            }
            else{
                graphNode[x][y].linkCost[0] = 0;
            }
            if(y>=1 && x<(w-1)){
                //D(link1)
                rDR = abs(image.pixelColor(x+1,y).red()-image.pixelColor(x,y-1).red())/sqrt(2.0);
                gDR = abs(image.pixelColor(x+1,y).green()-image.pixelColor(x,y-1).green())/sqrt(2.0);
                bDR = abs(image.pixelColor(x+1,y).blue()-image.pixelColor(x,y-1).blue())/sqrt(2.0);
                graphNode[x][y].linkCost[1] = sqrt((rDR*rDR+gDR*gDR+bDR*bDR)/3);
            }
            else{
                graphNode[x][y].linkCost[1] = 0;
            }
            if(y>=1 && x>=1 && x<(w-1)){
                //D(link2)
                rDR = abs((image.pixelColor(x-1,y).red()+image.pixelColor(x-1,y-1).red())-(image.pixelColor(x+1,y).red()+image.pixelColor(x+1,y-1).red()))/4;
                gDR = abs((image.pixelColor(x-1,y).green()+image.pixelColor(x-1,y-1).green())-(image.pixelColor(x+1,y).green()+image.pixelColor(x+1,y-1).green()))/4;
                bDR = abs((image.pixelColor(x-1,y).blue()+image.pixelColor(x-1,y-1).blue())-(image.pixelColor(x+1,y).blue()+image.pixelColor(x+1,y-1).blue()))/4;
                graphNode[x][y].linkCost[2] = sqrt((rDR*rDR+gDR*gDR+bDR*bDR)/3);
            }
            else{
                graphNode[x][y].linkCost[2] = 0;
            }
            if(y>=1 && x>=1){
                //D(link3)
                rDR = abs(image.pixelColor(x-1,y).red()-image.pixelColor(x,y-1).red())/sqrt(2.0);
                gDR = abs(image.pixelColor(x-1,y).green()-image.pixelColor(x,y-1).green())/sqrt(2.0);
                bDR = abs(image.pixelColor(x-1,y).blue()-image.pixelColor(x,y-1).blue())/sqrt(2.0);
                graphNode[x][y].linkCost[3] = sqrt((rDR*rDR+gDR*gDR+bDR*bDR)/3);
            }
            else{
                graphNode[x][y].linkCost[3] = 0;
            }
            if(y>=1 && x>=1 && y<(h-1)){
                //D(link4)
                rDR = abs((image.pixelColor(x-1,y-1).red()+image.pixelColor(x,y-1).red())-(image.pixelColor(x,y+1).red()+image.pixelColor(x-1,y+1).red()))/4;
                gDR = abs((image.pixelColor(x-1,y-1).green()+image.pixelColor(x,y-1).green())-(image.pixelColor(x,y+1).green()+image.pixelColor(x-1,y+1).green()))/4;
                bDR = abs((image.pixelColor(x-1,y-1).blue()+image.pixelColor(x,y-1).blue())-(image.pixelColor(x,y+1).blue()+image.pixelColor(x-1,y+1).blue()))/4;
                graphNode[x][y].linkCost[4] = sqrt((rDR*rDR+gDR*gDR+bDR*bDR)/3);
            }
            else{
                graphNode[x][y].linkCost[4] = 0;
            }
            if(x>=1 && y<(h-1)){
                //D(link5)
                rDR = abs(image.pixelColor(x-1,y).red()-image.pixelColor(x,y+1).red())/sqrt(2.0);
                gDR = abs(image.pixelColor(x-1,y).green()-image.pixelColor(x,y+1).green())/sqrt(2.0);
                bDR = abs(image.pixelColor(x-1,y).blue()-image.pixelColor(x,y+1).blue())/sqrt(2.0);
                graphNode[x][y].linkCost[5] = sqrt((rDR*rDR+gDR*gDR+bDR*bDR)/3);
            }
            else{
                graphNode[x][y].linkCost[5] = 0;
            }
            if(x>=1 && y<(h-1) && x<(w-1)){
                //D(link6)
                rDR = abs((image.pixelColor(x-1,y+1).red()+image.pixelColor(x-1,y).red())-(image.pixelColor(x+1,y+1).red()+image.pixelColor(x+1,y).red()))/4;
                gDR = abs((image.pixelColor(x-1,y+1).green()+image.pixelColor(x-1,y).green())-(image.pixelColor(x+1,y+1).green()+image.pixelColor(x+1,y).green()))/4;
                bDR = abs((image.pixelColor(x-1,y+1).blue()+image.pixelColor(x-1,y).blue())-(image.pixelColor(x+1,y+1).blue()+image.pixelColor(x+1,y).blue()))/4;
                graphNode[x][y].linkCost[6] = sqrt((rDR*rDR+gDR*gDR+bDR*bDR)/3);
            }
            else{
                graphNode[x][y].linkCost[6] = 0;
            }
            if(y<(h-1) && x<(w-1)){
                //D(link7)
                rDR = abs(image.pixelColor(x+1,y).red()-image.pixelColor(x,y+1).red())/sqrt(2.0);
                gDR = abs(image.pixelColor(x+1,y).green()-image.pixelColor(x,y+1).green())/sqrt(2.0);
                bDR = abs(image.pixelColor(x+1,y).blue()-image.pixelColor(x,y+1).blue())/sqrt(2.0);
                graphNode[x][y].linkCost[7] = sqrt((rDR*rDR+gDR*gDR+bDR*bDR)/3);
            }
            else{
                graphNode[x][y].linkCost[7] = 0;
            }

            for(int i = 0;i < 8;i++)
            {
                if(graphNode[x][y].linkCost[i] > max)
                    max = graphNode[x][y].linkCost[i];
                if(graphNode[x][y].linkCost[i] > graphNode[x][y].maxDeriv)
                    graphNode[x][y].maxDeriv = graphNode[x][y].linkCost[i];
            }
        }

    }

    for(int y = 0; y < h; y++){
        for(int x = 0; x < w; x++){
            for(int i = 0;i < 8;i++)
            {
                graphNode[x][y].linkCost[i] = (max - graphNode[x][y].linkCost[i]) * 1;
                if(i%2 != 0)
                    graphNode[x][y].linkCost[i] *= sqrt(2);
                //graphNode[x][y].linkCost[i] /= 2;
            }
        }
    }

}

