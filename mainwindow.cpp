#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    cap.open(0);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateFrame);
    timer->start(60);
}
void MainWindow::updateFrame()
{
    cv::Mat frame;
    cap >> frame;
    if (frame.empty()) return;

    // 轉為灰階方便人臉辨識
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

    // 載入Haar Cascade分類器
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

    // 畫出偵測到的人臉框
    for (size_t i = 0; i < faces.size(); i++) {
        cv::rectangle(frame, faces[i], cv::Scalar(0, 255, 0), 2);
    }

    // 轉RGB並顯示到Qt Label
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
