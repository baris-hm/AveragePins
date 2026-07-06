#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem> // image items
#include <QList>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_addImageButton_clicked();

private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene;

    // Track image layers
    QList<QGraphicsPixmapItem*> layers;

    void updateLayerOrder(); // z-order helper
};
#endif // MAINWINDOW_H
