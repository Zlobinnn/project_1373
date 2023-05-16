#ifndef MYGRAPHICSVIEW_H
#define MYGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QMap>

class MyGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    QVector<QVector<QPoint>> polygonPoints;
    QVector<QGraphicsPolygonItem*> polygons;
    QVector<QGraphicsTextItem*> polygonsTexts;
    QGraphicsPixmapItem *start;
    QGraphicsPixmapItem *finish;
    MyGraphicsView();
    QMap<QGraphicsPolygonItem*, float> polygonsPatency;
public slots:
    void clearPolygon();


signals:
    void mousePress(QMouseEvent *event);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override{}

private:
//    QVector<QPoint> polygonPoints;

};

#endif // MYGRAPHICSVIEW_H

