#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    bool add_but_is_clicked;
    bool delete_but_is_clicked;
    bool way_but_is_clicked;
    bool start_but_is_clicked;
    bool finish_but_is_clicked;

//    QGraphicsView* view;


private slots:
    void on_add_but_clicked();

    void on_delete_but_clicked();

    void on_way_but_clicked();

    void on_start_but_clicked();

    void on_finish_but_clicked();

    void turn_off_flags();

    void on_mousePress(QMouseEvent *event);

    void onActionTriggered();

    void onAction_2Triggered();

    void onAction_3Triggered();

    void wheelEvent(QWheelEvent *event) override{}

private:
    Ui::MainWindow *ui;

};
#endif // MAINWINDOW_H
