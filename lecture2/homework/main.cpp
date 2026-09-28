#include <opencv2/opencv.hpp>

#include "io/camera.hpp"
#include "tasks/yolo.hpp"
#include "tools/img_tools.hpp"

int main()
{
  io::Camera camera;
  auto_aim::YOLO detector("configs/yolo.yaml", false);

  cv::Mat img;
  while (camera.read(img)) {
    for (const auto & armor : detector.detect(img)) {
      tools::draw_points(img, armor.points, cv::Scalar(0, 255, 0), 2);

      const auto label = auto_aim::COLORS[armor.color] + auto_aim::ARMOR_NAMES[armor.name];
      const auto bounds = cv::boundingRect(armor.points);
      tools::draw_text(img, label, {bounds.x, bounds.y - 8}, cv::Scalar(0, 0, 255), 0.8, 2);
    }

    cv::imshow("img", img);
    const int key = cv::waitKey(1);
    if (key == 'q' || key == 27) break;
  }

  cv::destroyAllWindows();
  return 0;
}
