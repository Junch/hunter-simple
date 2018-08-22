#include <gtest/gtest.h>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/opencv.hpp>

using namespace cv;

TEST(opencv, load_image)
{
    // https://www.opencv-srf.com/2017/11/load-and-display-image.html
    Mat image = imread("./data/Einstein.jpg");

    // Check for failure
    if (image.empty())
    {
        std::cout << "Could not open or find the image" << std::endl;
        std::cin.get(); // wait for any key press
        return;
    }

    String windowName = "Einstein"; // Name of the window
    namedWindow(windowName);        // Create a window
    imshow(windowName, image);      // Show our image inside the created window.
    waitKey(0);                     // Wait for any keystroke in the window
    destroyWindow(windowName);      // destroy the created window
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
    Mat image(600, 800, CV_8UC3, Scalar(100, 250, 30));

    String windowName = "Window with Blank Image"; // Name of the window
    namedWindow(windowName);                       // Create a window
    imshow(windowName, image);                     // Show our image inside the created window.
    waitKey(0);                                    // Wait for any keystroke in the window
    destroyWindow(windowName);                     // destroy the created window
}

TEST(opencv, gaussian_blur)
{
    // Read the image file
    Mat image = imread("./data/Einstein.jpg");

    // Check for failure
    if (image.empty())
    {
        std::cout << "Could not open or find the image" << std::endl;
        std::cin.get(); // wait for any key press
        return;
    }

    // Blur the image with 3x3 Gaussian kernel
    Mat image_blurred_with_3x3_kernel;
    GaussianBlur(image, image_blurred_with_3x3_kernel, Size(3, 3), 0);

    // Blur the image with 5x5 Gaussian kernel
    Mat image_blurred_with_5x5_kernel;
    GaussianBlur(image, image_blurred_with_5x5_kernel, Size(5, 5), 0);

    // Define names of the windows
    String window_name = "Einstein";
    String window_name_blurred_with_3x3_kernel = "Lotus Blurred with 3 x 3 Gaussian Kernel";
    String window_name_blurred_with_5x5_kernel = "Lotus Blurred with 5 x 5 Gaussian Kernel";

    // Create windows with above names
    namedWindow(window_name);
    namedWindow(window_name_blurred_with_3x3_kernel);
    namedWindow(window_name_blurred_with_5x5_kernel);

    // Show our images inside the created windows.
    imshow(window_name, image);
    imshow(window_name_blurred_with_3x3_kernel, image_blurred_with_3x3_kernel);
    imshow(window_name_blurred_with_5x5_kernel, image_blurred_with_5x5_kernel);
    waitKey(0);          // Wait for any keystroke in the window
    destroyAllWindows(); // destroy all opened windows
}

TEST(opencv, gray_image)
{
    // http://www.bogotobogo.com/OpenCV/opencv_3_tutorial_load_convert_save_image.php
    // Read the image file
    Mat image = imread("./data/Dostoevsky.jpg");

    // Check for failure
    if (image.empty())
    {
        std::cout << "Could not open or find the image" << std::endl;
        std::cin.get(); // wait for any key press
        return;
    }

    // create a MAT object for gray image
    Mat gray_image;

    // convert to Greyscale format
    // cvtColor( image, gray_image, CV_BGR2GRAY );
    cvtColor( image, gray_image, COLOR_BGR2GRAY );

    // save the transformed image to a file
    imwrite( "../images/GrayImage.jpg", gray_image );

    // creates two windows
    namedWindow( "Dostoevsky", CV_WINDOW_AUTOSIZE );
    namedWindow( "Gray image", CV_WINDOW_AUTOSIZE );

    // imshow() displays an image in the specified window. 
    // If the window was created with the CV_WINDOW_AUTOSIZE flag, 
    // the image is shown with its original size
    imshow( "Dostoevsky", image );
    imshow( "Gray image", gray_image );

    // wait for key press
    waitKey(0);
}