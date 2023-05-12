#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mygraphicsview.h"
#include "astar.h"
#include <QMouseEvent>
#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsEllipseItem>
#include <QDebug>
#include <QXmlStreamWriter>
#include <QtCore/QFile>
#include <QtXml>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QXmlStreamAttribute>
#include <QFile>
#include <QList>
#include <QGraphicsItem>
#include <QPainter>
#include <QPoint>
#include <QPolygon>
#include <QGraphicsPolygonItem>
#include <QMap>
#include <QSlider>
#include <QColor>
#include <QMessageBox>
#include <QString>
#include <QGraphicsTextItem>
#include <QPen>
#include <QGraphicsDropShadowEffect>

MyGraphicsView *view;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //Создаём новый объект типа MyGraphicsView и задаём его параметры
    view = new MyGraphicsView;
    view->setMouseTracking(true);
    view->setScene(new QGraphicsScene);
    view->setRenderHint(QPainter::Antialiasing);
    view->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    view->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    view->setMinimumSize(800, 600);

    ui->verticalLayout->addWidget(view);

    //Присоединяем метод отслеживания нажатия кнопки мыши по объекту типа MyGraphicsView и метод обработки нажатия
    connect(view, &MyGraphicsView::mousePress, this, &MainWindow::on_mousePress);

    // Запретить автоматическое изменение масштаба
    view->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    view->setSceneRect(0, 0, 800, 600);
    view->setMinimumSize(800, 600);
    view->setMaximumSize(800, 600);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        //connect(view, &MyGraphicsView::mousePress, this, &MainWindow::on_mousePress);
//            connect(ui->createPolygonButton, &QPushButton::clicked, graphicsView, &MyGraphicsView::createPolygon);
//            connect(ui->clearPolygonButton, &QPushButton::clicked, graphicsView, &MyGraphicsView::clearPolygon);


    turn_off_flags();

    //Ищем элементы типа QAction в нашем приложении и соединяем методы отслеживания и обработки нажатия кнопки мыши по ним
    QAction *myAction = findChild<QAction*>("action");
    connect(myAction, &QAction::triggered, this, &MainWindow::onActionTriggered);

    QAction *myAction_2 = findChild<QAction*>("action_2");
    connect(myAction_2, &QAction::triggered, this, &MainWindow::onAction_2Triggered);

    QAction *myAction_3 = findChild<QAction*>("action_3");
    connect(myAction_3, &QAction::triggered, this, &MainWindow::onAction_3Triggered);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void saveFiguresToXml(const QString& filePath, QGraphicsScene* scene)
{
    QFile file(filePath);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
                return;

            QXmlStreamWriter writer(&file);
            writer.setAutoFormatting(true);
            writer.writeStartDocument();

            writer.writeStartElement("scene");
            writer.writeStartElement("figures");
            for (int i = 0; i < view->polygonPoints.size(); ++i) {
                writer.writeStartElement("figure");
                writer.writeStartElement("vertexes");
                for (int j = 0; j < view->polygonPoints[i].size(); ++j) {
                    writer.writeStartElement("vertex");
                    writer.writeAttribute("x",QString::number(view->polygonPoints[i][j].x()));
                    writer.writeAttribute("y",QString::number(view->polygonPoints[i][j].y()));
                    writer.writeEndElement();
                }

                QGraphicsPolygonItem*tmp = view->polygons[i];
                writer.writeTextElement("penetration",QString::number((view->polygonsPatency.value(tmp))));
                qDebug() << view->polygonsPatency;
                writer.writeEndElement(); // end of vertexes
                writer.writeEndElement(); // end of figure
            }
            writer.writeEndElement(); // end of item
            writer.writeEndElement(); // end of scene
            writer.writeEndDocument();
            file.close();
}

