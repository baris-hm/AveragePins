#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QFileInfo>
#include <utility>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QToolButton>
#include <QLineEdit>

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

    // layer name edit connectino
    connect(ui->layerListWidget, &QListWidget::itemDoubleClicked,
            this, &MainWindow::on_layerItemDoubleClicked);
    // pin set name edit connection
    connect(ui->pinSetListWidget, &QListWidget::itemDoubleClicked,
            this, &MainWindow::on_pinSetItemDoubleClicked);

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


#include <QHBoxLayout>
#include <QToolButton>

void MainWindow::on_addImageButton_clicked()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(
        this, tr("Open Images"), "", tr("Image Files (*.png *.jpg *.jpeg *.bmp)"));

    for (const QString &fileName : std::as_const(fileNames)) {
        QPixmap pixmap(fileName);
        if (pixmap.isNull()) continue;

        QGraphicsPixmapItem *pixmapItem = new QGraphicsPixmapItem(pixmap);
        pixmapItem->setFlag(QGraphicsItem::ItemIsMovable, true);
        pixmapItem->setFlag(QGraphicsItem::ItemIsSelectable, true);

        scene->addItem(pixmapItem);
        layers.append(pixmapItem);

        QFileInfo fileInfo(fileName);

        // 1. Create a blank base list item
        QListWidgetItem *listItem = new QListWidgetItem();
        ui->layerListWidget->addItem(listItem);

        // Store our layer pointer in the item just like before
        listItem->setData(Qt::UserRole, QVariant::fromValue(static_cast<void*>(pixmapItem)));

        // 2. Create a custom container widget for this row
        QWidget *rowWidget = new QWidget();
        QHBoxLayout *rowLayout = new QHBoxLayout(rowWidget);
        rowLayout->setContentsMargins(5, 2, 5, 2); // Tight spacing

        // 3. Add Elements to the row layout
        QLabel *nameLabel = new QLabel(fileInfo.fileName());
        QToolButton *visButton = new QToolButton();
        QToolButton *delButton = new QToolButton();

        visButton->setText("👁");
        visButton->setCheckable(true);
        visButton->setChecked(true);
        delButton->setText("🗑");

        rowLayout->addWidget(nameLabel, 1); // Gives label maximum stretch space
        rowLayout->addWidget(visButton);
        rowLayout->addWidget(delButton);

        // 4. Inject the custom row widget into the list widget item slot
        ui->layerListWidget->setItemWidget(listItem, rowWidget);

        // Match list item visual height to the newly inserted custom widget height
        listItem->setSizeHint(rowWidget->sizeHint());

        // 5. Connect the button signals using C++ lambdas!
        connect(visButton, &QToolButton::toggled, this, [pixmapItem](bool checked) {
            pixmapItem->setVisible(checked); // Toggles image on canvas
        });

        connect(delButton, &QToolButton::clicked, this, [this, listItem, pixmapItem]() {
            // Remove from canvas scene
            this->scene->removeItem(pixmapItem);
            this->layers.removeOne(pixmapItem);
            delete pixmapItem; // Free memory safely

            // Remove from UI List
            int row = this->ui->layerListWidget->row(listItem);
            delete this->ui->layerListWidget->takeItem(row);

            this->updateLayerOrder();
        });
    }

    updateLayerOrder();
    updateLayerInteractivity();
}

void MainWindow::updateLayerOrder()
{
    on_layersReordered();
}

#include <QHBoxLayout>
#include <QToolButton>

