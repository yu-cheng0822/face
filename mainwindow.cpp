#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QCoreApplication>

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

    QString createTable = "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT";
    for (int i = 1; i <= 128; ++i)
        createTable += QString(", v%1 REAL").arg(i);
    createTable += ");";

    QSqlQuery q;
    if(!q.exec(createTable))
        qDebug() << "Create table failed:" << q.lastError().text();

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

    /* ---------- 攝影機 ---------- */
    cap.open(0);
    if (!cap.isOpened()) {
        qDebug() << "Camera open failed";
        return;
    }

    /* ---------- Timer ---------- */
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

        int userId = -1;
        if (recognizeFace(faceROI, userId)) {
            authorized = true;
        }
    }

    if (authorized && !doorOpen) {
        doorOpen = true;
        ui->label_status->setText("Authorized\nDoor Open");
        ui->label_status->setStyleSheet("color:green; font-weight:bold;");
        doorTimer->start(3000);
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
    cv::Mat vec = embedNet.forward(); // 1x128

    if(addFaceToDB(name, vec)){
        ui->label_status->setText("Registered: " + name);
        ui->label_status->setStyleSheet("color:green; font-weight:bold;");
    } else {
        ui->label_status->setText("Register failed");
        ui->label_status->setStyleSheet("color:red; font-weight:bold;");
    }
}
bool MainWindow::addFaceToDB(const QString &name, const cv::Mat &vec)
{
    cv::Mat vecF;
    if(vec.rows == 128 && vec.cols == 1)
        vecF = vec.t(); // 128x1 -> 1x128
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

    QSqlQuery q("SELECT id, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, "
                "v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, "
                "v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, "
                "v31, v32, v33, v34, v35, v36, v37, v38, v39, v40, "
                "v41, v42, v43, v44, v45, v46, v47, v48, v49, v50, "
                "v51, v52, v53, v54, v55, v56, v57, v58, v59, v60, "
                "v61, v62, v63, v64, v65, v66, v67, v68, v69, v70, "
                "v71, v72, v73, v74, v75, v76, v77, v78, v79, v80, "
                "v81, v82, v83, v84, v85, v86, v87, v88, v89, v90, "
                "v91, v92, v93, v94, v95, v96, v97, v98, v99, v100, "
                "v101, v102, v103, v104, v105, v106, v107, v108, v109, v110, "
                "v111, v112, v113, v114, v115, v116, v117, v118, v119, v120, "
                "v121, v122, v123, v124, v125, v126, v127, v128 FROM users");

    while(q.next())
    {
        int id = q.value(0).toInt();
        cv::Mat dbVec(1,128,CV_32F);
        for(int i=0;i<128;i++){
            dbVec.at<float>(0,i) = q.value(i+1).toFloat();
        }

        cv::Mat diff = vec - dbVec;
        float dist = cv::norm(diff);

        if(dist < 0.6){
            outId = id;
            return true;
        }
        if(dist < 0.6){
            qDebug() << "Recognized ID:" << id << "Distance:" << dist;
            outId = id;
            return true;
        }

    }

    return false;
}
