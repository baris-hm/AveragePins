#ifndef PIXELCANVASVIEW_H
#define PIXELCANVASVIEW_H

#include <QGraphicsView>
#include <QWheelEvent>
#include <QMouseEvent>

class PixelCanvasView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit PixelCanvasView(QWidget *parent = nullptr);

protected:
    // Override mouse wheel event for zooming
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
};

#endif // PIXELCANVASVIEW_H