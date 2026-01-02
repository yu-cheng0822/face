#include <QMainWindow>
#include <QTimer>
#include <opencv2/opencv.hpp>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <opencv2/dnn.hpp>s

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
    void updateFrame();   // 更新影像

private:
    Ui::MainWindow *ui;
    QTimer *timer;
    cv::VideoCapture cap; // 攝影機
    bool doorOpen = false;
    QTimer *doorTimer;
    bool facePresent = false;
    QSqlDatabase db;
    cv::dnn::Net faceNet;
};
