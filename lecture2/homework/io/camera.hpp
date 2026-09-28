#ifndef HOMEWORK_SOLUTION_IO_CAMERA_HPP
#define HOMEWORK_SOLUTION_IO_CAMERA_HPP

#include <opencv2/core/mat.hpp>

namespace io
{

class Camera
{
public:
  Camera();
  ~Camera() noexcept;

  bool read(cv::Mat & img);

private:
  Camera(const Camera &) = delete;
  Camera & operator=(const Camera &) = delete;

  void * handle_ = nullptr;
  bool opened_ = false;
  bool grabbing_ = false;
};

}  

#endif  
