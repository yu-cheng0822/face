#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("users.db");

    if (!db.open()) {
        qDebug() << "DB open failed:" << db.lastError().text();
    } else {
        qDebug() << "DB opened";
    }
    QSqlQuery query("SELECT id, name FROM users");

    while (query.next()) {
        qDebug() << "ID:" << query.value(0).toInt()
        << "Name:" << query.value(1).toString();
    }
    ui->setupUi(this);


    cap.open(0);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateFrame);
    timer->start(60);
    doorTimer = new QTimer(this);
    doorTimer->setSingleShot(true);

    connect(doorTimer, &QTimer::timeout, this, [=]() {
        doorOpen = false;
        ui->label_status->setText("Door Locked");
        ui->label_status->setStyleSheet("color: red; font-weight: bold;");
    });


}
void MainWindow::updateFrame()
{
    cv::Mat frame;
    cap >> frame;
    if (frame.empty()) return;

    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

    static cv::CascadeClassifier face_cascade;
    static bool loaded = false;
    if(!loaded) {
        if(!face_cascade.load("haarcascade_frontalface_default.xml")) {
            qDebug() << "Error loading face cascade";
            return;
        }
        loaded = true;
    }

    std::vector<cv::Rect> faces;
    face_cascade.detectMultiScale(gray, faces, 1.1, 3, 0, cv::Size(30, 30));
    if (!faces.empty() && !facePresent) {
        facePresent = true;

        if (!doorOpen) {
            doorOpen = true;
            ui->label_status->setText("有人\nDoor Open");
            ui->label_status->setStyleSheet("color: green; font-weight: bold;");
            doorTimer->start(3000);
        }
    }
    else if (faces.empty()) {
        facePresent = false;
    }

    for (size_t i = 0; i < faces.size(); i++) {
        cv::rectangle(frame, faces[i], cv::Scalar(0, 255, 0), 2);
    }

    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
    QImage img(
        frame.data,
        frame.cols,
        frame.rows,
        frame.step,
        QImage::Format_RGB888
        );

    ui->label_camera->setPixmap(
        QPixmap::fromImage(img).scaled(
            ui->label_camera->size(),
            Qt::KeepAspectRatio
            )
        );

}

MainWindow::~MainWindow()
{
    cap.release();
    delete ui;
}
