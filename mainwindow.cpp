#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QFileInfo>
#include <utility>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Initalizing the scene
    scene = new QGraphicsScene(this);

    // Setting up a default canvas size. Here 1920x1080
    scene->setSceneRect(0,0,1920,1080);

    ui -> graphicsView -> setScene(scene);

    // bg color
    scene -> setBackgroundBrush(Qt::lightGray);


    activePinSetIndex = -1;
    // letting this go for now, it's already connected by slot - hence opens the thing twice
    //connect(ui->addImageButton, &QPushButton::clicked, this, &MainWindow::on_addImageButton_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_addImageButton_clicked()
{
    // 1. Open file dialog to select images
    QStringList fileNames = QFileDialog::getOpenFileNames(
        this,
        tr("Open Images"), "",
        tr("Image Files (*.png *.jpg *.jpeg *.bmp)"));

    // 2. Process each selected file
    // std::as_const wrapper here. otherwise Qt detaches (copies the whole thing) -> memory/cpu wasted.
    for (const QString &fileName : std::as_const(fileNames)) {
        QPixmap pixmap(fileName);
        if (pixmap.isNull()) continue; // Skip if the image failed to load

        // Create a new graphics item for the image
        QGraphicsPixmapItem *pixmapItem = new QGraphicsPixmapItem(pixmap);

        // This flag allows the user to click and drag the image around the canvas later!
        pixmapItem->setFlag(QGraphicsItem::ItemIsMovable, true);
        pixmapItem->setFlag(QGraphicsItem::ItemIsSelectable, true);

        // Add to scene and our tracking list
        scene->addItem(pixmapItem);
        layers.append(pixmapItem);

        // Add the file name to the UI ListWidget sidebar
        QFileInfo fileInfo(fileName);
        ui->layerListWidget->addItem(fileInfo.fileName()); // Ensure your QListWidget is named layerListWidget
    }

    updateLayerOrder();
}

void MainWindow::updateLayerOrder()
{
    // Go backward or forward depending on your preference.
    // Let's say index 0 in the list widget is the topmost visual layer.
    int count = ui->layerListWidget->count();
    for (int i = 0; i < count; ++i) {
        // Find the matching graphics item.
        // Note: This simple approach assumes layers list matches layerListWidget 1:1
        if (i < layers.size()) {
            // High stack value means it sits on top.
            // If i = 0 (top of list), give it the highest Z-value.
            layers[i]->setZValue(count - i);
        }
    }
}

void MainWindow::on_newPinSetButton_clicked()
{
    PinSet newSet;
    int setNumber = pinSets.size() + 1;
    newSet.name = QString("PinSet %1").arg(setNumber);

    QColor colors[] = {Qt::red, Qt::green, Qt::blue, Qt::magenta, Qt::cyan, Qt::yellow};
    newSet.color = colors[pinSets.size() % 6];

    pinSets.append(newSet);

    //pinSetListWidget here refers to the actual QListWidget in mainwindow.ui
    ui->pinSetListWidget->addItem(newSet.name);

    ui->pinSetListWidget->setCurrentRow(pinSets.size() - 1);
    activePinSetIndex = pinSets.size() - 1;
}

void MainWindow::addPinAtPosition(const QPointF& scenePos)
{
    activePinSetIndex = ui->pinSetListWidget->currentRow();

    if (activePinSetIndex < 0 || activePinSetIndex >= pinSets.size()) {
        // if they haven't created a pin set yet, don't let them drop a pin
        return;
    }

    // snap to the exact absolute center of the pixel clicked
    QPointF snappedPos(std::floor(scenePos.x()) + 0.5, std::floor(scenePos.y()) + 0.5);

    //  an actual small circle to represent the pin
    double radius = 1.0;
    QGraphicsEllipseItem *visualPin = scene->addEllipse(
        snappedPos.x() - radius,
        snappedPos.y() - radius,
        radius * 2,
        radius * 2,
        QPen(Qt::black, 0.2), // Black outline
        QBrush(pinSets[activePinSetIndex].color) // Filled with the Pin Set's unique color
        );

    // Ensure pins sit on the very top layer above all photos
    visualPin->setZValue(10000);

    // Bundle it into our data structures
    Pin pin;
    pin.position = snappedPos;
    pin.visualItem = visualPin;

    pinSets[activePinSetIndex].pins.append(pin);
}