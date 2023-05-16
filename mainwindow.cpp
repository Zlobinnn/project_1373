#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mygraphicsview.h"
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
#include <iostream>
#include <cmath>
#include <list>
#include <vector>
#include <algorithm>
#include <QLabel>

MyGraphicsView *view;
int row = 500;
int col = 690;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //Создаём новый объект типа MyGraphicsView и задаём его параметры
    view = new MyGraphicsView;
    view->setMouseTracking(true);
    view->setScene(new QGraphicsScene);

    ui->verticalLayout->addWidget(view);

    //Присоединяем метод отслеживания нажатия кнопки мыши по объекту типа MyGraphicsView и метод обработки нажатия
    connect(view, &MyGraphicsView::mousePress, this, &MainWindow::on_mousePress);

    // Запретить автоматическое изменение масштаба
    view->setRenderHint(QPainter::Antialiasing);
    view->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    view->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    view->setSceneRect(0, 0, 690, 500);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

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
                writer.writeEndElement();
                writer.writeEndElement();
            }
            writer.writeEndElement();
            writer.writeEndElement();
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
            int pixels = 0;
            qDebug()<<view->scene()->items().count();
            for (QGraphicsItem *item : view->scene()->items()) {
                    if (item==view->start) {
                        writer.writeStartElement("start");
                        writer.writeAttribute("x",QString::number(item->scenePos().x()+12));
                        writer.writeAttribute("y",QString::number(item->scenePos().y()+22));
                        writer.writeEndElement();
                    }
                    if (item==view->finish) {
                        writer.writeStartElement("finish");
                        writer.writeAttribute("x",QString::number(item->scenePos().x()+12));
                        writer.writeAttribute("y",QString::number(item->scenePos().y()+22));
                        writer.writeEndElement();
                    }
                    if (item->type() == QGraphicsEllipseItem::Type) {
                        pixels++;
                    }
            }
            writer.writeStartElement("route");
            writer.writeTextElement("distace",QString::number(pixels));
            writer.writeTextElement("speed",QString::number(5));
            writer.writeTextElement("time",QString::number(pixels/5));
            writer.writeEndElement();
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

                writer.writeEndElement();
                writer.writeEndElement();
            }
            writer.writeEndElement();
            writer.writeEndDocument();
            file.close();
}

