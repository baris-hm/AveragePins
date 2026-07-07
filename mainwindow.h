#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem> // image items
#include <QList>
#include <QGraphicsEllipseItem> // for pins
#include <QColor>

// my own pin struct

struct Pin{
    QPointF position;
    QGraphicsEllipseItem* visualItem;
};

// a group of pins
struct PinSet{
    QString name;
    QColor color;
    QList<Pin> pins;
    bool isVisible = true;
};



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void addPinAtPosition(const QPointF& scenePos);

    enum ToolMode {
        HandTool,
        MoveTool
    };

    ToolMode currentToolMode() const {return m_currentToolMode;}

private slots:
    void on_addImageButton_clicked();
    void on_newPinSetButton_clicked();

    void on_handToolButton_clicked();
    void on_moveToolButton_clicked();

    void on_calculateButton_clicked();

    void updateLayerInteractivity();
    void on_layersReordered();

private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene;

    // Track image layers
    QList<QGraphicsPixmapItem*> layers;

    QList<PinSet> pinSets;
    int activePinSetIndex = -1; // could also call this selectedPinSetIndex

    ToolMode m_currentToolMode = HandTool; // this will be the default

    void updateLayerOrder(); // z-order helper
};
#endif // MAINWINDOW_H
