#include "persistent_data_manager.h"
#include <string.h>
#include <stdbool.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

static const char *TAG = "persistent_data_mgr";
static nvs_handle_t s_nvs_handle = 0;

void persistent_data_manager_init(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }
    err = nvs_open("storage", NVS_READWRITE, &s_nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS handle: %s", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "NVS handle opened");
    }
}

bool persistent_data_manager_set(const char *key, const char *value) {
    esp_err_t err = nvs_set_str(s_nvs_handle, key, value);
    if (err == ESP_OK) {
        err = nvs_commit(s_nvs_handle);
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set key '%s': %s", key, esp_err_to_name(err));
        return false;
    }
    ESP_LOGI(TAG, "Set key '%s'", key);
    return true;
}

bool persistent_data_manager_get(const char *key, char *out_value, size_t max_len) {
    size_t required_size = max_len;
    esp_err_t err = nvs_get_str(s_nvs_handle, key, out_value, &required_size);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Key '%s' not found or error: %s", key, esp_err_to_name(err));
        return false;
    }
    ESP_LOGI(TAG, "Loaded key '%s'", key);
    return true;
}

bool persistent_data_manager_remove(const char *key) {
    esp_err_t err = nvs_erase_key(s_nvs_handle, key);
    if (err == ESP_OK) {
        err = nvs_commit(s_nvs_handle);
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to remove key '%s': %s", key, esp_err_to_name(err));
        return false;
    }
    ESP_LOGI(TAG, "Removed key '%s'", key);
    return true;
}
