#ifndef INCLUDED_REALSENSE_FORMATTER_HPP
#define INCLUDED_REALSENSE_FORMATTER_HPP

#include <pxcsensemanager.h>
#include <ostream>

std::ostream & operator<<(std::ostream &os, PXCImage::PixelFormat fmt);
std::ostream & operator<<(std::ostream &os, pxcStatus status);

#endif  // INCLUDED_REALSENSE_FORMATTER_HPP