void readScene(const QString& filePath, QGraphicsScene* scene)
{
    view->scene()->clear();
    view->polygonPoints.clear();
    view->polygonsPatency.clear();
    view->polygons.clear();
    view->polygonsTexts.clear();
    view->start = nullptr;
    view->finish = nullptr;

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
        QGraphicsPolygonItem* polygonItem = view->polygons[i];
        float color = view->polygonsPatency.value(view->polygons[i]);
        polygonItem->setPen(QPen(Qt::black, 1));
        polygonItem->setBrush(QBrush(QColor::fromRgbF(color,color,color)));
        QGraphicsTextItem* textItem = new QGraphicsTextItem(QString::number(color * 100) + "%");
        view->polygonsTexts<<textItem;
        textItem->setParentItem(polygonItem);
        textItem->setDefaultTextColor(Qt::white);
        textItem->setFont(QFont("Arial", 14));
        textItem->setOpacity(1);
        QGraphicsDropShadowEffect* textShadow = new QGraphicsDropShadowEffect();
        textShadow->setBlurRadius(3);
        textShadow->setColor(Qt::black);
        textShadow->setOffset(1,1);
        textItem->setGraphicsEffect(textShadow);
        QRectF textRect = textItem->boundingRect();
        textItem->setPos(polygonItem->boundingRect().center() - textItem->boundingRect().center());
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

        //Создаём объекты типа QLabel (тексты) в нашем приложении
        QLabel *label = new QLabel("Выберите коэффициент проходимости");
        QLabel *label2 = new QLabel("0%");
        QLabel *label3 = new QLabel("100%");

        // Добавляем виджеты
        ui->gridLayout_2->addWidget(label,0,1,Qt::AlignCenter);
        ui->gridLayout_2->addWidget(label2,0,0,Qt::AlignLeft);
        ui->gridLayout_2->addWidget(label3,0,2,Qt::AlignRight);

    }
    else{
        add_but_is_clicked = false;

        QSlider* slider = findChild<QSlider*>();

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

        //Удаляем тексты с приложения
        for (int i=0;i<3;i++){
            QLabel *label = findChild<QLabel*>();
            delete label;
        }

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

        QPushButton* delete_but = findChild<QPushButton*>("delete_but");
        delete_but->setStyleSheet("border: 1px solid blue;");
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
    //Очищаем маршрут
    if (!way_but_is_clicked){
        for (QGraphicsItem *item : view->scene()->items()) {
                 if (item->type() == QGraphicsEllipseItem::Type) {
                        view->scene()->removeItem(item);
                 }
        }
        qDebug() << view->polygons.size() << view->polygons;
        qDebug() << view->polygonPoints.size() << view->polygonPoints;
        qDebug() << view->polygonsPatency.size() << view->polygonsPatency;
    }

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
                item->setPos(point.x()-12, point.y()-22);
                item->setZValue(1);

                view->scene()->addItem(item);
                qDebug() << item->scenePos();

                qDebug() << item->scenePos();
            }
            else{
                view->start->setPos(point.x()-12, point.y()-22);
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
                item->setPos(point.x()-12, point.y()-22);
                item->setZValue(1);
                view->scene()->addItem(item);
            }
            else{
                view->finish->setPos(point.x()-12, point.y()-22);
            }
            qDebug() << view->scene()->items();
        }
        else{
            QMessageBox::critical(this, tr("Ошибка"), tr("Невозможно поставить точку финиша на препятствие!"));
        }
    }

    //Обрабатываем действия при нажатой кнопке "Добавить"
    if ((event->button() == Qt::LeftButton)&&(add_but_is_clicked)) {
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
        polygonItem->setPen(QPen(Qt::black, 1));
        polygonItem->setBrush(QBrush(Qt::gray));
        view->polygons[view->polygons.size()-1] = polygonItem;
        qDebug() << "polygons: " << view->polygons;

        view->scene()->addItem(polygonItem);
        view->scene()->update();


        QList<QGraphicsItem *> itemss = view->scene()->items();

        qDebug() << "items: " << itemss;
        qDebug() << "points: " << view->polygonPoints;

        QPainterPath path1 = polygonItem->shape();
        QPainterPath path2;

        //Проверяем, не соприкасаются ли полигоны
        for (QGraphicsItem *item : view->scene()->items()) {
            path2 = item->shape();
                if ((item->type() == QGraphicsPolygonItem::Type && item->contains(event->pos())&&item!=polygonItem)||(item->type() == QGraphicsPolygonItem::Type && item!=polygonItem && path1.intersects(path2))) {
                    QMessageBox::critical(this, tr("Ошибка"), tr("Нельзя поставить узел препятствия на другое препятствие!"));
                    view->scene()->removeItem(view->polygons[view->polygons.size()-1]);
                    view->polygonPoints[view->polygonPoints.size()-1].removeAt(view->polygonPoints[view->polygonPoints.size()-1].size()-1);
                    QPolygon polygon(view->polygonPoints[view->polygonPoints.size()-1]);
                    QGraphicsPolygonItem* polygonItem = new QGraphicsPolygonItem(polygon);
                    polygonItem->setPen(QPen(Qt::black, 1));
                    polygonItem->setBrush(QBrush(Qt::gray));
                    view->polygons[view->polygons.size()-1] = polygonItem;

                    view->scene()->addItem(polygonItem);
                    view->scene()->update();
                    break;
                }
        }
    }

    //Обрабатываем удаление
    if(delete_but_is_clicked){
        //Смотрим, попал ли клик по полигону
           for (QGraphicsItem *item : view->scene()->items()) {
                    if (item->type() == QGraphicsPolygonItem::Type && item->contains(event->pos())) {
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
                    }
            }

    }

}

