#pragma once

#include <QMainWindow>
#include <QSqlDatabase>
#include <QTimer>
#include <QList>
#include <QString>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct User {
    int id;
    QString name;
    cv::Mat vec;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateFrame();
    void on_pushButton_register_clicked();
    void on_pushButton_DELETE_clicked();

private:
    Ui::MainWindow *ui;

    QSqlDatabase db;
    cv::dnn::Net faceNet;
    cv::dnn::Net embedNet;
    cv::VideoCapture cap;
    QTimer *timer;
    QTimer *doorTimer;
    bool doorOpen = false;

    QList<User> usersCache;

    bool addFaceToDB(const QString &name, const cv::Mat &vec);
    bool recognizeFace(const cv::Mat &faceROI, int &outId);
    bool deleteUser(const QString &name);
};
