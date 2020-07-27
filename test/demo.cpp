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

int main(int argc, char **argv)
{
    string weight_path = "models/yolov4.weights";
    string cfg_path = "models/yolov4.cfg";

    Net yolo_net = readNetFromDarknet(cfg_path, weight_path);
    yolo_net.setPreferableBackend(DNN_BACKEND_CUDA); //set backend cuda
    yolo_net.setPreferableTarget(DNN_TARGET_CUDA);   //set device gpu

    std::vector<string> outNames = yolo_net.getUnconnectedOutLayersNames(); //get output layers
    Mat frame = imread("images/1.jpg");
    Mat inputBlob = blobFromImage(frame, 1 / 255.F, Size(608, 608), Scalar(), true, false);
    yolo_net.setInput(inputBlob);
    std::vector<Mat> out_result;
    cout << "begin do inference " << endl;
    yolo_net.forward(out_result, outNames);
    cout << "do inference ok" << endl;
    cout << "output result size is "<< out_result.size() << endl;

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
            if (confidence > 0.2)
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
        }
    }

    cout << "boxes size is "<< boxes.size() << endl;
    vector<int> indices;
    NMSBoxes(boxes, confidences, 0.2, 0.4, indices); // do nms process
    cout << "indices size is :"<< indices.size()<<endl;
    for (size_t i = 0; i < indices.size(); ++i)
    {
        Rect box = boxes[indices[i]];
        rectangle(frame, box, Scalar(0, 0, 255), 2, 8, 0);
    }
    imshow("YOLOv4-Detections", frame);
    waitKey(0);
    return 0;
}