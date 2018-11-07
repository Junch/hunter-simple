#include <chrono>
#include <gtest/gtest.h>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/opencv.hpp>

namespace avatar
{

const int PORTRAIT_CROP_PERCENTAGE = 30;

cv::Mat &crop(cv::Mat &image)
{
    int top = 0;
    int left = 0;
    int height = image.rows;
    int width = image.cols;

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

    image = image(cv::Rect(left, top, width, height));
    return image;
}

cv::Mat circle(cv::Mat &image)
{
    CV_Assert(image.rows == image.cols);

    cv::Mat mask(image.size(), CV_32FC1, cv::Scalar::all(0.0f));
    int radius = image.rows / 2;
    cv::circle(mask, cv::Point(radius, radius), radius - 2, cv::Scalar::all(1.0f), -1, CV_AA);
    cv::threshold(1.0 - mask, mask, 0.9, 1.0, cv::THRESH_BINARY_INV);
    cv::boxFilter(mask, mask, 5, cv::Size(5, 5), cv::Point(-1, -1), true, cv::BORDER_CONSTANT);

    cv::Mat alphaChannel(image.rows, image.cols, CV_32FC1, cv::Scalar(1.0f));
    if (image.channels() == 4)
    {
        std::vector<cv::Mat> channels;
        cv::split(image, channels);
        channels[3].convertTo(alphaChannel, CV_32FC1);
        channels.pop_back();
        cv::merge(channels, image);
    }

    // Create a white image
    cv::Mat result(image.size(), CV_32FC4, cv::Scalar(1.0f, 1.0f, 1.0f));
    using Pixel = cv::Point3_<float>;
    image.forEach<Pixel>([&mask, &result, &alphaChannel](Pixel &pixel, const int *po) {
        float alpha = mask.at<float>(po[0], po[1]);
        float belta = alphaChannel.at<float>(po[0], po[1]);
        if (alpha != 0.0f && belta != 0.0f)
        {
            result.at<cv::Vec4f>(po[0], po[1]) = {
                pixel.x * alpha + 1.0f - alpha, pixel.y * alpha + 1.0f - alpha,
                pixel.z * alpha + 1.0f - alpha, belta * alpha + 1.0f - alpha};
        }
    });

    return result;
}

void resize(cv::Mat &image, int size)
{
    cv::resize(image, image, cv::Size(size, size), 0, 0,
               image.rows < size * 2 ? cv::INTER_LINEAR : cv::INTER_AREA);
}

cv::Mat transformImage(cv::Mat &image, int size)
{
    if (image.channels() == 3)
    {
        image.convertTo(image, CV_32FC3, 1.0 / 255.0);
    }
    else if (image.channels() == 4)
    {
        image.convertTo(image, CV_32FC4, 1.0 / 255.0);
    }
    else
    {
        CV_Assert(!"Unexpected channels");
    }

    crop(image);
    if (image.rows > 128)
    {
        resize(image, 128);
    }
    auto start = std::chrono::high_resolution_clock::now();
    cv::Mat r = circle(image);
    auto end = std::chrono::high_resolution_clock::now();
    auto delta = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    std::cout << "circle seconds = " << delta.count() << '\n';
    resize(r, size);
    r.convertTo(r, CV_8UC4, 255);
    return r;
}

TEST(avatar, lena)
{
    cv::Mat image = cv::imread("./data/Lena.jpg");
    auto start = std::chrono::high_resolution_clock::now();
    cv::Mat result = transformImage(image, 64);
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
    cv::Mat result = transformImage(image, 64);
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
    cv::Mat image = cv::imread("./data/Small-mario.png", CV_LOAD_IMAGE_UNCHANGED);
    auto start = std::chrono::high_resolution_clock::now();
    cv::Mat result = transformImage(image, 64);
    auto end = std::chrono::high_resolution_clock::now();
    auto delta = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    std::cout << "seconds = " << delta.count() << '\n';
    imwrite("avatar_mario.png", result);
    cv::imshow("original", image);
    cv::imshow("avatar_mario", result);
    cv::waitKey(0);
}

} // namespace opencv_float