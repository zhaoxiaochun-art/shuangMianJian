#pragma once

#pragma region MyRegion

#include "opencv.hpp"
#include "highgui.hpp"
#include <memory.h>
#include "spdlog/spdlog.h"
//#include "QTLiveUSBkey.h"
namespace spd = spdlog;
#ifdef DEBUG
#pragma comment(lib,"opencv_core347d.lib")
#pragma comment(lib,"opencv_highgui347d.lib")
#pragma comment(lib,"opencv_imgcodecs347d.lib")
#pragma comment(lib,"opencv_imgproc347d.lib")
#else
#pragma comment(lib,"opencv_core347.lib")
#pragma comment(lib,"opencv_highgui347.lib")
#pragma comment(lib,"opencv_imgcodecs347.lib")
#pragma comment(lib,"opencv_imgproc347.lib")
#endif
using namespace cv;
#pragma endregion