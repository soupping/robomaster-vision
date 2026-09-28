#include "camera.hpp"

#include "../../io/hikrobot/include/MvCameraControl.h"

namespace
{

struct FrameBufferGuard
{
  void * handle_;
  MV_FRAME_OUT * frame_;
  ~FrameBufferGuard() noexcept
  {
    (void)MV_CC_FreeImageBuffer(handle_, frame_);
  }
};

}  
namespace io
{

Camera::Camera()
{
  MV_CC_DEVICE_INFO_LIST devices{};
  if (MV_CC_EnumDevices(MV_USB_DEVICE, &devices) != MV_OK) return;
  if (devices.nDeviceNum == 0 || devices.pDeviceInfo[0] == nullptr) return;

  if (MV_CC_CreateHandle(&handle_, devices.pDeviceInfo[0]) != MV_OK) return;
  if (MV_CC_OpenDevice(handle_) != MV_OK) return;
  opened_ = true;

  (void)MV_CC_SetEnumValue(handle_, "BalanceWhiteAuto", MV_BALANCEWHITE_AUTO_CONTINUOUS);
  (void)MV_CC_SetEnumValue(handle_, "ExposureAuto", MV_EXPOSURE_AUTO_MODE_OFF);
  (void)MV_CC_SetEnumValue(handle_, "GainAuto", MV_GAIN_MODE_OFF);
  (void)MV_CC_SetFloatValue(handle_, "ExposureTime", 10000.0f);
  (void)MV_CC_SetFloatValue(handle_, "Gain", 20.0f);
  (void)MV_CC_SetFrameRate(handle_, 60.0f);

  grabbing_ = MV_CC_StartGrabbing(handle_) == MV_OK;
}

Camera::~Camera() noexcept
{
  if (grabbing_) (void)MV_CC_StopGrabbing(handle_);
  if (opened_) (void)MV_CC_CloseDevice(handle_);
  if (handle_ != nullptr) (void)MV_CC_DestroyHandle(handle_);
}

bool Camera::read(cv::Mat & img)
{
  if (!grabbing_) return false;

  MV_FRAME_OUT frame{};
  if (MV_CC_GetImageBuffer(handle_, &frame, 100) != MV_OK) return false;

  FrameBufferGuard buffer{handle_, &frame};
  const auto & info = frame.stFrameInfo;
  const unsigned int width = info.nExtendWidth != 0 ? info.nExtendWidth : info.nWidth;
  const unsigned int height = info.nExtendHeight != 0 ? info.nExtendHeight : info.nHeight;
  if (frame.pBufAddr == nullptr || width == 0 || height == 0) return false;

  cv::Mat bgr(static_cast<int>(height), static_cast<int>(width), CV_8UC3);
  MV_CC_PIXEL_CONVERT_PARAM_EX conversion{};
  conversion.nWidth = width;
  conversion.nHeight = height;
  conversion.enSrcPixelType = info.enPixelType;
  conversion.pSrcData = frame.pBufAddr;
  conversion.nSrcDataLen = info.nFrameLen;
  conversion.enDstPixelType = PixelType_Gvsp_BGR8_Packed;
  conversion.pDstBuffer = bgr.data;
  conversion.nDstBufferSize = static_cast<unsigned int>(bgr.total() * bgr.elemSize());
  if (MV_CC_ConvertPixelTypeEx(handle_, &conversion) != MV_OK) return false;

  img = bgr;
  return true;
}

}  
