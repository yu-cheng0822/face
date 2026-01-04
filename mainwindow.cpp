#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QCoreApplication>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("users.db");

    if (!db.open()) {
        qDebug() << "DB open failed:" << db.lastError().text();
    } else {
        qDebug() << "DB opened";
    }

    QSqlQuery q;
    if(!q.exec("DROP TABLE IF EXISTS users")) {
        qDebug() << "Failed to drop users table:" << q.lastError().text();
    }

    QString createTable = "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT";
    for (int i = 1; i <= 128; ++i)
        createTable += QString(", v%1 REAL").arg(i);
    createTable += ");";

    if(!q.exec(createTable))
        qDebug() << "Create table failed:" << q.lastError().text();


    usersCache.clear();

    QString basePath = QCoreApplication::applicationDirPath() + "/face/";

    faceNet = cv::dnn::readNetFromCaffe(
        (basePath + "deploy.prototxt").toStdString(),
        (basePath + "res10_300x300_ssd_iter_140000.caffemodel").toStdString()
        );

    embedNet = cv::dnn::readNetFromTorch(
        (basePath + "openface_nn4.small2.v1.t7").toStdString()
        );

    if (faceNet.empty() || embedNet.empty()) {
        qDebug() << "DNN model load FAILED";
    } else {
        qDebug() << "DNN models loaded OK";
    }

    cap.open(0);
    if (!cap.isOpened()) {
        qDebug() << "Camera open failed";
        return;
    }

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateFrame);
    timer->start(60);

    doorTimer = new QTimer(this);
    doorTimer->setSingleShot(true);
    connect(doorTimer, &QTimer::timeout, this, [=]() {
        doorOpen = false;
        ui->label_status->setText("Door Locked");
        ui->label_status->setStyleSheet("color:red; font-weight:bold;");
    });
}

MainWindow::~MainWindow()
{
    cap.release();
    delete ui;
}

