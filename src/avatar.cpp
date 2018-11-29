#include "string_format.h"
#include <chrono>
#include <gtest/gtest.h>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/opencv.hpp>

namespace avatar
{

const int PORTRAIT_CROP_PERCENTAGE = 30;

class AvatarGenerator
{
  public:
    AvatarGenerator(cv::Mat image);
    const cv::Mat &transformImage(int size);

  private:
    void crop();
    void circle();
    void resize(int size);

    cv::Mat mat_;
};

AvatarGenerator::AvatarGenerator(cv::Mat image) : mat_(std::move(image))
{
    if (mat_.channels() == 3)
    {
        mat_.convertTo(mat_, CV_32FC3, 1.0 / 255.0);
    }
    else if (mat_.channels() == 4)
    {
        mat_.convertTo(mat_, CV_32FC4, 1.0 / 255.0);
    }
    else
    {
        CV_Assert(!"Unexpected channels");
    }
}

void AvatarGenerator::crop()
{
    int top = 0;
    int left = 0;
    int height = mat_.rows;
    int width = mat_.cols;

    if (height > width) // portrait
    {
        // take portraitCropPercentage (30%) off the top and the rest off the bottom
        int difference = height - width;
        int topOff = (difference * PORTRAIT_CROP_PERCENTAGE) / 100;
        top += topOff;
        height = width;
    }
    else if (height < width) // landscape
    {
        int difference = width - height;
        left += difference / 2;
        width = height;
    }

    mat_ = mat_(cv::Rect(left, top, width, height));
}

// Another implementation of circle
// void AvatarGenerator::circle()
// {
//     CV_Assert(mat_.rows == mat_.cols);
//     if (mat_.rows != mat_.cols)
//     {
//         return;
//     }

//     cv::Mat mask(mat_.size(), CV_32FC1, cv::Scalar::all(0.0f));
//     int radius = mask.rows / 2;
//     cv::circle(mask, cv::Point(radius, radius), radius - 2, cv::Scalar::all(1.0f), -1, CV_AA);
//     cv::threshold(1.0 - mask, mask, 0.9, 1.0, cv::THRESH_BINARY_INV);
//     cv::boxFilter(mask, mask, 5, cv::Size(5, 5), cv::Point(-1, -1), true, cv::BORDER_CONSTANT);

//     cv::Mat alphaChannel(mask.rows, mask.cols, CV_32FC1, cv::Scalar(1.0f));
//     if (mat_.channels() == 4)
//     {
//         std::vector<cv::Mat> channels;
//         cv::split(mat_, channels);
//         channels[3].convertTo(alphaChannel, CV_32FC1);
//         channels.pop_back();
//         cv::merge(channels, mat_);
//     }

//     // Create a white image
//     cv::Mat result(mask.size(), CV_32FC4, cv::Scalar(1.0f, 1.0f, 1.0f));
//     using Pixel = cv::Point3_<float>;
//     mat_.forEach<Pixel>([&mask, &result, &alphaChannel](Pixel &pixel, const int *po) {
//         float alpha = mask.at<float>(po[0], po[1]);
//         float belta = alphaChannel.at<float>(po[0], po[1]);

//         result.at<cv::Vec4f>(po[0], po[1]) = {pixel.x * alpha + 1.0f - alpha,
//                                               pixel.y * alpha + 1.0f - alpha,
//                                               pixel.z * alpha + 1.0f - alpha, alpha * belta};
//     });

//     mat_ = result;
// }

void AvatarGenerator::circle()
{
    CV_Assert(mat_.rows == mat_.cols);
    if (mat_.rows != mat_.cols)
    {
        return;
    }

    cv::Mat mask(mat_.size(), CV_32FC1, cv::Scalar::all(0.0f));
    int radius = mask.rows / 2;
    cv::circle(mask, cv::Point(radius, radius), radius - 2, cv::Scalar::all(1.0f), -1, CV_AA);
    cv::threshold(1.0 - mask, mask, 0.9, 1.0, cv::THRESH_BINARY_INV);
    cv::boxFilter(mask, mask, 5, cv::Size(5, 5), cv::Point(-1, -1), true, cv::BORDER_CONSTANT);

    cv::Mat alphaChannel(mask.rows, mask.cols, CV_32FC1, cv::Scalar(1.0f));
    if (mat_.channels() == 4)
    {
        std::vector<cv::Mat> channels;
        cv::split(mat_, channels);
        channels[3].convertTo(alphaChannel, CV_32FC1);
        channels.pop_back();
        cv::merge(channels, mat_);
    }

    using Pixel = cv::Point3_<float>;
    mat_.forEach<Pixel>([&mask, &alphaChannel](Pixel &pixel, const int *po) {
        float &alpha = mask.at<float>(po[0], po[1]);
        float belta = alphaChannel.at<float>(po[0], po[1]);
        pixel.x = pixel.x * alpha + 1.0f - alpha;
        pixel.y = pixel.y * alpha + 1.0f - alpha;
        pixel.z = pixel.z * alpha + 1.0f - alpha;
        alpha = alpha * belta;
    });

    {
        std::vector<cv::Mat> matChannels;
        cv::split(mat_, matChannels);
        matChannels.emplace_back(mask);
        cv::merge(matChannels, mat_);
    }
}

void AvatarGenerator::resize(int size)
{
    cv::resize(mat_, mat_, cv::Size(size, size), 0, 0,
               mat_.rows < size * 2 ? cv::INTER_LINEAR : cv::INTER_AREA);
}

const cv::Mat &AvatarGenerator::transformImage(int size)
{
    crop();
    if (mat_.rows > 256)
    {
        resize(256);
    }

    circle();
    resize(size);

    mat_.convertTo(mat_, CV_8UC4, 255);
    return mat_;
}

TEST(avatar, lena)
{
    cv::Mat image = cv::imread("./data/Lena.jpg");
    auto start = std::chrono::high_resolution_clock::now();
    AvatarGenerator ag(image);
    const cv::Mat &result = ag.transformImage(64);
    auto end = std::chrono::high_resolution_clock::now();
    auto delta = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    std::cout << "seconds = " << delta.count() << '\n';
    imwrite("avatar_lena.png", result);
    cv::imshow("original", image);
    cv::imshow("avatar_lena", result);
    cv::waitKey(0);
}

TEST(avatar, lena_gray)
{
    cv::Mat image = cv::imread("./data/Lena-gray.png");
    auto start = std::chrono::high_resolution_clock::now();
    AvatarGenerator ag(image);
    const cv::Mat &result = ag.transformImage(64);
    auto end = std::chrono::high_resolution_clock::now();
    auto delta = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    std::cout << "seconds = " << delta.count() << '\n';
    imwrite("avatar_lena_gray.png", result);
    cv::imshow("original", image);
    cv::imshow("avatar_lena_gray", result);
    cv::waitKey(0);
}

TEST(avatar, mario)
{
    cv::Mat image = cv::imread("./data/Small-mario.png", cv::IMREAD_UNCHANGED);
    auto start = std::chrono::high_resolution_clock::now();
    AvatarGenerator ag(image);
    const cv::Mat &result = ag.transformImage(64);
    auto end = std::chrono::high_resolution_clock::now();
    auto delta = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    std::cout << "seconds = " << delta.count() << '\n';
    imwrite("avatar_mario.png", result);
    cv::imshow("original", image);
    cv::imshow("avatar_mario", result);
    cv::waitKey(0);
}

TEST(avatar, performance)
{
#define N 9

    std::array<cv::Mat, N> mats;
    for (int i = 0; i < mats.size(); ++i)
    {
        std::string path = string_format("./data/performance/p%d.png", i);
        mats[i] = cv::imread(path, cv::IMREAD_UNCHANGED);
    }

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<cv::Mat> results;
    results.reserve(N * 3);
    for (auto &image : mats)
    {
        {
            AvatarGenerator ag(image);
            results.emplace_back(ag.transformImage(32));
        }
        {
            AvatarGenerator ag(image);
            results.emplace_back(ag.transformImage(64));
        }
        {
            AvatarGenerator ag(image);
            results.emplace_back(ag.transformImage(128));
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto delta = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    std::cout << "take seconds: " << delta.count() << '\n';

    for (int i = 0; i < results.size(); ++i)
    {
        std::string path = string_format("%d-%d.png", i / 3, results[i].rows);
        imwrite(path, results[i]);
    }
}

///////////////////////////////////////////////////////////////////////////////
// No. 2
// Title: Filling holes in an image using OpenCV
// Web: https://www.learnopencv.com/filling-holes-in-an-image-using-opencv-python-c
///////////////////////////////////////////////////////////////////////////////

TEST(avatar, nickel)
{
    cv::Mat im_in = cv::imread("./data/nickel.jpg", cv::IMREAD_GRAYSCALE);

    // Threshold.
    // Set values equal to or above 220 to 0.
    // Set values below 220 to 255.
    cv::Mat im_th;
    threshold(im_in, im_th, 220, 255, cv::THRESH_BINARY_INV);
     
    // Floodfill from point (0, 0)
    cv::Mat im_floodfill = im_th.clone();
    floodFill(im_floodfill, cv::Point(0,0), cv::Scalar(255));
     
    // Invert floodfilled image
    cv::Mat im_floodfill_inv;
    bitwise_not(im_floodfill, im_floodfill_inv);
     
    // Combine the two images to get the foreground.
    cv::Mat im_out = (im_th | im_floodfill_inv);
 
    // Display images
    imshow("Original Image", im_in);
    imshow("Thresholded Image", im_th);
    imshow("Floodfilled Image", im_floodfill);
    imshow("Inverted Floodfilled Image", im_floodfill_inv);
    imshow("Foreground", im_out);
    cv::waitKey(0);
}

} // namespace avatar
