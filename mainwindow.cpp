#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QFileInfo>
#include <utility>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->layerListWidget->setDragEnabled(true);
    ui->layerListWidget->setAcceptDrops(true);
    ui->layerListWidget->setDropIndicatorShown(true);
    ui->layerListWidget->setDefaultDropAction(Qt::MoveAction);
    ui->layerListWidget->setDragDropMode(QAbstractItemView::InternalMove);
    connect(ui->layerListWidget->model(), &QAbstractItemModel::rowsMoved,
            this, &MainWindow::on_layersReordered);
    // Initalizing the scene
    scene = new QGraphicsScene(this);

    // Setting up a default canvas size. Here 1920x1080
    scene->setSceneRect(0,0,1920,1080);

    ui -> graphicsView -> setScene(scene);

    // bg color
    scene -> setBackgroundBrush(Qt::lightGray);


    activePinSetIndex = -1;

    ui->graphicsView->setAlignment(Qt::AlignCenter);
    // letting this go for now, it's already connected by slot - hence opens the thing twice
    //connect(ui->addImageButton, &QPushButton::clicked, this, &MainWindow::on_addImageButton_clicked);


    ui->handToolButton->setCheckable(true);
    ui->moveToolButton->setCheckable(true);
    ui->handToolButton->setChecked(true);
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
        QListWidgetItem *listItem = new QListWidgetItem(fileInfo.fileName());
        QVariant pointerAsVariant = QVariant::fromValue(static_cast<void*>(pixmapItem));
        listItem->setData(Qt::UserRole, pointerAsVariant);

        ui->layerListWidget->addItem(listItem);
    }

    updateLayerOrder();
    updateLayerInteractivity();
}

void MainWindow::updateLayerOrder()
{
    on_layersReordered();
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
        return;
    }

    // find out if there is an image layer under our click position
    QGraphicsPixmapItem* targetLayer = nullptr;

    // itemAt returns the topmost item at this coordinate
    QGraphicsItem* clickedItem = scene->itemAt(scenePos, QTransform());

    // check if what we clicked is actually one of our image layers
    if (clickedItem) {
        targetLayer = qgraphicsitem_cast<QGraphicsPixmapItem*>(clickedItem);
    }

    //  snapping based on whether we hit an image or empty canvas
    QPointF snappedPos;
    if (targetLayer) {
        // Map the global scene click down to the local pixel coordinate of the image
        QPointF localPos = targetLayer->mapFromScene(scenePos);
        snappedPos = QPointF(std::floor(localPos.x()) + 0.5, std::floor(localPos.y()) + 0.5);
    } else {
        snappedPos = QPointF(std::floor(scenePos.x()) + 0.5, std::floor(scenePos.y()) + 0.5);
    }

    // 3. create the avtual pin item
    double radius = 1.0;
    QGraphicsEllipseItem *visualPin = new QGraphicsEllipseItem(
        -radius, -radius, radius * 2, radius * 2
        );

    visualPin->setPen(QPen(Qt::black, 0.2));
    visualPin->setBrush(QBrush(pinSets[activePinSetIndex].color));
    visualPin->setZValue(10000); // Always stay visible above the image details

    // 4. ATTACH THE PIN
    if (targetLayer) {
        // Make the image layer the parent!
        visualPin->setParentItem(targetLayer);
        visualPin->setPos(snappedPos); // This sets position relative to the image's top-left corner
    } else {
        // If they clicked the empty canvas, attach it directly to the scene instead
        scene->addItem(visualPin);
        visualPin->setPos(snappedPos);
    }

    // 5. Store the pin data
    Pin pin;
    // absolute scene position matters for calculations later!!!!
    pin.visualItem = visualPin;

    pinSets[activePinSetIndex].pins.append(pin);
}

void MainWindow::on_handToolButton_clicked(){
    m_currentToolMode = HandTool;
    ui->handToolButton->setChecked(true);
    ui->moveToolButton->setChecked(false);
    updateLayerInteractivity();
}

void MainWindow::on_moveToolButton_clicked() {
    m_currentToolMode = MoveTool;
    ui->handToolButton->setChecked(false);
    ui->moveToolButton->setChecked(true);
    updateLayerInteractivity();
}


void MainWindow::on_calculateButton_clicked()
{
    if (pinSets.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Please create a pin set and place some pins first!"));
        return;
    }

    QString resultsSummary = "Average Points:\n";

    // loop through pin sets
    for (const PinSet& set : std::as_const(pinSets)) {
        if (set.pins.isEmpty()) {
            resultsSummary += QString("- %1: No pins placed.\n").arg(set.name);
            continue;
        }

        double totalX = 0.0;
        double totalY = 0.0;

        // sum up positions
        for (const Pin& pin : set.pins) {
            // mapToScene handles converting relative coordinates back to global canvas space
            QPointF absolutePos = pin.visualItem->mapToScene(0, 0);
            totalX += absolutePos.x();
            totalY += absolutePos.y();
        }

        // calculate final average position
        double avgX = totalX / set.pins.size();
        double avgY = totalY / set.pins.size();

        // round to the nearest whole pixel for your output coordinate
        int finalX = std::floor(avgX);
        int finalY = std::floor(avgY);

        resultsSummary += QString("- %1: Pixel (%2, %3)\n").arg(set.name, QString::number(finalX), QString::number(finalY));

        // Let's drop a big, special "Output Crosshair Pin" on the canvas so the user can visually see it!
        double crossRadius = 4.0;
        QGraphicsEllipseItem* resultMarker = scene->addEllipse(
            avgX - crossRadius, avgY - crossRadius, crossRadius * 2, crossRadius * 2,
            QPen(Qt::black, 1.0),
            QBrush(set.color) // Filled with matching pin set color
            );
        resultMarker->setZValue(20000); // Sit on top of everything else
        }

    // Pop up a neat dialog box showing the user the exact final output pixels
    QMessageBox::information(this, tr("Calculation Complete"), resultsSummary);
}

void MainWindow::updateLayerInteractivity(){
    bool canMove = (m_currentToolMode == MoveTool);
    for (QGraphicsPixmapItem* layer : std::as_const(layers)) {
        layer->setFlag(QGraphicsItem::ItemIsMovable, canMove);
        layer->setFlag(QGraphicsItem::ItemIsSelectable, canMove);
    }
}

void MainWindow::on_layersReordered()
{
    int count = ui->layerListWidget->count();

    // Loop through the list widget in its new physical order
    for (int i = 0; i < count; ++i) {
        QListWidgetItem *item = ui->layerListWidget->item(i);
        if (!item) continue;

        // Extract the hidden pointer back out
        void* rawPointer = item->data(Qt::UserRole).value<void*>();
        QGraphicsPixmapItem* layer = static_cast<QGraphicsPixmapItem*>(rawPointer);

        if (layer) {
            // If index 0 is the top layer, give it the highest Z-value (count - i)
            // So if there are 3 items: Index 0 gets Z=3, Index 1 gets Z=2, Index 2 gets Z=1
            layer->setZValue(count - i);
        }
    }
}