void saveSceneToXml(const QString& filePath, QGraphicsScene* scene)
{
    QFile file(filePath);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
                return;

            QXmlStreamWriter writer(&file);
            writer.setAutoFormatting(true);
            writer.writeStartDocument();
            writer.writeStartElement("scene");

            for (QGraphicsItem *item : view->scene()->items()) {
                    if (item==view->start) {
                        writer.writeStartElement("start");
                        writer.writeAttribute("x",QString::number(item->scenePos().x()));
                        writer.writeAttribute("y",QString::number(item->scenePos().y()));
                        writer.writeEndElement();
                    }
                    if (item==view->finish) {
                        writer.writeStartElement("finish");
                        writer.writeAttribute("x",QString::number(item->scenePos().x()));
                        writer.writeAttribute("y",QString::number(item->scenePos().y()));
                        writer.writeEndElement();
                    }
            }
            writer.writeStartElement("figures");
            for (int i = 0; i < view->polygonPoints.size(); ++i) {
                writer.writeStartElement("figure");
                writer.writeStartElement("vertexes");
                for (int j = 0; j < view->polygonPoints[i].size(); ++j) {
                    writer.writeStartElement("vertex");
                    writer.writeAttribute("x",QString::number(view->polygonPoints[i][j].x()));
                    writer.writeAttribute("y",QString::number(view->polygonPoints[i][j].y()));
                    writer.writeEndElement();
                }

                QGraphicsPolygonItem*tmp = view->polygons[i];
                writer.writeTextElement("penetration",QString::number((view->polygonsPatency[tmp])));

                writer.writeEndElement(); // end of vertexes
                writer.writeEndElement(); // end of figure
            }
            writer.writeEndElement(); // end of figures
            writer.writeEndDocument();// end of scene
            file.close();
}

void readScene(const QString& filePath, QGraphicsScene* scene)
{
    view->scene()->clear();
    view->polygonPoints.clear();
    view->polygonsPatency.clear();
    view->polygons.clear();
    view->polygonsTexts.clear();
    QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return;
        QXmlStreamReader reader(&file);

        while (!reader.atEnd()&&!reader.hasError())
        {
            if (reader.isStartElement() && reader.qualifiedName() == QLatin1String("figure"))
            {
                QVector<QPoint>figure;
                QPoint vertex;
                reader.readNext();
                while (!(reader.isEndElement() && reader.qualifiedName() == QLatin1String("figure")))
                {
                    if (reader.isStartElement() && reader.qualifiedName() == QLatin1String("vertex"))
                    {
                        int x = reader.attributes().value("x").toInt();
                        int y = reader.attributes().value("y").toInt();
                        vertex.setX(x);
                        vertex.setY(y);
                        figure<<vertex;
                    }
                    if (reader.qualifiedName() == QLatin1String("penetration"))
                    {
                        QPolygon polygon(figure);
                        QGraphicsPolygonItem* polygonItem = new QGraphicsPolygonItem(polygon);
                        float penetration = reader.readElementText().toFloat();
                        view->polygonsPatency.insert(polygonItem,penetration);
                        view->polygons<<polygonItem;
                    }
                    reader.readNext();
                }
                view->polygonPoints<<figure; // все vertex для figure обошли(встретили </figure>, кидаем в массив фигур.
            }
            reader.readNext();
        }
        file.close();

    for(int i = 0; i < view->polygonPoints.size(); ++i) {
        //QPolygon polygon(view->polygonPoints[i]);
        QGraphicsPolygonItem* polygonItem = view->polygons[i];
        float color = view->polygonsPatency.value(view->polygons[i]);
        polygonItem->setPen(QPen(Qt::black, 1));
        polygonItem->setBrush(QBrush(QColor::fromRgbF(color,color,color)));
        //view->polygons[i] = polygonItem;
        QGraphicsTextItem* textItem = new QGraphicsTextItem(QString::number(color * 100) + "%");
        view->polygonsTexts<<textItem;
        textItem->setParentItem(polygonItem);
        textItem->setDefaultTextColor(Qt::white);
        textItem->setFont(QFont("Arial", 14, QFont::Bold));
        textItem->setOpacity(1);
        QGraphicsDropShadowEffect* textShadow = new QGraphicsDropShadowEffect();
        textShadow->setBlurRadius(7);
        textShadow->setColor(QColor("black"));
        textShadow->setOffset(0,0);
        textItem->setGraphicsEffect(textShadow);
        QRectF textRect = textItem->boundingRect();
        textItem->setPos(polygonItem->boundingRect().center().x() - textRect.width() / 2.0, polygonItem->boundingRect().center().y() - textRect.height() / 2.0);
        view->scene()->addItem(polygonItem);
        view->scene()->update();
        qDebug()<<view->scene()->items().count();
    }
}

