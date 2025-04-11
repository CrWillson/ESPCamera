#include "sdcard.hpp"

#include <exception>
#include "constants.hpp"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "ff.h"
#include <dirent.h>
#include <esp_spiffs.h>
#include <esp_log.h>
#include "sdkconfig.h"

esp_err_t SDCard::mount_sd_card() 
{
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


esp_err_t SDCard::unmount_sd_card()
{
    try {
        esp_vfs_fat_sdmmc_unmount();
        ESP_LOGI(TAG, "Unmounted SD Card");
        return ESP_OK;
    } catch (...) {
        ESP_LOGE(TAG, "Failed to unmount SD Card");
        return ESP_FAIL;
    }
}


// Function to find the next available image filename
void SDCard::get_next_filename(char *filename) {
    int file_number = 0;
    FILE* config_file = fopen(CONFIG_FILE, "r");

    if (config_file) {
        if (fscanf(config_file, "%d", &file_number) != 1) {
            ESP_LOGE(TAG, "Failed to read file number from config file");
        }
        fclose(config_file);
    } else {
        ESP_LOGI(TAG, "Config file not found, starting from 0");
    }

    sprintf(filename, MOUNT_POINT "/" FILE_PREFIX "%d" FILE_EXTENSION, file_number);
    ESP_LOGI(TAG, "Next filename: %s", filename);

    config_file = fopen(CONFIG_FILE, "w");
    if (config_file) {
        fprintf(config_file, "%d", file_number + 1);
        fclose(config_file);
    } else {
        ESP_LOGE(TAG, "Failed to open config file for writing");
    }

}