void MainWindow::on_newPinSetButton_clicked()
{
    // 1. Initialize the internal data structure
    PinSet newSet;
    int setNumber = pinSets.size() + 1;
    newSet.name = QString("Pin Set %1").arg(setNumber);

    QColor colors[] = {Qt::red, Qt::green, Qt::blue, Qt::magenta, Qt::cyan, Qt::yellow};
    newSet.color = colors[pinSets.size() % 6];
    newSet.isVisible = true;

    pinSets.append(newSet);
    int currentSetIndex = pinSets.size() - 1;

    // 2. Create a blank list widget item
    QListWidgetItem *listItem = new QListWidgetItem();
    ui->pinSetListWidget->addItem(listItem);

    // Store the unique index number inside this list item so we know which PinSet it maps to
    listItem->setData(Qt::UserRole, currentSetIndex);

    // 3. Build the custom container widget row
    QWidget *rowWidget = new QWidget();
    QHBoxLayout *rowLayout = new QHBoxLayout(rowWidget);
    rowLayout->setContentsMargins(5, 2, 5, 2);

    QLabel *nameLabel = new QLabel(newSet.name);
    QToolButton *visButton = new QToolButton();
    QToolButton *delButton = new QToolButton();

    visButton->setText("👁"); // this sucks, I'll replace it with an icon later on
    visButton->setCheckable(true);
    visButton->setChecked(true);
    delButton->setText("🗑");

    rowLayout->addWidget(nameLabel, 1);
    rowLayout->addWidget(visButton);
    rowLayout->addWidget(delButton);

    ui->pinSetListWidget->setItemWidget(listItem, rowWidget);
    listItem->setSizeHint(rowWidget->sizeHint());

    // 4. Automatically highlight the newly created set
    ui->pinSetListWidget->setCurrentItem(listItem);
    activePinSetIndex = currentSetIndex;

    // 5. Connect Visibility Toggle
    // We pass currentSetIndex by value to know which set to modify
    connect(visButton, &QToolButton::toggled, this, [this, currentSetIndex](bool checked) {
        if (currentSetIndex >= this->pinSets.size()) return;

        this->pinSets[currentSetIndex].isVisible = checked;

        // Loop through all pins belonging to this set and hide/show them
        for (const Pin& pin : std::as_const(this->pinSets[currentSetIndex].pins)) {
            if (pin.visualItem) {
                pin.visualItem->setVisible(checked);
            }
        }
    });

    connect(delButton, &QToolButton::clicked, this, [this, listItem]() {
        // Find which index this item currently points to
        int indexToDelete = listItem->data(Qt::UserRole).toInt();
        if (indexToDelete >= this->pinSets.size()) return;

        // Clean up all physical visual pins from the canvas scene first
        for (const Pin& pin : std::as_const(this->pinSets[indexToDelete].pins)) {
            if (pin.visualItem) {
                this->scene->removeItem(pin.visualItem);
                delete pin.visualItem;
            }
        }

        // Remove the data structure from our vector
        this->pinSets.removeAt(indexToDelete);

        // Delete the UI list item row
        int row = this->ui->pinSetListWidget->row(listItem);
        delete this->ui->pinSetListWidget->takeItem(row);

        // CRITICAL FIX: Because we removed an item from the middle of the pinSets list,
        // all items after it shifted down by 1 index! We must re-index the remaining UI rows.
        for (int i = 0; i < this->ui->pinSetListWidget->count(); ++i) {
            QListWidgetItem *item = this->ui->pinSetListWidget->item(i);
            if (item && item->data(Qt::UserRole).toInt() > indexToDelete) {
                int oldIndex = item->data(Qt::UserRole).toInt();
                item->setData(Qt::UserRole, oldIndex - 1);
            }
        }

        // Reset active selection safely
        this->activePinSetIndex = this->ui->pinSetListWidget->currentRow();
    });
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

    // TODOne: COMMENT THIS MESSAGE OUT.
    // Pop up a neat dialog box showing the user the exact final output pixels
    //QMessageBox::information(this, tr("Calculation Complete"), resultsSummary);
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

    for (int i = 0; i < count; ++i) {
        QListWidgetItem *item = ui->layerListWidget->item(i);
        if (!item) continue;

        void* rawPointer = item->data(Qt::UserRole).value<void*>();
        QGraphicsPixmapItem* layer = static_cast<QGraphicsPixmapItem*>(rawPointer);

        if (layer) {
            layer->setZValue(count - i);
        }
    }
}

#include <QLineEdit>

void MainWindow::on_layerItemDoubleClicked(QListWidgetItem *item)
{
    if (!item) return;

    // Get the custom widget layout we injected into this row
    QWidget *rowWidget = ui->layerListWidget->itemWidget(item);
    if (!rowWidget) return;

    // Find the QLabel inside that row widget
    QLabel *nameLabel = rowWidget->findChild<QLabel*>();
    if (!nameLabel || !nameLabel->isVisible()) return; // Don't do anything if it's already editing

    // Create an editable line input field
    QLineEdit *lineEdit = new QLineEdit(nameLabel->text(), rowWidget);

    // Swap them in the layout
    rowWidget->layout()->replaceWidget(nameLabel, lineEdit);
    nameLabel->hide();
    lineEdit->setFocus();
    lineEdit->selectAll();

    // Helper lambda function to finalize the text change
    auto finishEditing = [nameLabel, lineEdit, rowWidget]() {
        if (!lineEdit->text().trimmed().isEmpty()) {
            nameLabel->setText(lineEdit->text().trimmed());
        }
        rowWidget->layout()->replaceWidget(lineEdit, nameLabel);
        nameLabel->show();
        lineEdit->deleteLater(); // Safely destroy the input box
    };

    // Trigger finish if they press Enter or click away
    connect(lineEdit, &QLineEdit::returnPressed, this, finishEditing);
    connect(lineEdit, &QLineEdit::editingFinished, this, finishEditing);
}


void MainWindow::on_pinSetItemDoubleClicked(QListWidgetItem *item)
{
    if (!item) return;

    QWidget *rowWidget = ui->pinSetListWidget->itemWidget(item);
    if (!rowWidget) return;

    QLabel *nameLabel = rowWidget->findChild<QLabel*>();
    if (!nameLabel || !nameLabel->isVisible()) return;

    QLineEdit *lineEdit = new QLineEdit(nameLabel->text(), rowWidget);
    rowWidget->layout()->replaceWidget(nameLabel, lineEdit);
    nameLabel->hide();
    lineEdit->setFocus();
    lineEdit->selectAll();

    int setIndex = item->data(Qt::UserRole).toInt();

    auto finishEditing = [this, nameLabel, lineEdit, rowWidget, setIndex]() {
        QString newName = lineEdit->text().trimmed();
        if (!newName.isEmpty()) {
            nameLabel->setText(newName);

            if (setIndex >= 0 && setIndex < this->pinSets.size()) {
                this->pinSets[setIndex].name = newName;
            }
        }
        rowWidget->layout()->replaceWidget(lineEdit, nameLabel);
        nameLabel->show();
        lineEdit->deleteLater();
    };

    connect(lineEdit, &QLineEdit::returnPressed, this, finishEditing);
    connect(lineEdit, &QLineEdit::editingFinished, this, finishEditing);
}