void MainWindow::turn_off_flags(){
    //Задаём все флаги в значение false
    add_but_is_clicked = false;
    delete_but_is_clicked = false;
    way_but_is_clicked = false;
    start_but_is_clicked = false;
    finish_but_is_clicked = false;

    //Привязываем указатели к кнопкам типа QPushButton в нашем приложении
    QPushButton* add_but = findChild<QPushButton*>("add_but");
    QPushButton* delete_but = findChild<QPushButton*>("delete_but");
    QPushButton* way_but = findChild<QPushButton*>("way_but");
    QPushButton* start_but = findChild<QPushButton*>("start_but");
    QPushButton* finish_but = findChild<QPushButton*>("finish_but");

    //Задаём стандартный вид вшнених кнопок
    add_but->setStyleSheet("");
    delete_but->setStyleSheet("");
    way_but->setStyleSheet("");
    start_but->setStyleSheet("");
    finish_but->setStyleSheet("");
}



void MainWindow::on_add_but_clicked()
{   

    if (!add_but_is_clicked){
        turn_off_flags();
        add_but_is_clicked = true;

        //Проверяем, лежит ли в массиве полигонов пустой полигон
        if(view->polygons.size()==0 || ((view->polygons.size()>0)&&(view->polygons[view->polygons.size()-1]!=nullptr))){
            //Создаём пустой массив точек и пустой полигон и кидаем их в массивы polygonPoints и polygons
            QVector<QPoint> a;
            view->polygonPoints << a;

            QGraphicsPolygonItem* polygon = nullptr;
            view->polygons << polygon;
        }

        //Меняет текст кнопки "Добавить" на "Готово"
        QPushButton* myButton = findChild<QPushButton*>("add_but");
        myButton->setText("Готово");

        //Задаём выделение кнопки
        QPushButton* add_but = findChild<QPushButton*>("add_but");
        add_but->setStyleSheet("border: 1px solid blue;");

        //Отключаем работоспособность всех других кнопок
        QPushButton* delete_but = findChild<QPushButton*>("delete_but");
        QPushButton* way_but = findChild<QPushButton*>("way_but");
        QPushButton* start_but = findChild<QPushButton*>("start_but");
        QPushButton* finish_but = findChild<QPushButton*>("finish_but");
        delete_but->setEnabled(false);
        way_but->setEnabled(false);
        start_but->setEnabled(false);
        finish_but->setEnabled(false);

        //Создаём объект типа slider (ползунок) в нашем приложении
        QSlider *slider = new QSlider(Qt::Horizontal, this);
        slider->setToolTip("Описание для ползунка");
        slider->setRange(0, 100);
        slider->setSingleStep(1);
        slider->setPageStep(10);
        slider->setTickInterval(10);
        slider->setTickPosition(QSlider::TicksBelow);
        ui->verticalLayout_2->addWidget(slider);
    }
    else{
        add_but_is_clicked = false;

        QSlider* slider = findChild<QSlider*>();

        //if(view->polygons.size()==0 || ((view->polygons.size()>0)&&(view->polygons[view->polygons.size()-1]!=nullptr))){
        //Проверяем, является ли последний полигон в массиве пустым
        if(((view->polygons.size()>0)&&(view->polygons[view->polygons.size()-1]!=nullptr))){
            //Получаем значение из ползунка
            float sliderValue = 0.0;
            sliderValue = static_cast<float>(slider->value()) / 100.0;
            qDebug() << sliderValue;

            //Записываем значение ползунка в массив коэф. проходимости
            view->polygonsPatency.insert(view->polygons[view->polygons.size()-1],sliderValue);
            qDebug() << "patency" << view->polygonsPatency;

            //Задаём цвет полигона в зависимости от коэф. проходимости
            QColor color = QColor::fromRgbF(sliderValue, sliderValue, sliderValue);
            QBrush brush(color);
            QList<QGraphicsItem*> items = view->scene()->items();

            //Ищем на сцене нужный полигон, чтобы на него накинуть текст
            foreach (QGraphicsItem* item, items) {
                if (item == view->polygons[view->polygons.size()-1]) {
                    QGraphicsPolygonItem* polygonItem = qgraphicsitem_cast<QGraphicsPolygonItem*>(item);
                    polygonItem->setBrush(brush);
                    QGraphicsTextItem* textItem = new QGraphicsTextItem(QString::number(sliderValue*100) + "%");
                    textItem->setDefaultTextColor(Qt::white);
                    textItem->setFont(QFont("Arial", 14));
                    textItem->setOpacity(1);
                    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect();
                    effect->setColor(Qt::black);
                    effect->setBlurRadius(3);
                    effect->setOffset(1,1);
                    textItem->setGraphicsEffect(effect);
                    textItem->setPos(polygonItem->boundingRect().center() - textItem->boundingRect().center());

                    //Добавляем текстовый элемент на сцену
                    polygonItem->scene()->addItem(textItem);
                    view->polygonsTexts << textItem;
                }
            }
        }
        else{
            //Удаляем пустой полигон и пустой массив точек
            view->polygons.removeAt(view->polygons.size()-1);
            view->polygonPoints.removeAt(view->polygonPoints.size()-1);
        }

        //Удаляем ползунок с приложения
        ui->verticalLayout_2->removeWidget(slider);
        delete slider;

        //Меняем текст кнопки на "Добавить"
        QPushButton* myButton = findChild<QPushButton*>("add_but");
        myButton->setText("Добавить");
        myButton->setStyleSheet("");

        //Включаем работоспособность всех других кнопок
        QPushButton* delete_but = findChild<QPushButton*>("delete_but");
        QPushButton* way_but = findChild<QPushButton*>("way_but");
        QPushButton* start_but = findChild<QPushButton*>("start_but");
        QPushButton* finish_but = findChild<QPushButton*>("finish_but");
        delete_but->setEnabled(true);
        way_but->setEnabled(true);
        start_but->setEnabled(true);
        finish_but->setEnabled(true);
    }

}


