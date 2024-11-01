#pragma once

#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/portmacro.h"

#undef EPS
#include "opencv2/core.hpp"
#define EPS 192

#include "esp_camera.h"
#include "esp_log.h"

#include "constants.hpp"

namespace ESPCamera {
    /// @brief Configures the ESP-32-CAM.
    void config_cam();

    cv::Mat get_frame(camera_fb_t** fb_p);

    namespace debug {
        void print_matrix(const cv::Mat& mat);
    }
}