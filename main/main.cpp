// SD Card Imports
#include "camera.hpp"
#include "constants.hpp"
#include "opencv2.hpp"
#include "sdcard.hpp"

// Esp imports
#include <esp_err.h>
#include <esp_log.h>

// This is necessary because it allows ESP-IDF to find the main function,
// even though C++ mangles the function name.
extern "C" {
void app_main(void);
}


/// @brief The entry-point.
void app_main(void)
{
    constexpr int THROWAWAY_IMG_COUNT = 10;

    // Configure the camera
    Camera::config_cam();

    if (SDCard::mount_sd_card() == ESP_OK) {
        // Throw away some frames to reduce the yellow tint
        for (int i = 0; i < THROWAWAY_IMG_COUNT; i++) {
            auto res = Camera::get_frame();
            ESP_LOGI(Camera::TAG, "Captured throwaway frame: %d", i);
        }

        // Capture and save a good image
        Camera::capture_and_save_image_nocv();
        ESP_LOGI(Camera::TAG, "Image captured and saved to SD card");

        // Unmount the SD card
        SDCard::unmount_sd_card();
    } else {
        ESP_LOGE(SDCard::TAG, "Failed to mount SD card");
    }

    gpio_set_level(GPIO_NUM_4, 0);
}