void MainWindow::on_delete_but_clicked()
{
    //Задаём флаг для кнопки удаления
    if (!add_but_is_clicked){
        turn_off_flags();
        delete_but_is_clicked = true;

        //saveSceneToXml("my_graphics_view.xml",view->scene());

        QPushButton* delete_but = findChild<QPushButton*>("delete_but");
        delete_but->setStyleSheet("border: 1px solid blue;");
    }
}

void MainWindow::on_way_but_clicked()
{

    if (!add_but_is_clicked){
        turn_off_flags();
        way_but_is_clicked = true;

        /*float A[800][600];
        int count = 0;
        for (int i=0;i<800;i++){
            for (int k=0;k<600;k++){
                A[i][k] = 1;
                for (QGraphicsItem *item : view->scene()->items()) {
                    if (item->type() == QGraphicsPolygonItem::Type && item->contains(QPoint(i, k))) {
                        count++;
                        QGraphicsPolygonItem *polygonItem = qgraphicsitem_cast<QGraphicsPolygonItem *>(item);
                        if (view->polygonsPatency.value(polygonItem) != 0){
                            //qDebug() << 1 / view->polygonsPatency.value(polygonItem);
                        }
                        else{
                            //qDebug() << 10000000;
                        }
                    }
                }
            }
        }
        //qDebug << A;
        QRectF sceneRect = view->sceneRect();

        // Переводим координаты (0,0) в QGraphicsView в координаты на QGraphicsScene
        QPointF scenePos = view->mapToScene(0, 0);
        qDebug() << "Scene rect:" << sceneRect << " Scene position:" << scenePos;

        for (int i=0;i < qRound(view->scene()->width());i++){
            for (int k=0;k < qRound(view->scene()->height());k++){
                for (const auto& item : view->scene()->items()) {
                    if (item->type() == QGraphicsPolygonItem::Type && item->contains(QPoint(i, k))) {
                        count++;
                    }
                }
            }
        }
        qDebug() << count;
        qDebug() << qRound(view->scene()->width());
        qDebug() << qRound(view->scene()->height());*/
        Astar astar;
        Node *startPos = new Node(view->start->scenePos().x(),view->start->scenePos().y());
        Node *endPos = new Node(view->finish->scenePos().x(),view->finish->scenePos().y());
        astar.search(startPos, endPos);
    }
}


