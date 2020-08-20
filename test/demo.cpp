/*
 * @Author: Lei Jiang
 * @Email: leijiang420@163.com
 * @Date: 2020-07-23 15:28:51
 * @Description: code description
 */
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "../src/core/common.hpp"
#include <string>
#include <iostream>

using namespace std;
using namespace cv;
using namespace cv::dnn;

Mat preprocess(Mat input_image, int net_w, int net_h)
{
    Mat resize_mat(net_h, net_w, CV_8UC3);
    int image_w = input_image.cols;
    int image_h = input_image.rows;
    //judge size
    int top, bottom, left, right;
    if (image_w > image_h)
    {
        float scale = image_w * 1.0 / net_w;
        int new_image_h = int(image_h / scale);
        int half_border = (net_h - new_image_h) / 2;
        top = half_border;
        bottom = net_h - new_image_h - half_border;
        left = 0;
        right = 0;
        resize(input_image, input_image, Size(net_w, new_image_h));
    }
    else
    {
        float scale = image_h * 1.0 / net_h;
        int new_image_w = int(image_w / scale);
        int half_border = (net_w - new_image_w) / 2;
        top = 0;
        bottom = 0;
        left = half_border;
        right = net_w - new_image_w - half_border;
        resize(input_image, input_image, Size(new_image_w, net_h));
    }
    copyMakeBorder(input_image, resize_mat, top, bottom, left, right, BORDER_CONSTANT, Scalar(114, 114, 114));
    return resize_mat;
}

void PostProcess(Rect &box, int image_w, int image_h, int net_w, int net_h)
{
    if (image_w > image_h)
    {
        float scale = image_w * 1.0 / net_w;
        int new_image_h = int(image_h / scale);
        int half_border = (net_h - new_image_h) / 2;
        box.y = box.y - half_border;
        box.y = int(box.y * scale);
        box.x = int(box.x * scale);
        box.width = int(box.width * scale);
        box.height = int(box.height * scale);
    }
    else
    {
        float scale = image_h * 1.0 / net_h;
        int new_image_w = int(image_w / scale);
        int half_border = (net_w - new_image_w) / 2;
        box.x = box.x - half_border;
        box.y = int(box.y * scale);
        box.x = int(box.x * scale);
        box.width = int(box.width * scale);
        box.height = int(box.height * scale);
    }
}

int main(int argc, char **argv)
{
    string weight_path = "models/yolov4.weights";
    string cfg_path = "models/yolov4.cfg";

    Net yolo_net = readNetFromDarknet(cfg_path, weight_path);
    yolo_net.setPreferableBackend(DNN_BACKEND_CUDA); //set backend cuda
    yolo_net.setPreferableTarget(DNN_TARGET_CUDA);   //set device gpu
    int letter_box = 1;

    std::vector<string> outNames = yolo_net.getUnconnectedOutLayersNames(); //get output layers

    Mat inputBlob;
    Mat frame = imread("images/1.jpg");
    Mat letter_frame;
    int img_h = frame.rows;
    int img_w = frame.cols;
    int net_w = 608;
    int net_h = 608;
    if (letter_box)
    {
        letter_frame = preprocess(frame, net_w, net_h);
        inputBlob = blobFromImage(letter_frame, 1 / 255.F, Size(net_w, net_h), Scalar(), true, false);
    }
    else
    {
        inputBlob = blobFromImage(frame, 1 / 255.F, Size(net_w, net_h), Scalar(), true, false);
    }

    yolo_net.setInput(inputBlob);
    std::vector<Mat> out_result;
    cout << "begin do inference " << endl;
    yolo_net.forward(out_result, outNames);
    cout << "do inference ok" << endl;
    cout << "output result size is " << out_result.size() << endl;

    //post process
    vector<Rect> boxes;
    vector<int> classIds;
    vector<float> confidences;

    for (size_t i = 0; i < out_result.size(); ++i)
    {
        // detected objects and C is a number of classes + 4 where the first 4
        float *data = (float *)out_result[i].data;

        for (int j = 0; j < out_result[i].rows; ++j, data += out_result[i].cols)
        {
            Mat scores = out_result[i].row(j).colRange(5, out_result[i].cols);
            Point classIdPoint;
            double confidence;
            minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
            if (confidence > 0.4)
            {
                if (letter_box == 0)
                {
                    int centerX = (int)(data[0] * frame.cols);
                    int centerY = (int)(data[1] * frame.rows);
                    int width = (int)(data[2] * frame.cols);
                    int height = (int)(data[3] * frame.rows);
                    int left = centerX - width / 2;
                    int top = centerY - height / 2;

                    classIds.push_back(classIdPoint.x);
                    confidences.push_back((float)confidence);
                    boxes.push_back(Rect(left, top, width, height));
                }
                else
                {

                    int centerX = (int)(data[0] * letter_frame.cols);
                    int centerY = (int)(data[1] * letter_frame.rows);
                    int width = (int)(data[2] * letter_frame.cols);
                    int height = (int)(data[3] * letter_frame.rows);
                    int left = centerX - width / 2;
                    int top = centerY - height / 2;

                    classIds.push_back(classIdPoint.x);
                    confidences.push_back((float)confidence);
                    boxes.push_back(Rect(left, top, width, height));
                }
            }
        }
    }

    cout << "boxes size is " << boxes.size() << endl;
    vector<int> indices;
    NMSBoxes(boxes, confidences, 0.2, 0.4, indices); // do nms process
    cout << "indices size is :" << indices.size() << endl;
    for (size_t i = 0; i < indices.size(); ++i)
    {
        Rect box = boxes[indices[i]];
        PostProcess(box, img_w, img_h, net_w, net_h);
        rectangle(frame, box, Scalar(0, 0, 255), 2, 8, 0);
    }
    imshow("YOLOv4-Detections", frame);
    waitKey(0);
    return 0;
}