#include "pixelcanvasview.h"

PixelCanvasView::PixelCanvasView(QWidget * parent)
    : QGraphicsView(parent)
{
    // smooth panning w/ click & drag
    setDragMode(QGraphicsView::ScrollHandDrag);

    // center mouse while zooming
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);

    // we don't want anti-aliasing
    setRenderHint(QPainter::Antialiasing, false);
    setRenderHint(QPainter::SmoothPixmapTransform, false);

    // hide scroll bars (looks unprofessional otherwise imo)
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}


void PixelCanvasView::wheelEvent(QWheelEvent *event)
{
    // Calculate zoom factor based on scroll direction
    double angleDelta = event->angleDelta().y();
    double factor = 1.15; // 15% zoom per step

    if (angleDelta < 0) {
        factor = 1.0 / factor; // Zoom out
    }

    // Get current scale to prevent infinite zooming out or crashingly close zooming in
    double currentScale = transform().m11(); // m11 is the horizontal scaling factor

    // Limits: Don't zoom out past 10% or in past 100x (10000%) original size
    if ((factor > 1.0 && currentScale < 100.0) || (factor < 1.0 && currentScale > 0.1)) {
        scale(factor, factor);
    }
}