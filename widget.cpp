#include "widget.h"
#include "ui_widget.h"
#include <QDebug>

#include <opencv2/opencv.hpp>
#include <filesystem>
#include <fstream>
#include <QMessageBox>

#define CV_SORT_EVERY_ROW    0
#define CV_SORT_EVERY_COLUMN 1
#define CV_SORT_ASCENDING    0
#define CV_SORT_DESCENDING   16

// 函数：将QImage转换为cv::Mat
cv::Mat QImageToMat(const QImage &image)
{
    // cv::Mat mat(image.height(), image.width(), CV_16UC1, const_cast<uchar*>(image.bits()), image.bytesPerLine());
    cv::Mat mat(image.height(), image.width(), CV_8U, const_cast<uchar*>(image.bits()), image.bytesPerLine());
    return mat.clone(); // 返回一个深拷贝，以避免数据共享问题
}

// 函数：将cv::Mat转换回QImage
QImage MatToQImage(const cv::Mat &mat)
{
    if(mat.empty())
        qDebug()<<"mat == null!!!";

    return   QImage((const unsigned char*)mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale16);
    // return QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale16).copy();
}

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

}

Widget::~Widget()
{
    delete ui;
}

//支持png、tif文件
void Widget::enhanceLocalContrast()
{
    img = imread(filename.toStdString());

    cvtColor(img, img_gray, cv::COLOR_BGR2GRAY);
    // equalizeHist(img_gray, result);//使图像的灰度直方图分布更佳均匀的变换方法(HE算法)

    //setTilesGridSize
    //clipLimit：对比对限制阈值，默认为40；tileGridSize：直方图均衡的栅格尺寸，输入图像将会按照该尺寸分隔后进行局部直方图均衡，默认是8×8大小；
    //src：输入图像，8bit单通道；   dst：均衡后的输出图像，类型同src；
    cv::Ptr<CLAHE> clahe = cv::createCLAHE();
    clahe->setClipLimit(4);
    clahe->setTilesGridSize(cv::Size(10, 10));//块大小，用于将图片分割为小块进行直方图均衡化
    clahe->apply(img_gray, clahe_result);

    // imshow("src", img_gray);
    // imshow("clahe_result", clahe_result);

    denoisedQImage=QImage(clahe_result.data, clahe_result.cols, clahe_result.rows, /*QImage::Format_Grayscale16*/QImage::Format_Grayscale8);
    QPixmap pix = QPixmap::fromImage(denoisedQImage);

    ui->label->setScaledContents(true);//图片自适应label大小
    ui->label->setPixmap(QPixmap::fromImage(denoisedQImage));
}

//支持raw文件
void Widget::rawEnhanceLocalContrast()
{
    getImageWithFilePath(filename);

    if(img.empty())
        qDebug()<<"img enpty!!!";

    cv::Ptr<CLAHE> clahe = cv::createCLAHE();
    clahe->setClipLimit(4);
    clahe->setTilesGridSize(cv::Size(10, 10));//块大小，用于将图片分割为小块进行直方图均衡化
    clahe->apply(img, clahe_result);

    denoisedQImage=QImage(clahe_result.data, clahe_result.cols, clahe_result.rows, /*QImage::Format_Grayscale16*/QImage::Format_Grayscale16);
    QPixmap pix = QPixmap::fromImage(denoisedQImage);
    ui->label->setScaledContents(true);//图片自适应label大小
    ui->label->setPixmap(QPixmap::fromImage(denoisedQImage));
}

void Widget::selectProcessMode(QString filename)
{

    if(filename.contains(".raw"))
    {
          rawEnhanceLocalContrast();
    }else
    {
          enhanceLocalContrast();
    }

}

void Widget::on_save_pushButton_clicked()
{

   QString path = filename.mid(0,filename.size()-4);

   QString pictureFormat = filename.mid(filename.size()-4,filename.size()-1);
   QString filepath = path +"_new"+pictureFormat;
   qDebug()<<"  filepath"<<filepath;
   if(pictureFormat.contains("raw"))
   {
       saveRawPicture(filepath);
   }else
   {
       imwrite(filepath.toStdString(), clahe_result);
   }


}

void Widget::on_open_pushButton_clicked()
{
    filename = QFileDialog::getOpenFileName(this, tr("Open Image"), QCoreApplication::applicationDirPath(), tr(" *raw *png *tif"));   //打开图片文件，选择图片
    if (filename.isEmpty()){return;}
    else {qDebug() << "filename: " << filename;}

    selectProcessMode(filename);

}

void Widget::getImageWithFilePath(const QString &filePath)
{
    int width = 1536;
    int height = 1536;

    std::filesystem::path  rawFilePath = filename.toStdString();

    // 检查文件是否存在
    if (!std::filesystem::exists(rawFilePath))
    {
        qDebug() << "Error: input_image.raw does not exist";
    }

    // 打开RAW图像文件并读取图像数据
    std::ifstream rawFile(rawFilePath, std::ios::binary);
    if (!rawFile.is_open())
    {
        qDebug() << "Error: Unable to open input_image.raw";
    }

    // 创建缓冲区以保存图像数据
    std::vector<unsigned short> buffer(width * height);

    // 从文件中读取图像数据
    rawFile.read(reinterpret_cast<char*>(buffer.data()), buffer.size()* sizeof(unsigned short));
    rawFile.close();

    // 将RAW图像数据转换为OpenCV的Mat格式
    cv::Mat image(height, width, CV_16U, buffer.data());

    img = image.clone();
}

void Widget::saveRawPicture( QString filepath )
{
    std::ofstream file(filepath.toStdString(), std::ios::binary);
    if (file.is_open()) {

        file.write((char*)denoisedQImage.bits(), denoisedQImage.bytesPerLine()*denoisedQImage.height());
        file.close();
        std::cout << "did write file: " << filepath.toStdString() << std::endl;
        // return true;
    }
    else {
        std::cerr << "cannot write file" << std::endl;
    }
    // return false;
}