void MainWindow::onActionTriggered(){
    //Обрабатываем нажатие на кнопку "Сохранить маршрут"
    if (way_but_is_clicked){
        saveSceneToXml("scene.xml",view->scene());
    }
    else{
        QMessageBox::critical(this, tr("Ошибка"), tr("Для сохранения маршрута сначала необходимо его построить!"));
    }
}

void MainWindow::onAction_2Triggered(){
    //Обрабатываем нажатие на кнопку "Сохранить карту препятствий"
    saveFiguresToXml("my.xml",view->scene());
    qDebug() << view->scene()->items();
}

void MainWindow::onAction_3Triggered(){
    //Обрабатываем нажатие на кнопку "Загрузить карту препятствий"
    readScene("my.xml",view->scene());
}

class Vector2
{
    int x, y;
public:
    Vector2(int _x, int _y) : x(_x), y(_y) {}
    Vector2() = default;
    Vector2 operator +(const Vector2& other) {
        Vector2 temp;
        temp.x = this->x + other.x;
        temp.y = this->y + other.y;
        return temp;
    }
    int getX() const { return x; }
    int getY() const { return y; }

    friend class Map;
};

class Map
{
    int size;
public:
    friend struct Node;
    std::vector<float> data;
    Map() = default;
    Map(int _size) : size(_size) {
        data.resize(size);
        for (int i = 0; i < size; ++i) data[i] = 1;
    }
    void display() const {
        for (int i = 1; i <= size; ++i) {
        }
    }
    bool getIfInDanger(Vector2 position) const {
        if (position.y < 0) position.y = 0;
        if (position.x < 0) position.x = 0;
        if (position.y >= row) position.y = row - 1;
        if (position.x >= col) position.x = col - 1;
        return(data[position.getX() + (position.getY() * col)] == 0);
    }
    void setElement(float&& asda, Vector2 position) {
        data[position.getX() + (position.getY() * col)] = asda;
    }
};

struct Node
{
    Vector2 position;
    float G, H, F;
    Node* parent = nullptr;

    Node() = default;
    Node(const Node& other) = default;
    Node(Vector2 pos) :position(pos) {};

    void calc(const Vector2& endPos, class Map& map) {
        H = static_cast<int>((abs(static_cast<double>(position.getX() - endPos.getX())) + abs(static_cast<double>(position.getY() - endPos.getY()))));
        G = parent ? parent->G + 1 : 1;

        F = G + H;
        F /= map.data[position.getX() + position.getY()*col];
    }

    bool operator==(const Node& other) const {
        return (position.getX() == other.position.getX() && position.getY() == other.position.getY());
    }
    bool operator!=(const Node& other) const {
        return !(*this == other);
    }
    bool operator<(const Node& other)  const {
        return(F < other.F);
    }
};

