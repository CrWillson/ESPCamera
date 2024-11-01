// Stdlib imports imports
#include <stdio.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <iostream>
#include <algorithm>


// Esp imports
#include <esp_err.h>
#include <esp_spiffs.h>
#include <esp_log.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_sleep.h"

// SD Card Imports
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"

// FreeRTOS imports

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/portmacro.h"


// In-project imports
#include "camera.hpp"


// This is necessary because it allows ESP-IDF to find the main function,
// even though C++ mangles the function name.
extern "C" {
void app_main(void);
}


// Global variables
camera_fb_t* fb = nullptr;


// Function to initialize the SD card
esp_err_t mount_sd_card() {
    static const char *TAG = "SD_Mount";

    // SD card configuration
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    
    // GPIO configuration specific to ESP32-CAM
    slot_config.width = 1;           // 1-bit mode for ESP32-CAM
    gpio_set_pull_mode(GPIO_NUM_15, GPIO_PULLUP_ONLY);   // CMD
    gpio_set_pull_mode(GPIO_NUM_2, GPIO_PULLUP_ONLY);    // D0
    gpio_set_pull_mode(GPIO_NUM_4, GPIO_PULLUP_ONLY);    // D1
    gpio_set_pull_mode(GPIO_NUM_12, GPIO_PULLUP_ONLY);   // D2
    gpio_set_pull_mode(GPIO_NUM_13, GPIO_PULLUP_ONLY);   // D3

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    sdmmc_card_t *card;
    esp_err_t ret = esp_vfs_fat_sdmmc_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                     "If you want the card to be formatted, set format_if_mount_failed = true.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return ret;
    }

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);
    ESP_LOGI(TAG, "SD card mounted successfully.");
    return ESP_OK;
}


esp_err_t write_str_to_sd(std::string data_str, std::string file_name) {
    static const char *TAG = "SD_Write";

    // Construct the file path
    std::string file_path = MOUNT_POINT;
    file_path += file_name;

    // Open file for writing
    FILE *file = fopen(file_path.c_str(), "w");
    if (file == nullptr) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }

    // Write data string to file
    if (fprintf(file, "%s", data_str.c_str()) < 0) {
        ESP_LOGE(TAG, "Failed to write to file");
        fclose(file);
        return ESP_FAIL;
    }

    // Close the file and return success
    fclose(file);
    ESP_LOGI(TAG, "File written successfully");
    return ESP_OK;
}

void capture_and_save_image(std::string file_name) {
    static const char *TAG = "Camera_Sample";

    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Camera capture failed");
        return;
    }

    // Construct the file path
    std::string file_path = MOUNT_POINT;
    file_path += file_name;

    // Open file for writing
    FILE *file = fopen("/sdcard/image1.bin", "wb");
    if (file == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        esp_camera_fb_return(fb);
        return;
    }

    // Write image data to file
    fwrite(fb->buf, 1, fb->len, file);
    fclose(file);
    ESP_LOGI(TAG, "Image saved");

    // Return the frame buffer
    esp_camera_fb_return(fb);
}


/// @brief The entry-point.
void app_main(void)
{
    ESPCamera::config_cam();

    if (mount_sd_card() == ESP_OK) {
        capture_and_save_image("/imagecolor.bin");
    } else {
        static const char *TAG = "SD_Write";
        ESP_LOGE(TAG, "Failed to mount SD card");
    }
    
    esp_deep_sleep_start();
}
