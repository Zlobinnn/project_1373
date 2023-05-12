#include "mygraphicsview.h"
#include <QMouseEvent>
#include <QPainter>
#include <QPoint>
#include <QPolygon>
#include <QGraphicsPolygonItem>
#include <QDebug>

MyGraphicsView::MyGraphicsView() : QGraphicsView()
{
    start = nullptr;
    finish = nullptr;
}

void MyGraphicsView::mousePressEvent(QMouseEvent *event)
{
//    if (event->button() == Qt::LeftButton) {
//            polygonPoints << event->pos();
//            update();
//        }
    emit mousePress(event);
}

void MyGraphicsView::paintEvent(QPaintEvent *event)
{
    QGraphicsView::paintEvent(event);

    /*QPainter painter(viewport());
    painter.setPen(QPen(Qt::black, 2));
    painter.setBrush(QBrush(Qt::gray));

    for(int i=0;i<polygonPoints.size();i++){
        if (polygonPoints[i].size() > 1) {
            painter.drawPolygon(polygonPoints[i]);
        }
    }

   qDebug() << "paintEvent";*/
}

void MyGraphicsView::clearPolygon()
{
    polygonPoints.clear();
    update();
}