void MainWindow::on_start_but_clicked()
{
    //Задаём флаг для кнопки старта
    if (!add_but_is_clicked){
        turn_off_flags();
        start_but_is_clicked = true;

        QPushButton* start_but = findChild<QPushButton*>("start_but");
        start_but->setStyleSheet("border: 1px solid blue;");
    }
}


void MainWindow::on_finish_but_clicked()
{
    //Задаём флаг для кнопки финиша
    if (!add_but_is_clicked){
        turn_off_flags();
        finish_but_is_clicked = true;

        QPushButton* finish_but = findChild<QPushButton*>("finish_but");
        finish_but->setStyleSheet("border: 1px solid blue;");
    }
}

void MainWindow::on_mousePress(QMouseEvent *event)
{
    qDebug() << view->polygons.size() << view->polygons;
    qDebug() << view->polygonPoints.size() << view->polygonPoints;
    qDebug() << view->polygonsPatency.size() << view->polygonsPatency;

    //Обрабатываем действия при нажатой кнопке старта
    if(start_but_is_clicked){
        bool isPolygonClicked = false;

        //Проверяем попал ли наш клик на какой-нибуль полигон
        for (QGraphicsItem *item : view->scene()->items()) {
                if (item->type() == QGraphicsPolygonItem::Type && item->contains(event->pos())) {
                    isPolygonClicked = true;
                    break;
                }
        }
        if (!isPolygonClicked){
            //Добавляем точку старта на поле
            QPointF point = view->mapToScene(event->pos());

            if(view->start == nullptr){
                QPixmap pixmap("start.png");
                QGraphicsPixmapItem *item = new QGraphicsPixmapItem(pixmap);
                view->start = item;
                item->setScale(0.05);
                //item->setPos(point.x()-12, point.y()-22);
                item->setPos(point.x(), point.y());
                item->setZValue(1);
                view->scene()->addItem(item);
                qDebug() << item->scenePos();
            }
            else{
                view->start->setPos(point.x(), point.y());
            }
            qDebug() << view->scene()->items();
            qDebug() << view->start->scenePos().x();
        }
        else{
            QMessageBox::critical(this, tr("Ошибка"), tr("Невозможно поставить точку старта на препятствие!"));
        }
    }

    //Обрабатываем действия при нажатой кнопке финиша
    if(finish_but_is_clicked){
        bool isPolygonClicked = false;

        //Проверяем попал ли наш клик на какой-нибуль полигон
        for (QGraphicsItem *item : view->scene()->items()) {
                if (item->type() == QGraphicsPolygonItem::Type && item->contains(event->pos())) {
                    isPolygonClicked = true;
                    break;
                }
        }
        if (!isPolygonClicked){
            //Добавляем точку финиша на поле
            QPointF point = view->mapToScene(event->pos());

            if(view->finish == nullptr){
                QPixmap pixmap("finish.png");
                QGraphicsPixmapItem *item = new QGraphicsPixmapItem(pixmap);
                view->finish = item;
                item->setScale(0.05);
                item->setPos(point.x(), point.y());
                item->setZValue(1);
                view->scene()->addItem(item);
            }
            else{
                view->finish->setPos(point.x(), point.y());
            }
            qDebug() << view->scene()->items();
        }
        else{
            QMessageBox::critical(this, tr("Ошибка"), tr("Невозможно поставить точку финиша на препятствие!"));
        }
    }

    //Обрабатываем действия при нажатой кнопке "Добавить"
    if ((event->button() == Qt::LeftButton)&&(add_but_is_clicked)) {
            /*if (view) {
                view->polygonPoints[view->polygonPoints.size()-1] << event->pos();
                view->scene()->update();
                qDebug() << view->scene()->items();
            }

            QPolygonF polygon;
            polygon << QPointF(0, 0) << QPointF(100, 0) << QPointF(100, 100) << QPointF(0, 100);  // координаты вершин полигона

            QGraphicsPolygonItem* polygonItem = new QGraphicsPolygonItem(polygon);
            polygonItem->setPen(QPen(Qt::black, 2));  // устанавливаем цвет ручки
            polygonItem->setBrush(QBrush(Qt::gray));  // устанавливаем цвет заливки

            view->scene()->addItem(polygonItem);*/
        /*QVector<QPoint> newPoints = view->polygonPoints[view->polygonPoints.size()-1];
        newPoints << event->pos();
        QPolygon polygon(newPoints);
        QGraphicsPolygonItem* polygonItem = new QGraphicsPolygonItem(polygon);
        QPainterPath path1 = polygonItem->shape();
        QPainterPath path2;*/

//        bool isPolygonClicked = false;
//        for (QGraphicsItem *item : view->scene()->items()) {
//                if (item->type() == QGraphicsPolygonItem::Type && item->contains(event->pos())) {
//                    isPolygonClicked = true;
//                    break;
//                }
//        }

        view->polygonPoints[view->polygonPoints.size()-1] << event->pos();

        QList<QGraphicsItem*> items = view->scene()->items();

        foreach (QGraphicsItem* item, items) {
            if (item == view->polygons[view->polygons.size()-1]) {
                view->scene()->removeItem(view->polygons[view->polygons.size()-1]);
                break;
            }
        }

        QPolygon polygon(view->polygonPoints[view->polygonPoints.size()-1]);
        QGraphicsPolygonItem* polygonItem = new QGraphicsPolygonItem(polygon);
        polygonItem->setPen(QPen(Qt::black, 1));  // устанавливаем цвет ручки
        polygonItem->setBrush(QBrush(Qt::gray));  // устанавливаем цвет заливки
        view->polygons[view->polygons.size()-1] = polygonItem;
        qDebug() << "polygons: " << view->polygons;

        view->scene()->addItem(polygonItem);
        view->scene()->update();


        QList<QGraphicsItem *> itemss = view->scene()->items();

        qDebug() << "items: " << itemss;
        qDebug() << "points: " << view->polygonPoints;

        QPainterPath path1 = polygonItem->shape();
        QPainterPath path2;
        for (QGraphicsItem *item : view->scene()->items()) {
            path2 = item->shape();
                if ((item->type() == QGraphicsPolygonItem::Type && item->contains(event->pos())&&item!=polygonItem)||(item->type() == QGraphicsPolygonItem::Type && item!=polygonItem && path1.intersects(path2))) {
                    QMessageBox::critical(this, tr("Ошибка"), tr("Нельзя поставить узел препятствия на другое препятствие!"));
                    view->scene()->removeItem(view->polygons[view->polygons.size()-1]);
                    view->polygonPoints[view->polygonPoints.size()-1].removeAt(view->polygonPoints[view->polygonPoints.size()-1].size()-1);
                    QPolygon polygon(view->polygonPoints[view->polygonPoints.size()-1]);
                    QGraphicsPolygonItem* polygonItem = new QGraphicsPolygonItem(polygon);
                    polygonItem->setPen(QPen(Qt::black, 1));  // устанавливаем цвет ручки
                    polygonItem->setBrush(QBrush(Qt::gray));  // устанавливаем цвет заливки
                    view->polygons[view->polygons.size()-1] = polygonItem;

                    view->scene()->addItem(polygonItem);
                    view->scene()->update();
                    break;
                }
        }


        /*
        // Удалите старый полигон из сцены
                if (polygonItem) {
                    QList<QGraphicsItem*> items = view->scene()->items(Qt::SortOrder(), Qt::CoarseGrainingMode::CoarseGrainingPixmap);

                    view->scene()->removeItem();
                    delete polygonItem;
                    polygonItem = nullptr;
                }

                // Создайте новый полигон и добавьте его в сцену
                polygonItem = new QGraphicsPolygonItem(polygon);
                polygonItem->setPen(QPen(Qt::black, 2));
                polygonItem->setBrush(QBrush(Qt::gray));
                scene()->addItem(polygonItem);*/

        }
    if(delete_but_is_clicked){
           for (QGraphicsItem *item : view->scene()->items()) {
                    if (item->type() == QGraphicsPolygonItem::Type && item->contains(event->pos())) {
                        // обработка клика по полигону
                        for (int i=0;i < view->polygonPoints.size();i++){
                            for(QPoint point : view->polygonPoints[i]){
                                if (item->contains(point)){
                                    view->polygonPoints.removeAt(i);
                                    qDebug() << "view->polygonsTexts" << view->polygonsTexts;
                                    QGraphicsTextItem* text = view->polygonsTexts[i];
                                    view->polygonsTexts.removeAt(i);
                                    delete text;
                                    qDebug() << "view->polygonsTexts" << view->polygonsTexts;


                                    QGraphicsPolygonItem *polygonItem = qgraphicsitem_cast<QGraphicsPolygonItem *>(item);
                                    qDebug() << polygonItem;
                                    break;
                                }
                            }
                        }
                        QGraphicsPolygonItem *polygonItem = qgraphicsitem_cast<QGraphicsPolygonItem *>(item);
                        qDebug() << "item" << item;
                        qDebug() << "polygonItem" << polygonItem;
                        view->scene()->removeItem(polygonItem);
                        view->polygons.removeAll(polygonItem);
                        view->polygonsPatency.remove(polygonItem);
                        delete polygonItem;
                        // ...
                    }
            }

    }

}

