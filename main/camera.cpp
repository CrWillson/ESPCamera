#include "camera.hpp"

#include <esp_log.h>
#include "constants.hpp"
#include "esp_camera.h"
#include "sdcard.hpp"

void Camera::config_cam() {        
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
    config.pixel_format = PIXFORMAT_RGB565;  // Set the pixel format

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


esp_err_t Camera::get_frame()
{
    auto pic = esp_camera_fb_get();
    if (!pic) {
        ESP_LOGE(TAG, "Camera capture failed");
        return ESP_FAIL;
    }
    esp_camera_fb_return(pic);
    return ESP_OK;
}


esp_err_t Camera::get_frame(cv::Mat& image) 
{
    // Capture a picture
    auto* fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Camera capture failed");
        return ESP_FAIL;
    }

    // Ensure the format is RGB565
    if (fb->format != PIXFORMAT_RGB565) {
        ESP_LOGE(TAG, "Unsupported format. Expected RGB565.");
        esp_camera_fb_return(fb);
        return ESP_FAIL;
    }

    int width = fb->width;
    int height = fb->height;

    // Create an OpenCV Mat with type CV_8UC2 (8-bit, 2 channels per pixel)
    image.create(height, width, CV_8UC2);

    // Copy the RGB565 data directly into the OpenCV Mat
    memcpy(image.data, fb->buf, fb->len);

    // Release the frame buffer
    esp_camera_fb_return(fb);

    ESP_LOGI(TAG, "Image captured and stored as CV_8UC2 (RGB565)");
    return ESP_OK;
}


esp_err_t Camera::capture_and_save_image() {
    cv::Mat frame = cv::Mat::zeros(96, 96, CV_8UC2);
    get_frame(frame);

    // Get the next available filename
    char filename[32];
    SDCard::get_next_filename(filename);

    // Open file for writing
    FILE *file = fopen(filename, "w");
    if (!file) {
        ESP_LOGE(TAG, "Failed to open file for writing: %s", filename);
        return ESP_FAIL;
    }

    // Write image data to file
    fwrite(frame.data, 1, frame.total() * frame.elemSize(), file);
    
    // // Iterate over each pixel in the matrix
    // for (int i = 0; i < frame.rows; ++i) {
    //     for (int j = 0; j < frame.cols; ++j) {
    //         cv::Vec2b vecpixel = frame.at<cv::Vec2b>(i, j);
    //         uint16_t pixel = (static_cast<uint16_t>(vecpixel[0]) << 8) | vecpixel[1];
    //         fprintf(file, "%d ", pixel);
    //     }
    //     fprintf(file, "\n");
    // }

    // fclose(file);
    ESP_LOGI(TAG, "Image saved as: %s", filename);

    return ESP_OK;
}


esp_err_t Camera::capture_and_save_image_nocv() {
    // Capture a picture
    camera_fb_t *pic = esp_camera_fb_get();
    if (!pic) {
        ESP_LOGE(TAG, "Camera capture failed");
        return ESP_FAIL;
    }

    // Get the next available filename
    char filename[32];
    SDCard::get_next_filename(filename);

    // Open file for writing
    FILE *file = fopen(filename, "wb");
    if (!file) {
        ESP_LOGE(TAG, "Failed to open file for writing: %s", filename);
        esp_camera_fb_return(pic);
        return ESP_FAIL;
    }

    // Write image data to file
    fwrite(pic->buf, 1, pic->len, file);
    fclose(file);
    ESP_LOGI(TAG, "Image saved as: %s", filename);

    // Return the frame buffer back to the driver for reuse
    esp_camera_fb_return(pic);

    return ESP_OK;
}