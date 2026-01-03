# Face Detection Door Control System

## 專案概述 (Project Overview)

這是一個基於 Qt 和 OpenCV 開發的人臉偵測門控系統。當攝影機偵測到人臉時，系統會自動模擬開門，並在 3 秒後自動關閉。

This is a face detection door control system developed with Qt and OpenCV. When a face is detected by the camera, the system automatically simulates opening a door, which closes automatically after 3 seconds.

## 主要功能 (Key Features)

- **即時人臉偵測** - 使用 OpenCV 的 Haar Cascade 分類器進行即時人臉辨識
- **自動門控制** - 偵測到人臉時自動開門，3 秒後自動關閉
- **視覺化介面** - 使用 Qt 提供友善的圖形化使用者介面
- **攝影機預覽** - 即時顯示攝影機畫面，並在偵測到的人臉周圍繪製綠色方框
- **狀態顯示** - 清楚顯示門的開啟/關閉狀態

Features:
- **Real-time Face Detection** - Uses OpenCV's Haar Cascade classifier for real-time face recognition
- **Automatic Door Control** - Automatically opens door when face detected, closes after 3 seconds
- **Visual Interface** - User-friendly GUI built with Qt
- **Camera Preview** - Real-time camera feed with green rectangles around detected faces
- **Status Display** - Clear indication of door open/locked status

## 技術棧 (Technology Stack)

- **Qt 6** - 圖形使用者介面框架
- **OpenCV 4.12.0** - 電腦視覺與影像處理函式庫
- **C++17** - 程式語言
- **Haar Cascade** - 人臉偵測演算法

## 系統需求 (Requirements)

- Qt 6.x 或更高版本
- OpenCV 4.12.0 或相容版本
- 支援 C++17 的編譯器 (MSVC 2019 或更新版本)
- 可用的攝影機裝置

Requirements:
- Qt 6.x or higher
- OpenCV 4.12.0 or compatible version
- C++17 compatible compiler (MSVC 2019 or newer)
- Available camera device

## 專案結構 (Project Structure)

```
face/
├── main.cpp                                    # 應用程式進入點
├── mainwindow.h                                # 主視窗標頭檔
├── mainwindow.cpp                              # 主視窗實作
├── mainwindow.ui                               # Qt UI 設計檔案
├── face.pro                                    # Qt 專案設定檔
├── haarcascade_frontalface_default.xml         # 人臉偵測模型
├── msvc_make.bat                               # Windows 建置腳本
└── README.md                                   # 專案說明文件
```

## 安裝與設定 (Installation and Setup)

### 1. 安裝依賴項目

確保已安裝以下軟體：
- Qt 6 (含 Qt Creator)
- OpenCV 4.12.0

### 2. 設定 OpenCV 路徑

編輯 `face.pro` 檔案，修改 OpenCV 安裝路徑：

```qmake
INCLUDEPATH += C:/opencv/build/include

CONFIG(debug, debug|release) {
    LIBS += -LC:/opencv/build/x64/vc16/lib \
            -lopencv_world4120d
} else {
    LIBS += -LC:/opencv/build/x64/vc16/lib \
            -lopencv_world4120
}
```

### 3. 建置專案 (Build)

#### 使用 Qt Creator:
1. 開啟 Qt Creator
2. 載入 `face.pro` 檔案
3. 選擇適當的 Kit (MSVC 編譯器)
4. 點擊建置按鈕

#### 使用命令列:
```bash
qmake face.pro
nmake  # 或使用 msvc_make.bat
```

### 4. 執行應用程式

建置完成後，執行生成的可執行檔。確保 `haarcascade_frontalface_default.xml` 檔案與可執行檔在同一目錄下。

## 使用方式 (Usage)

1. **啟動應用程式** - 執行程式後，攝影機會自動啟動
2. **人臉偵測** - 將臉部對準攝影機，系統會自動偵測
3. **觀察狀態** - 當偵測到人臉時：
   - 畫面上會在人臉周圍顯示綠色方框
   - 狀態標籤會顯示「有人 Door Open」(綠色)
   - 3 秒後自動切換為「Door Locked」(紅色)

## 核心功能說明 (Core Functionality)

### 人臉偵測流程
1. 從攝影機擷取影像幀 (60ms 間隔)
2. 將彩色影像轉換為灰階
3. 使用 Haar Cascade 分類器偵測人臉
4. 在原始影像上繪製偵測框
5. 更新 UI 顯示

### 門控邏輯
- 偵測到人臉且門未開啟 → 開門並啟動 3 秒計時器
- 3 秒後 → 自動關閉門
- 門開啟期間再次偵測到人臉 → 不重複觸發

## 開發者資訊 (Developer Info)

此專案展示了如何整合 Qt GUI 框架與 OpenCV 電腦視覺函式庫，實現一個簡單但實用的人臉偵測應用。可作為學習以下技術的參考：

- Qt 信號與槽機制
- QTimer 的使用
- OpenCV 影像處理
- 即時視訊串流處理
- GUI 與電腦視覺的整合

This project demonstrates how to integrate Qt GUI framework with OpenCV computer vision library to create a simple yet practical face detection application. It serves as a reference for learning:

- Qt signals and slots mechanism
- QTimer usage
- OpenCV image processing
- Real-time video stream processing
- Integration of GUI and computer vision

## 授權 (License)

此專案為教育與學習用途。

This project is for educational and learning purposes.

## 問題回報 (Issue Reporting)

如有任何問題或建議，請在 GitHub 專案頁面提交 Issue。

For any issues or suggestions, please submit an issue on the GitHub project page.