void MainWindow::updateFrame()
{
    cv::Mat frame;
    cap >> frame;
    if (frame.empty()) return;

    cv::Mat blob = cv::dnn::blobFromImage(frame, 1.0, cv::Size(300,300),
                                          cv::Scalar(104,177,123), false, false);
    faceNet.setInput(blob);
    cv::Mat det = faceNet.forward();
    cv::Mat detMat(det.size[2], det.size[3], CV_32F, det.ptr<float>());

    bool authorized = false;
    int userId = -1;

    for (int i = 0; i < detMat.rows; i++) {
        float conf = detMat.at<float>(i, 2);
        if (conf < 0.6) continue;

        int x1 = int(detMat.at<float>(i, 3) * frame.cols);
        int y1 = int(detMat.at<float>(i, 4) * frame.rows);
        int x2 = int(detMat.at<float>(i, 5) * frame.cols);
        int y2 = int(detMat.at<float>(i, 6) * frame.rows);

        cv::Rect faceRect(cv::Point(x1,y1), cv::Point(x2,y2));
        cv::rectangle(frame, faceRect, cv::Scalar(0,255,0), 2);

        cv::Mat faceROI = frame(faceRect).clone();
        cv::cvtColor(faceROI, faceROI, cv::COLOR_BGR2RGB);
        cv::resize(faceROI, faceROI, cv::Size(96,96));

        if (recognizeFace(faceROI, userId)) {
            authorized = true;
        }
    }

    // 更新 UI
    if(authorized){
        ui->label_status->setText("Authorized\nID: " + QString::number(userId));
        ui->label_status->setStyleSheet("color:green; font-weight:bold;");
        if(!doorOpen){
            doorOpen = true;
            doorTimer->start(3000);
        }
    } else {
        ui->label_status->setText("Door Locked");
        ui->label_status->setStyleSheet("color:red; font-weight:bold;");
    }

    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
    QImage img(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
    ui->label_camera->setPixmap(QPixmap::fromImage(img)
                                    .scaled(ui->label_camera->size(), Qt::KeepAspectRatio));
}

void MainWindow::on_pushButton_register_clicked()
{
    QString name = ui->lineEdit_name->text().trimmed();
    if(name.isEmpty()){
        ui->label_status->setText("Name cannot be empty");
        ui->label_status->setStyleSheet("color:red; font-weight:bold;");
        return;
    }

    cv::Mat frame;
    cap >> frame;
    if(frame.empty()){
        ui->label_status->setText("Camera capture failed");
        ui->label_status->setStyleSheet("color:red; font-weight:bold;");
        return;
    }

    cv::Mat blob = cv::dnn::blobFromImage(frame,1.0,cv::Size(300,300),cv::Scalar(104,177,123),false,false);
    faceNet.setInput(blob);
    cv::Mat det = faceNet.forward();
    cv::Mat detMat(det.size[2], det.size[3], CV_32F, det.ptr<float>());

    int bestIdx=-1;
    float maxConf=0;
    for(int i=0;i<detMat.rows;i++){
        float conf = detMat.at<float>(i,2);
        if(conf > maxConf){
            maxConf = conf;
            bestIdx = i;
        }
    }

    if(bestIdx==-1 || maxConf<0.6){
        ui->label_status->setText("No face detected");
        ui->label_status->setStyleSheet("color:red; font-weight:bold;");
        return;
    }

    int x1=int(detMat.at<float>(bestIdx,3)*frame.cols);
    int y1=int(detMat.at<float>(bestIdx,4)*frame.rows);
    int x2=int(detMat.at<float>(bestIdx,5)*frame.cols);
    int y2=int(detMat.at<float>(bestIdx,6)*frame.rows);
    cv::Rect faceRect(cv::Point(x1,y1), cv::Point(x2,y2));

    cv::Mat faceROI = frame(faceRect).clone();
    cv::cvtColor(faceROI, faceROI, cv::COLOR_BGR2RGB);
    cv::resize(faceROI, faceROI, cv::Size(96,96));

    cv::Mat blobFace = cv::dnn::blobFromImage(faceROI,1.0/255.0,cv::Size(96,96),cv::Scalar(0,0,0),true,false);
    embedNet.setInput(blobFace);
    cv::Mat vec = embedNet.forward();

    if(addFaceToDB(name, vec)){
        ui->label_status->setText("Registered: " + name);
        ui->label_status->setStyleSheet("color:green; font-weight:bold;");
    } else {
        ui->label_status->setText("Register failed");
        ui->label_status->setStyleSheet("color:red; font-weight:bold;");
    }
}

void MainWindow::on_pushButton_DELETE_clicked()
{
    QString name = ui->lineEdit_DELETNAME->text().trimmed();
    if(name.isEmpty()){
        ui->label_status->setText("Name cannot be empty");
        ui->label_status->setStyleSheet("color:red; font-weight:bold;");
        return;
    }

    QSqlQuery q;
    q.prepare("DELETE FROM users WHERE name = ?");
    q.addBindValue(name);

    if(!q.exec()){
        qDebug() << "Delete failed:" << q.lastError().text();
        ui->label_status->setText("Delete failed");
        ui->label_status->setStyleSheet("color:red; font-weight:bold;");
        return;
    }

    if(q.numRowsAffected() == 0){
        ui->label_status->setText("User not found: " + name);
        ui->label_status->setStyleSheet("color:orange; font-weight:bold;");
    } else {
        ui->label_status->setText("Deleted: " + name);
        ui->label_status->setStyleSheet("color:green; font-weight:bold;");
    }

    qDebug() << "Deleted user:" << name;
}

bool MainWindow::addFaceToDB(const QString &name, const cv::Mat &vec)
{
    cv::Mat vecF;
    if(vec.rows == 128 && vec.cols == 1)
        vecF = vec.t();
    else
        vecF = vec.clone();

    vecF.convertTo(vecF, CV_32F);

    if(vecF.rows != 1 || vecF.cols != 128){
        qDebug() << "Vector size invalid:" << vecF.rows << "x" << vecF.cols;
        return false;
    }

    QString cmd = "INSERT INTO users (name";
    QString vals = " VALUES (?";
    for(int i=1; i<=128; i++){
        cmd += QString(", v%1").arg(i);
        vals += ", ?";
    }
    cmd += ")";
    vals += ")";
    cmd += vals;

    QSqlQuery q;
    q.prepare(cmd);
    q.addBindValue(name);
    for(int i=0; i<128; i++){
        q.addBindValue(vecF.at<float>(0,i));
    }

    if(!q.exec()){
        qDebug() << "Insert failed:" << q.lastError().text();
        return false;
    }
    return true;
}

bool MainWindow::recognizeFace(const cv::Mat &faceROI, int &outId)
{
    cv::Mat blob = cv::dnn::blobFromImage(faceROI, 1.0/255.0, cv::Size(96,96),
                                          cv::Scalar(0,0,0), true, false);
    embedNet.setInput(blob);
    cv::Mat vec = embedNet.forward(); // 1x128

    QString selectSql = "SELECT id";
    for(int i=1;i<=128;i++) selectSql += QString(", v%1").arg(i);
    selectSql += " FROM users";

    QSqlQuery q(selectSql);

    while(q.next())
    {
        int id = q.value(0).toInt();
        cv::Mat dbVec(1,128,CV_32F);
        for(int i=0;i<128;i++){
            dbVec.at<float>(0,i) = q.value(i+1).toFloat();
        }

        float dist = cv::norm(vec - dbVec);
        if(dist < 0.9){
            qDebug() << "Recognized ID:" << id << "Distance:" << dist;
            outId = id;
            return true;
        }
    }

    return false;
}
