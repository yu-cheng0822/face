#pragma once
#include <QMainWindow>
#include <QTimer>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateFrame();
    void on_pushButton_register_clicked();

private:
    Ui::MainWindow *ui;

    QSqlDatabase db;

    cv::VideoCapture cap;
    cv::dnn::Net faceNet;
    cv::dnn::Net embedNet;

    QTimer *timer;
    QTimer *doorTimer;
    bool doorOpen = false;

    bool recognizeFace(const cv::Mat &faceROI, int &outId);
    bool addFaceToDB(const QString &name, const cv::Mat &vec);
};
