#pragma once

#include <esp_err.h>

/**
 * @brief Functions all related to the SD card
 * 
 */
namespace SDCard {

    /// @brief Tag used in ESP debug logs
    static const char *TAG = "SD_Card";

    /**
     * @brief Mount the SD Card
     * 
     * @return esp_err_t - ESP_OK if the SD card was successfully mounted
     */
    esp_err_t mount_sd_card();

    /**
     * @brief Unmount the SD Card
     * 
     * @return esp_err_t - ESP_OK if the SD card was successfully dismounted
     */
    esp_err_t unmount_sd_card();

    /**
     * @brief Get the file name to save the next image as
     * 
     * @param filename - Buffer to store the next file name in
     */
    void get_next_filename(char *filename);
}