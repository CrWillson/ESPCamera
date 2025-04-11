#pragma once

#include "opencv2.hpp"
#include <esp_err.h>
#include "esp_camera.h"

/**
 * @brief Functions all related to the camera
 * 
 */
namespace Camera {

    /// @brief Tag used in ESP debug logs
    static const char* TAG = "CAMERA";

    /**
     * @brief Configure the camera
     * 
     */
    void config_cam();

    /**
     * @brief Get a frame from the camera and immediately throw it away
     *
     * @overload 
     * @return esp_err_t - ESP_OK if frame was successfully taken
     */
    esp_err_t get_frame();

    /**
     * @brief Get a frame and store it in an opencv matrix
     *
     * @overload
     * @param image - The matrix to store the frame in
     * @return esp_err_t - ESP_OK if the image was successfully stored in the matrix
     */
    esp_err_t get_frame(cv::Mat& image);

    /**
     * @brief Capture and save an opencv matrix image to the sd card
     * 
     * @return esp_err_t - ESP_OK if the image was successfully saved
     */
    esp_err_t capture_and_save_image();

    /**
     * @brief Capture and save a raw image to the sd card without OpenCV
     * 
     * @return esp_err_t - ESP_OK if the image was successfully saved
     */
    esp_err_t capture_and_save_image_nocv();
}