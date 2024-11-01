#include "camera.hpp"

/// @brief Configures the ESP-32-CAM.
void ESPCamera::config_cam()
{    
    constexpr char *TAG = "CAM_INIT";
    
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = CAM_PIN_D0;
    config.pin_d1 = CAM_PIN_D1;
    config.pin_d2 = CAM_PIN_D2;
    config.pin_d3 = CAM_PIN_D3;
    config.pin_d4 = CAM_PIN_D4;
    config.pin_d5 = CAM_PIN_D5;
    config.pin_d6 = CAM_PIN_D6;
    config.pin_d7 = CAM_PIN_D7;
    config.pin_xclk = CAM_PIN_XCLK;
    config.pin_pclk = CAM_PIN_PCLK;
    config.pin_vsync = CAM_PIN_VSYNC;
    config.pin_href = CAM_PIN_HREF;
    config.pin_sccb_sda = CAM_PIN_SIOD;
    config.pin_sccb_scl = CAM_PIN_SIOC;
    config.pin_pwdn = CAM_PIN_PWDN;
    config.pin_reset = CAM_PIN_RESET;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_GRAYSCALE;  // Set the pixel format

    // Set frame size to 96x96
    config.frame_size = FRAMESIZE_96X96;
    config.jpeg_quality = 12;  // JPEG quality (lower is better)
    config.fb_count = 1;  // Only one frame buffer

    // Initialize the camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);
        return;
    }

    ESP_LOGI(TAG, "Camera initialized successfully with resolution 96x96");
}


cv::Mat ESPCamera::get_frame(camera_fb_t** fb_p)
{
    // If a previous picture has been taken, give the frame-buffer back.
    if (*fb_p != nullptr)
    {
        esp_camera_fb_return(*fb_p);
    }

    // Take the picture.
    *fb_p = esp_camera_fb_get();
    auto fb = *fb_p;
    if (!fb) {
        ESP_LOGE("TAG", "Camera capture failed");
        return cv::Mat();
    }

    // Build the OpenCV matrix.
    // CV_8UC2 is two-channel color, with 8-bit channels.
    auto result = cv::Mat(fb->height, fb->width, CV_8UC3, fb->buf);

    // Flip the bytes of the matrix so that it can be processed using OpenCV functions.
    /*for (auto row = 0; row < result.rows; row++)
    {
        for (auto col = 0; col < result.cols; col++)
        {
            auto& pixel = result.at<cv::Vec2b>(row, col);
            const auto temp = pixel[0];
            pixel[0] = pixel[1];
            pixel[1] = temp;
        }
    }*/

    return result;
}

void ESPCamera::debug::print_matrix(const cv::Mat& mat) 
{
    #ifdef DEBUG_MODE
    // Begin transmission
    printf("START"); // Start of transmission

    printf("%04x", mat.rows);
    printf("%04x", mat.cols);
    printf("%04x", mat.channels());

    // Transmit what type of frame this is, of the common ones used in this app.
    const auto type = mat.type();

    // RGB585
    if (CV_8UC2 == type)
    {
        printf("CV_8UC2");
    }
    // Binary-mask
    else if (CV_8UC1 == type)
    {
        printf("CV_8UC1");
    }
    else if (CV_8U == type)
    {
        printf("CV_8U__");
    }
    // HSV
    else if (CV_8UC3 == type)
    {
        printf("CV_8UC3");
    }

    // Transmit the data of the frame.
    if (CV_8UC2 == type)
    {
        for (int row = 0; row < mat.rows; row++) 
        {
            for (int col = 0; col < mat.cols; col++)
            {
                const auto& pixel = mat.at<cv::Vec2b>(row, col);

                for (int channel = 0; channel < pixel.channels; channel++)
                {
                    printf("%02x", pixel[channel]);
                }
            }
            vTaskDelay(1); // To avoid the watchdog on especially large matrices.
        }
    }
    else if (CV_8U == type || CV_8UC1 == type)
    {
        for (int row = 0; row < mat.rows; row++) 
        {
            for (int col = 0; col < mat.cols; col++)
            {
                printf("%02x", mat.at<uint8_t>(row, col));
            }
            vTaskDelay(1); // To avoid the watchdog on especially large matrices.
        }
    }
    else if (CV_8UC3 == type)
    {
        for (int row = 0; row < mat.rows; row++)
        {
            for (int col = 0; col < mat.cols; col++)
            {
                const auto pixel = mat.at<cv::Vec3b>(row, col);

                for (int channel = 0; channel < pixel.channels; channel++)
                {
                    printf("%02x", pixel[channel]);
                }
            }
        }
    }

    // End transmission and flush
    printf("E\n");
    fflush(stdout);
    #endif
}