void MainWindow::onActionTriggered(){
    if (way_but_is_clicked){
//        saveFiguresToXml("my.xml",view->scene());
//        readScene("my.xml",view->scene());
//        qDebug() << "saved and readed";

    }
    else{
        QMessageBox::critical(this, tr("Ошибка"), tr("Для сохранения маршрута сначало необходимо его построить!"));
    }
}

void MainWindow::onAction_2Triggered(){
    saveFiguresToXml("my.xml",view->scene());
    qDebug() << view->scene()->items();
}

void MainWindow::onAction_3Triggered(){
    readScene("my.xml",view->scene());
}

const int row = 800;
const int col = 600;

std::vector < std::vector <float> > Matrix(row, std::vector <float> (col, 1) ); // complete matrix of '1'

// complete matrix with
void CompletePoint()
{
    for (int i=0;i<row;i++){
            for (int k=0;k<col;k++){
                for (QGraphicsItem *item : view->scene()->items()) {
                    if (item->type() == QGraphicsPolygonItem::Type && item->contains(QPoint(i, k))) {
                        QGraphicsPolygonItem *polygonItem = qgraphicsitem_cast<QGraphicsPolygonItem *>(item);
                            if (view->polygonsPatency.value(polygonItem) != 0)
                            {
                                Matrix[i][k] = 1 / view->polygonsPatency.value(polygonItem);
                            }
                            else
                            {
                                Matrix[i][k] = 1000;
                            }
                        }
                    }
            }
    }
}

