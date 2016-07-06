#include "util/realsense_formatters.h"
#include "util/format.h"
#include <map>

namespace Util {
  namespace RealSense {
    const std::map<PXCImage::PixelFormat, const char *> s_pixel_format
    {
      { PXCImage::PixelFormat::PIXEL_FORMAT_YUY2,             "PIXEL_FORMAT_YUY2" },
      { PXCImage::PixelFormat::PIXEL_FORMAT_NV12,             "PIXEL_FORMAT_NV12" },
      { PXCImage::PixelFormat::PIXEL_FORMAT_RGB32,            "PIXEL_FORMAT_RGB32" },
      { PXCImage::PixelFormat::PIXEL_FORMAT_RGB24,            "PIXEL_FORMAT_RGB24" },
      { PXCImage::PixelFormat::PIXEL_FORMAT_Y8,               "PIXEL_FORMAT_Y8" },
      { PXCImage::PixelFormat::PIXEL_FORMAT_Y8_IR_RELATIVE,   "PIXEL_FORMAT_Y8_IR_RELATIVE" },
      { PXCImage::PixelFormat::PIXEL_FORMAT_Y16,              "PIXEL_FORMAT_Y16" },
      { PXCImage::PixelFormat::PIXEL_FORMAT_DEPTH,            "PIXEL_FORMAT_DEPTH" },
      { PXCImage::PixelFormat::PIXEL_FORMAT_DEPTH_RAW,        "PIXEL_FORMAT_DEPTH_RAW" },
      { PXCImage::PixelFormat::PIXEL_FORMAT_DEPTH_F32,        "PIXEL_FORMAT_DEPTH_F32" },
      { PXCImage::PixelFormat::PIXEL_FORMAT_DEPTH_CONFIDENCE, "PIXEL_FORMAT_DEPTH_CONFIDENCE" }
    };

    static const char * ToString(PXCImage::PixelFormat fmt)
    {
      auto it = s_pixel_format.find(fmt);
      if (it == s_pixel_format.end())
        return "UNKNOWN";
      return it->second;
    }

    const std::map<pxcStatus, const char *> s_status
    {
      { PXC_STATUS_NO_ERROR,          "PXC_STATUS_NO_ERROR" },
      { PXC_STATUS_DEVICE_BUSY,       "PXC_STATUS_DEVICE_BUSY" },
      { PXC_STATUS_PARAM_UNSUPPORTED, "PXC_STATUS_UNSUPPORTED" },
      { PXC_STATUS_ITEM_UNAVAILABLE,  "PXC_STATUS_ITEM_UNAVAILABLE" }
      // Still more to add...
    };

    static const char * ToString(pxcStatus status)
    {
      auto it = s_status.find(status);
      if (it == s_status.end())
        return "UNKNOWN";
      return it->second;
    }
  } // RealSense
} // Util

std::ostream & operator<<(std::ostream &os, PXCImage::PixelFormat fmt)
{
  os << Util::RealSense::ToString(fmt);
  return os;
}

std::ostream & operator<<(std::ostream &os, pxcStatus status)
{
  os << Util::RealSense::ToString(status);
  return os;
}
