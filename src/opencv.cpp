#include <gtest/gtest.h>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/opencv.hpp>
#include <chrono>

bool matIsEqual(const cv::Mat& mat1, const cv::Mat& mat2){
    // https://stackoverflow.com/questions/9905093/how-to-check-whether-two-matrixes-are-identical-in-opencv

    // treat two empty mat as identical as well
    if (mat1.empty() && mat2.empty()) {
        return true;
    }
    // if dimensionality of two mat is not identical, these two mat is not identical
    if (mat1.cols != mat2.cols || mat1.rows != mat2.rows || mat1.dims != mat2.dims) {
        return false;
    }
    cv::Mat diff;
    cv::compare(mat1, mat2, diff, cv::CMP_NE);
    int nz = cv::countNonZero(diff);
    return nz==0;
}

TEST(opencv, load_image)
{
    // https://www.opencv-srf.com/2017/11/load-and-display-image.html
    cv::Mat image = cv::imread("./data/Einstein.jpg");

    // Check for failure
    if (image.empty())
    {
        std::cout << "Could not open or find the image" << std::endl;
        std::cin.get(); // wait for any key press
        return;
    }

    cv::String windowName = "Einstein"; // Name of the window
    imshow(windowName, image);      // Show our image inside the created window.
    cv::waitKey(0);                     // Wait for any keystroke in the window
    cv::destroyWindow(windowName);      // destroy the created window
}

TEST(opencv, create_blank_image)
{
    // https://www.opencv-srf.com/2017/11/load-and-display-image.html
    // create a new image which consists of
    // 3 channels
    // image depth of 8 bits
    // 800 x 600 of resolution (800 wide and 600 high)
    // each pixels initialized to the value of (100, 250, 30) for Blue, Green and Red planes
    // respectively.
    cv::Mat image(600, 800, CV_8UC3, cv::Scalar(100, 250, 30));

    cv::String windowName = "Window with Blank Image"; // Name of the window
    imshow(windowName, image);                     // Show our image inside the created window.
    cv::waitKey(0);                                    // Wait for any keystroke in the window
    cv::destroyWindow(windowName);                     // destroy the created window
}

TEST(opencv, gaussian_blur)
{
    // Read the image file
    cv::Mat image = cv::imread("./data/Einstein.jpg");

    // Check for failure
    if (image.empty())
    {
        std::cout << "Could not open or find the image" << std::endl;
        std::cin.get(); // wait for any key press
        return;
    }

    // Blur the image with 3x3 Gaussian kernel
    cv::Mat image_blurred_with_3x3_kernel;
    GaussianBlur(image, image_blurred_with_3x3_kernel, cv::Size(3, 3), 0);

    // Blur the image with 5x5 Gaussian kernel
    cv::Mat image_blurred_with_5x5_kernel;
    cv::GaussianBlur(image, image_blurred_with_5x5_kernel, cv::Size(5, 5), 0);

    // Define names of the windows
    cv::String window_name = "Einstein";
    cv::String window_name_blurred_with_3x3_kernel = "Lotus Blurred with 3 x 3 Gaussian Kernel";
    cv::String window_name_blurred_with_5x5_kernel = "Lotus Blurred with 5 x 5 Gaussian Kernel";

    // Show our images inside the created windows.
    imshow(window_name, image);
    imshow(window_name_blurred_with_3x3_kernel, image_blurred_with_3x3_kernel);
    imshow(window_name_blurred_with_5x5_kernel, image_blurred_with_5x5_kernel);
    cv::waitKey(0);          // Wait for any keystroke in the window
    cv::destroyAllWindows(); // destroy all opened windows
}

TEST(opencv, gray_image)
{
    // http://www.bogotobogo.com/OpenCV/opencv_3_tutorial_load_convert_save_image.php
    // Read the image file
    cv::Mat image = cv::imread("./data/Dostoevsky.jpg");

    // Check for failure
    if (image.empty())
    {
        std::cout << "Could not open or find the image" << std::endl;
        std::cin.get(); // wait for any key press
        return;
    }

    // create a MAT object for gray image
    cv::Mat gray_image;

    // convert to Greyscale format
    // cvtColor( image, gray_image, CV_BGR2GRAY );
    cvtColor( image, gray_image, cv::COLOR_BGR2GRAY );

    // save the transformed image to a file
    imwrite( "../images/GrayImage.jpg", gray_image );

    // imshow() displays an image in the specified window. 
    // If the window was created with the CV_WINDOW_AUTOSIZE flag, 
    // the image is shown with its original size
    imshow( "Dostoevsky", image );
    imshow( "Gray image", gray_image );

    // wait for key press
    cv::waitKey(0);
}

///////////////////////////////////////////////////////////////////////////////
// Title: Parallel Pixel Access in OpenCV using forEach
// Web: https://www.learnopencv.com/parallel-pixel-access-in-opencv-using-foreach/
///////////////////////////////////////////////////////////////////////////////
TEST(opencv, forEach_1_channel)
{
    typedef cv::Point3_<uint8_t> Pixel;

    cv::Mat image(3, 2, CV_8UC1, cv::Scalar(0));
    image.forEach<Pixel>([](Pixel &pixel, const int * position){
        printf("[%d, %d] = %d\n", position[0], position[1], (int)pixel.x);
    });
}

TEST(opencv, forEach_4_channel)
{
    using Pixel = cv::Point3_<uint8_t>;

    cv::Mat image(3, 2, CV_8UC4, cv::Scalar(0,0,0,1));
    image.forEach<Pixel>(
        [&image](Pixel &pixel, const int * position){
            const auto& v = image.at<cv::Vec4b>(position[0], position[1]);

            printf("[%d, %d] = %d %d %d %d\n", position[0], position[1], v[0], v[1], v[2], v[3]);
    });
}

///////////////////////////////////////////////////////////////////////////////
// Title: OpenCV Transparent API
// Web: https://www.learnopencv.com/opencv-transparent-api
///////////////////////////////////////////////////////////////////////////////
TEST(opencv, transparent_legacy)
{
    cv::Mat img, gray;
    img = cv::imread("./data/lena.jpg", cv::IMREAD_COLOR);

    auto start = std::chrono::high_resolution_clock::now();
    cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    GaussianBlur(gray, gray, cv::Size(7, 7), 1.5);
    Canny(gray, gray, 0, 50);
    auto end = std::chrono::high_resolution_clock::now();
    auto delta = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    std::cout << "seconds = " << delta.count() << '\n';

    imshow("edges", gray);
    cv::waitKey();
}

TEST(opencv, transparent)
{
    cv::UMat img, gray;
    cv::imread("./data/lena.jpg", cv::IMREAD_COLOR).copyTo(img);

    auto start = std::chrono::high_resolution_clock::now();
    cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    GaussianBlur(gray, gray, cv::Size(7, 7), 1.5);
    Canny(gray, gray, 0, 50);
    auto end = std::chrono::high_resolution_clock::now();
    auto delta = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    std::cout << "seconds = " << delta.count() << '\n';

    imshow("edges", gray);
    cv::waitKey();
}

TEST(opencv, version)
{
    std::cout << "OpenCV version : " << CV_VERSION << '\n';
    std::cout << "Major version : " << CV_MAJOR_VERSION << '\n';
    std::cout << "Minor version : " << CV_MINOR_VERSION << '\n';
    std::cout << "Subminor version : " << CV_SUBMINOR_VERSION << '\n';
}
