#include "string_format.h"
#include <chrono>
#include <gtest/gtest.h>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/opencv.hpp>

// https://stackoverflow.com/questions/21795643/image-edge-smoothing-with-opencv

namespace avatar_legacy
{

const int PORTRAIT_CROP_PERCENTAGE = 30;

cv::Mat crop(const cv::Mat &image)
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
        // take 50% off both sides
        int difference = width - height;
        left += difference / 2;
        width = height;
    }
    cv::Mat croppedImage(image, cv::Rect(left, top, width, height));

    return croppedImage;
}

cv::Mat extendToAlphaChannel(const cv::Mat &image, const cv::Mat &alphaChannel)
{
    std::vector<cv::Mat> channels;
    cv::split(image, channels);

    if (channels.size() == 3) // may bot need a judgment, actually all image will have 3
                              // channels after cv::imencode with CV_LOAD_IMAGE_COLOR
    {
        channels.push_back(alphaChannel);
    }

    cv::Mat mergedPng;
    cv::merge(channels, mergedPng);
    return mergedPng;
}

cv::Mat transformImage(const cv::Mat &image, int size)
{
    if (image.empty())
    {
        throw std::runtime_error("image is corrupted");
    }

    cv::Mat whole_image = image.clone();
    if (image.channels() == 3)
    {
        cv::Mat alphaChannel(image.rows, image.cols, CV_8UC1, cv::Scalar(255));
        whole_image = extendToAlphaChannel(image, alphaChannel);
    }

    cv::Mat croppedWholeImage = crop(whole_image);
    croppedWholeImage.convertTo(croppedWholeImage, CV_32FC4, 1.0 / 255.0);

    // black background
    cv::Mat mask(croppedWholeImage.size(), croppedWholeImage.type(), cv::Scalar::all(0));
    // draw a white circle
    int radius = croppedWholeImage.rows / 2;
    cv::circle(mask, cv::Point(radius, radius), radius - 2, cv::Scalar::all(255), -1, CV_AA);
    // cv::treshold expects grayscale and single channel image therefore following conversions
    cv::cvtColor(mask, mask, cv::COLOR_BGR2GRAY);
    mask.convertTo(mask, CV_32FC1);

    /*
    Separate out regions of an image corresponding to objects which we want to analyze.
    To differentiate the pixels we are interested in from the rest (which will eventually be
    rejected), we perform a comparison of each pixel intensity value with respect to a
    threshold. Pixels of interest will have values of 1.0 and the rest will be 0.
    */
    cv::threshold(1.0 - mask, mask, 0.9, 1.0, cv::THRESH_BINARY_INV);
    /*
    The function smoothes the mask using the kernel.
    The parameters where chosen by trial and error method.
    */
    cv::boxFilter(mask, mask, 5, cv::Size(5, 5), cv::Point(-1, -1), true, cv::BORDER_CONSTANT);
    // create background image
    // cv::Mat bg = cv::Mat(croppedWholeImage.size(), CV_32FC4,);
    // bg = cv::Scalar(1.0, 1.0, 1.0);

    cv::Mat bg = cv::Mat(croppedWholeImage.size(), CV_32FC4, cv::Scalar(1.0, 1.0, 1.0));

    std::vector<cv::Mat> ch_img(4);
    std::vector<cv::Mat> ch_bg(4);
    // split image and background image into channels BLUE, GREEN, RED, ALPHA
    cv::split(croppedWholeImage, ch_img);
    cv::split(bg, ch_bg);
    /*
    Apply mask to each channel of an image by element-wise multiplication to extract only pixels
    of interest(inside of the circle). Apply inverted mask (1.0 - mask) to each channel of an
    background image to extract only the area outside of the circle. Add those channels(images)
    together.
    */
    ch_img[0] = ch_img[0].mul(mask) + ch_bg[0].mul(1.0 - mask);
    ch_img[1] = ch_img[1].mul(mask) + ch_bg[1].mul(1.0 - mask);
    ch_img[2] = ch_img[2].mul(mask) + ch_bg[2].mul(1.0 - mask);
    ch_img[3] = ch_img[3].mul(mask) + ch_bg[3].mul(1.0 - mask);
    // merge the channels into resulting image
    // Reget the image fragment with smoothed mask
    cv::Mat res;
    cv::merge(ch_img, res);

    // resize to representation size
    cv::resize(res, res, cv::Size(size, size), 0.0, 0.0,
               croppedWholeImage.rows < size * 2 ? cv::INTER_LINEAR : cv::INTER_AREA);

    // Only images of CV_8U type can be stored on disk therefore conversion
    res.convertTo(res, CV_8UC4, 255);
    return res;
}

TEST(avatar, performance)
{
#define N 9

    std::array<cv::Mat, N> mats;
    for (int i = 0; i < mats.size(); ++i)
    {
        std::string path = string_format("./data/performance/p%d.png", i);
        mats[i] = cv::imread(path, CV_LOAD_IMAGE_UNCHANGED);
    }

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<cv::Mat> results;
    results.reserve(N * 3);
    for (auto &image : mats)
    {
        results.emplace_back(transformImage(image, 32));
        results.emplace_back(transformImage(image, 64));
        results.emplace_back(transformImage(image, 128));
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto delta = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    std::cout << "legacy take seconds: " << delta.count() << '\n';

    for (int i = 0; i < results.size(); ++i)
    {
        std::string path = string_format("%d-%d-legacy.png", i / 3, results[i].rows);
        imwrite(path, results[i]);
    }
}

} // namespace avatar_legacy
