#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <opencv2/dnn.hpp>
#include <opencv2/opencv.hpp>
#include <QDir>
#include <QCoreApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // ----------------------
    // 資料庫初始化
    // ----------------------
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

    // ----------------------
    // DNN 模型載入
    // ----------------------
    QString protoPath = "C:/opencv_project/face/deploy.prototxt";
    QString modelPath = "C:/opencv_project/face/res10_300x300_ssd_iter_140000.caffemodel";

    faceNet = cv::dnn::readNetFromCaffe(protoPath.toStdString(),
                                        modelPath.toStdString());

    if (faceNet.empty()) {
        qDebug() << "Failed to load DNN face model!";
    } else {
        qDebug() << "DNN face model loaded successfully";
    }

    // ----------------------
    // 攝影機初始化
    // ----------------------
    cap.open(0);
    if (!cap.isOpened()) {
        qDebug() << "Camera failed to open!";
        return;
    }

    // ----------------------
    // 計時器初始化
    // ----------------------
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateFrame);
    timer->start(60); // 約 16 FPS

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

    // 縮小影像加速 DNN
    cv::Mat smallFrame;
    cv::resize(frame, smallFrame, cv::Size(300, 300));

    cv::Mat blob = cv::dnn::blobFromImage(smallFrame, 1.0,
                                          cv::Size(300, 300),
                                          cv::Scalar(104.0, 177.0, 123.0),
                                          false, false);

    faceNet.setInput(blob);
    cv::Mat detection = faceNet.forward();

    cv::Mat detectionMat(detection.size[2], detection.size[3], CV_32F, detection.ptr<float>());

    std::vector<cv::Rect> faces;
    for (int i = 0; i < detectionMat.rows; i++) {
        float confidence = detectionMat.at<float>(i, 2);
        if (confidence > 0.5) { // 可調整信心門檻
            int x1 = static_cast<int>(detectionMat.at<float>(i, 3) * frame.cols);
            int y1 = static_cast<int>(detectionMat.at<float>(i, 4) * frame.rows);
            int x2 = static_cast<int>(detectionMat.at<float>(i, 5) * frame.cols);
            int y2 = static_cast<int>(detectionMat.at<float>(i, 6) * frame.rows);

            faces.push_back(cv::Rect(cv::Point(x1, y1), cv::Point(x2, y2)));
        }
    }

    // ----------------------
    // 門控制邏輯
    // ----------------------
    if (!faces.empty() && !facePresent) {
        facePresent = true;
        if (!doorOpen) {
            doorOpen = true;
            ui->label_status->setText("有人\nDoor Open");
            ui->label_status->setStyleSheet("color: green; font-weight: bold;");
            doorTimer->start(3000); // 3 秒後自動鎖門
        }
    } else if (faces.empty()) {
        facePresent = false;
    }

    // 繪製人臉框
    for (const auto &face : faces) {
        cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);
    }

    // 轉成 Qt 可用影像並顯示
    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
    QImage img(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
    ui->label_camera->setPixmap(QPixmap::fromImage(img)
                                    .scaled(ui->label_camera->size(), Qt::KeepAspectRatio));
}

MainWindow::~MainWindow()
{
    cap.release();
    delete ui;
}
