/**
 * @file mainwindow.h
 * @brief 主視窗類別標頭檔
 * @description 定義人臉辨識門禁系統的主視窗介面
 */

#pragma once

#include <QMainWindow>
#include <QSqlDatabase>
#include <QTimer>
#include <QList>
#include <QString>
#include <QDateTime>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @struct User
 * @brief 使用者資料結構
 * @description 儲存使用者的 ID、姓名及人臉特徵向量
 */
struct User {
    int id;              // 使用者 ID
    QString name;        // 使用者姓名
    cv::Mat vec;         // 人臉特徵向量 (128 維)
};

/**
 * @class MainWindow
 * @brief 主視窗類別
 * @description 實作人臉辨識門禁系統的主要功能，包括即時影像顯示、
 *              人臉註冊、辨識和使用者管理
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief 建構子
     * @param parent 父視窗指標，預設為 nullptr
     */
    explicit MainWindow(QWidget *parent = nullptr);

    /**
     * @brief 解構子
     * @description 釋放資源，關閉攝影機和 UI
     */
    ~MainWindow();

private slots:
    /**
     * @brief 更新影像幀
     * @description 定時器觸發，從攝影機讀取影像並進行人臉偵測與辨識
     */
    void updateFrame();

    /**
     * @brief 註冊按鈕點擊事件處理
     * @description 捕捉當前影像中的人臉並註冊到資料庫
     */
    void on_pushButton_register_clicked();

    /**
     * @brief 刪除按鈕點擊事件處理
     * @description 從資料庫中刪除指定使用者
     */
    void on_pushButton_DELETE_clicked();

private:
    Ui::MainWindow *ui;          // UI 介面指標

    // 資料庫相關
    QSqlDatabase db;             // SQLite 資料庫連線

    // 深度學習模型
    cv::dnn::Net faceNet;        // 人臉偵測網路 (Caffe SSD)
    cv::dnn::Net embedNet;       // 人臉特徵提取網路 (OpenFace)

    // 攝影機與定時器
    cv::VideoCapture cap;        // 攝影機捕捉物件
    QTimer *timer;               // 影像更新定時器
    QTimer *doorTimer;           // 門禁控制定時器
    bool doorOpen = false;       // 門禁狀態旗標

    // 辨識追蹤
    int recognizedUserId = -1;   // 當前辨識到的使用者 ID
    QDateTime recognitionTime;   // 辨識時間
    bool hasWrittenFile = false; // 是否已寫入檔案
    QString workDirPath;         // work 資料夾路徑

    // 使用者快取
    QList<User> usersCache;      // 使用者列表快取 (目前未使用)

    // 模型檔案常數
    inline static const QString MODEL_FACE_DETECTOR = "res10_300x300_ssd_iter_140000.caffemodel";
    inline static const QString MODEL_FACE_PROTOTXT = "deploy.prototxt";
    inline static const QString MODEL_FACE_EMBEDDING = "openface_nn4.small2.v1.t7";

    /**
     * @brief 檢查深度學習模型是否已載入
     * @return 模型已載入返回 true，否則返回 false
     */
    bool isModelsLoaded() const;

    /**
     * @brief 新增人臉資料到資料庫
     * @param name 使用者姓名
     * @param vec 人臉特徵向量 (128 維)
     * @return 成功返回 true，失敗返回 false
     */
    bool addFaceToDB(const QString &name, const cv::Mat &vec);

    /**
     * @brief 辨識人臉
     * @param faceROI 人臉影像區域 (96x96 RGB)
     * @param outId 輸出參數，辨識成功時返回使用者 ID
     * @return 辨識成功返回 true，失敗返回 false
     * @description 與資料庫中的特徵向量比對，使用歐式距離判斷
     */
    bool recognizeFace(const cv::Mat &faceROI, int &outId);

    /**
     * @brief 刪除使用者 (目前未使用)
     * @param name 使用者姓名
     * @return 成功返回 true，失敗返回 false
     */
    bool deleteUser(const QString &name);
};