Astar::Astar()
{

}
Astar::~Astar()
{

}

void Astar::search(Node* startPos, Node* endPos)
{
    if (startPos->x < 0 || startPos->x > row || startPos->y < 0 || startPos->y >col ||
        endPos->x < 0 || endPos->x > row || endPos->y < 0 || endPos->y > col)
    {
        return;
    }

    Node* current;

    this->startPos = startPos;
    this->endPos = endPos;

    openList.push_back(startPos);

    while (openList.size() > 0)
    {
            current = openList[0];
            if (current->x == endPos->x && current->y == endPos->y)
            {
                    // DRAW the WAY (SAVVA and VANYA)
                    qDebug() << "HUI";
                    openList.clear();
                    closeList.clear();
                    break;
            }
            NextStep(current);
            closeList.push_back(current);
            openList.erase(openList.begin());
            sort(openList.begin(), openList.end(), compare);
    }
}


void Astar::checkPoit(int x, int y, Node* father, int g)
{
    if (x < 0 || x > row || y < 0 || y > col)
    {
        return;
    }
    if (this->unWalk(x, y))
    {
        return;
    }
    if (isContains(&closeList, x, y) != -1)
    {
        return;
    }
    int index;
    if ((index = isContains(&openList, x, y)) != -1)
    {
            Node *point = openList[index];
            if (point->g > father->g + g)
            {
                    point->father = father;
                    point->g = father->g + g;
                    point->f = point->g + point->h;
            }
    }
    else
    {
            Node * point = new Node(x, y, father);
            countGHF(point, endPos, g * Matrix[x][y]);
            openList.push_back(point);
    }
}



