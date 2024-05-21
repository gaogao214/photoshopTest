#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <fstream>
#include <vector>
#include <QFile>
#include <QFileDialog>
using namespace cv;
using namespace std;

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

public:
    void enhanceLocalContrast();
    void rawEnhanceLocalContrast();
    void selectProcessMode(QString filename);
    void getImageWithFilePath(const QString &filePath);
    void saveRawPicture( QString filepath );
private slots:
    void on_open_pushButton_clicked();

    void on_save_pushButton_clicked();

private:
    Ui::Widget *ui;

    cv::Mat img ;
    QImage denoisedQImage;
    QString filename;
    QSize m_imgSize;
    Mat img_gray, result;
    Mat clahe_result;
    int Height = 1536;//2496;
    int Width = 1536;//3008;

};
#endif // WIDGET_H