class Solver
{
    Vector2 startPos, endPos;
    std::vector<Vector2> directions;
    Map map;
public:
    Solver(Vector2 _startPos, Vector2 _endPos, int size) : startPos(_startPos), endPos(_endPos) {
        Map temp(size);
        map = temp;


        for (int i=0;i<row;i++){
                for (int k=0;k<col;k++){
                    for (QGraphicsItem *item : view->scene()->items()) {
                        if (item->type() == QGraphicsPolygonItem::Type && item->contains(QPoint(i, k))) {
                            QGraphicsPolygonItem *polygonItem = qgraphicsitem_cast<QGraphicsPolygonItem *>(item);
                                if (view->polygonsPatency.value(polygonItem) != 0)
                                {
                                    map.setElement(view->polygonsPatency.value(polygonItem), Vector2(i, k));
                                    //qDebug() << view->polygonsPatency.value(polygonItem);
                                }
                                else
                                {
                                    map.setElement(0, Vector2(i, k));
                                }
                            }
                        }
                }
        }

        // направления движения
        directions.resize(8);
        directions[0] = Vector2(-1, 1);
        directions[1] = Vector2(-1, 0);
        directions[2] = Vector2(-1, -1);
        directions[3] = Vector2(0, 1);
        directions[4] = Vector2(0, -1);
        directions[5] = Vector2(1, 1);
        directions[6] = Vector2(1, 0);
        directions[7] = Vector2(1, -1);
    }
    std::vector<float> aStar() {
        Node startNode(startPos);
        Node goalNode(Vector2(endPos.getX(), endPos.getY()));

        if (map.getIfInDanger(startNode.position) || map.getIfInDanger(goalNode.position)) {
            std::cout << "Either the start of this map is obstructed or so is the end.";
            std::vector<float> a;
            a.push_back(0);
            return a;
        }

        std::list<Node> openList;
        std::list<Node> closedList;

        startNode.calc(endPos, map);
        openList.push_back(startNode);

        while (!openList.empty()) {
            auto current = Node(*std::min_element(openList.begin(), openList.end()));

            current.calc(endPos, map);

            closedList.push_back(current);
            openList.remove(current);
            if (current == goalNode) break;

            for (auto& direction : directions) {
                Node successor(direction + current.position);

                if (map.getIfInDanger(successor.position) || successor.position.getX() > col - 1 ||
                    successor.position.getY() > row - 1 || successor.position.getX() < 0 ||
                    successor.position.getY() < 0 ||
                    std::find(closedList.begin(), closedList.end(), successor) != closedList.end()) {
                    continue;
                }

                successor.calc(endPos, map);

                auto inOpen = std::find(openList.begin(), openList.end(), successor);
                if (inOpen == openList.end()) {
                    successor.parent = &closedList.back();
                    successor.calc(endPos, map);

                    openList.push_back(successor);
                }
                else
                    if (successor.G < inOpen->G) successor.parent = &closedList.back();
            }
        }

        if (!openList.size()) {
            std::cout << "No path has been found\n";
            std::vector<float> a;
            a.push_back(0);
            return a;
        }

        auto inClosed = std::find(closedList.begin(), closedList.end(), goalNode);
        if (inClosed != closedList.end()) {
            while (*inClosed != startNode) {
                map.setElement(2, inClosed->position); //путь - это двойка
                //qDebug() << inClosed->position.getX() << inClosed->position.getY();

                QGraphicsEllipseItem* point = new QGraphicsEllipseItem(0, 0, 0.5, 0.5);
                point->setPos(inClosed->position.getX(), inClosed->position.getY());
                point->setBrush(Qt::red);
                view->scene()->addItem(point);

                *inClosed = *inClosed->parent;
            }
        }

        map.display();
        return map.data;
    }
};

void MainWindow::on_way_but_clicked()
{
    //Обрабатываем кнопку "Построить маршрут"
    if (!add_but_is_clicked){
        if (view->start!=nullptr && view->finish!=nullptr){
            turn_off_flags();
            way_but_is_clicked = true;

            //Удаляем предыдущий маршрут
            for (QGraphicsItem *item : view->scene()->items()) {
                     if (item->type() == QGraphicsEllipseItem::Type) {
                            view->scene()->removeItem(item);
                     }
             }

            int x=view->start->pos().x()+12, y=view->start->pos().y()+22;
            Solver solve(Vector2(x-1, y-1), Vector2(view->finish->pos().x()+11, view->finish->pos().y()+21), row*col);
            std::vector<float> vec = solve.aStar();


            qDebug() << "Ok";
        }
        else{
            QMessageBox::critical(this, tr("Ошибка"), tr("Для построения маршрута сначала необходимо задать начальную и конечные точки!"));
        }
    }
}
