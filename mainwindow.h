#include <QMainWindow>
#include <QTimer>
#include <opencv2/opencv.hpp>

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
};
