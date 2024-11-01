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
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_sleep.h"
#include "esp_camera.h"

// SD Card Imports
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "ff.h"
#include <dirent.h>

// FreeRTOS imports
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/portmacro.h"

#include "constants.hpp"


// This is necessary because it allows ESP-IDF to find the main function,
// even though C++ mangles the function name.
extern "C" {
void app_main(void);
}


#define BASE_PATH "/sdcard"
#define FILE_PREFIX "IMAGE"
#define FILE_EXTENSION ".BIN"

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


// Function to find the next available image filename
void get_next_filename(char *filename) {
    static const char *TAG = "CAMERA_SD";

    int max_number = 0;
    DIR *dir = opendir(BASE_PATH);
    if (dir == NULL) {
        ESP_LOGE(TAG, "Failed to open directory: %s", BASE_PATH);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            int num;
            if (sscanf(entry->d_name, FILE_PREFIX "%d" FILE_EXTENSION, &num) == 1) {
                if (num > max_number) {
                    max_number = num;
                }
            }
        }
    }
    closedir(dir);

    // Generate the next filename
    sprintf(filename, BASE_PATH "/" FILE_PREFIX "%d" FILE_EXTENSION, max_number + 1);
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


esp_err_t capture_and_save_image() {
    static const char *TAG = "Camera_Sample";

    // Capture a picture
    camera_fb_t *pic = esp_camera_fb_get();
    if (!pic) {
        ESP_LOGE(TAG, "Camera capture failed");
        return ESP_FAIL;
    }

    // Get the next available filename
    char filename[32];
    get_next_filename(filename);

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


void config_cam()
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

/// @brief The entry-point.
void app_main(void)
{
    static const char *TAG = "CAMERA_SD";
    config_cam();

    if (mount_sd_card() == ESP_OK) {
        capture_and_save_image();

        // Unmount the SD card
        esp_vfs_fat_sdmmc_unmount();
        ESP_LOGI(TAG, "SD card unmounted");
    } else {
        static const char *TAG = "SD_Write";
        ESP_LOGE(TAG, "Failed to mount SD card");
    }

    gpio_set_level(GPIO_NUM_4, 0);
}