void Astar::NextStep(Node* current)
{
    checkPoit(current->x-1, current->y, current, WeightW);          //Left
    checkPoit(current->x + 1, current->y, current, WeightW);        //right
    checkPoit(current->x, current->y + 1, current, WeightW);        //up
    checkPoit(current->x, current->y-1, current, WeightW);          //down
    checkPoit(current->x-1, current->y + 1, current, WeightWH);     //top left
    checkPoit(current->x-1, current->y-1, current, WeightWH);       //bottom left
    checkPoit(current->x + 1, current->y-1, current, WeightWH);     //bottom right
    checkPoit(current->x + 1, current->y + 1, current, WeightWH);   //top right
}




int Astar::isContains(vector<Node*>* Nodelist, int x, int y)
{
    for (int i = 0; i < Nodelist->size(); i++)
    {
            if (Nodelist->at(i)->x == x && Nodelist->at(i)->y == y)
            {
                    return i;
            }
    }
    return -1;
}



void Astar::countGHF(Node* sNode, Node* eNode, int g)
{
    int h = (abs(sNode->x - eNode->x) + abs(sNode->y - eNode->y)) * WeightW;
    int currentg = sNode->father->g * Matrix[sNode->father->x][sNode->father->y] + g;
    int f = currentg + h;
    sNode->f = f;
    sNode->h = h;
    sNode->g = currentg;
}


// f comparison
bool Astar::compare(Node* n1, Node* n2)
{
    return n1->f < n2->f;
}


bool Astar::unWalk(int x, int y)
{
    if (Matrix[x][y] != 1000)
    {
        return true;
    }

    else
    {
        return false;
    }
}
