
// Standard C
#include <string.h>

// ESP-IDF
#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_netif_ip_addr.h"

// Project headers
#include "wifi_manager.h"
#include "provisioning_server.h"

// Defines
#define WIFI_NVS_NAMESPACE "wifi_cfg"
#define WIFI_NVS_KEY_SSID "ssid"
#define WIFI_NVS_KEY_PASS "pass"

// Static/global variables
static char s_wifi_ssid[32] = {0};
static char s_wifi_pass[64] = {0};

// Forward declaration for static function
static esp_err_t wifi_manager_load_credentials_from_nvs(char *ssid, size_t ssid_len, char *password, size_t pass_len);


// Public function implementations
void wifi_manager_load_credentials(void) {
    char nvs_ssid[64] = {0};
    char nvs_pass[64] = {0};
    esp_err_t nvs_err = wifi_manager_load_credentials_from_nvs(nvs_ssid, sizeof(nvs_ssid), nvs_pass, sizeof(nvs_pass));
    if (nvs_err == ESP_OK) {
        strncpy(s_wifi_ssid, nvs_ssid, sizeof(s_wifi_ssid) - 1);
        s_wifi_ssid[sizeof(s_wifi_ssid) - 1] = '\0';
        strncpy(s_wifi_pass, nvs_pass, sizeof(s_wifi_pass) - 1);
        s_wifi_pass[sizeof(s_wifi_pass) - 1] = '\0';
    }
}

// Private function implementations
static esp_err_t wifi_manager_save_credentials_to_nvs(const char *ssid, const char *password) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(WIFI_NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) return err;
    err = nvs_set_str(nvs_handle, WIFI_NVS_KEY_SSID, ssid);
    if (err == ESP_OK) {
        err = nvs_set_str(nvs_handle, WIFI_NVS_KEY_PASS, password);
    }
    if (err == ESP_OK) {
        err = nvs_commit(nvs_handle);
    }
    nvs_close(nvs_handle);
    return err;
}

static esp_err_t wifi_manager_load_credentials_from_nvs(char *ssid, size_t ssid_len, char *password, size_t pass_len) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(WIFI_NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) return err;
    err = nvs_get_str(nvs_handle, WIFI_NVS_KEY_SSID, ssid, &ssid_len);
    if (err == ESP_OK) {
        err = nvs_get_str(nvs_handle, WIFI_NVS_KEY_PASS, password, &pass_len);
    }
    nvs_close(nvs_handle);
    return err;
}

void wifi_manager_set_credentials(const char *ssid, const char *password) {
    strncpy(s_wifi_ssid, ssid, sizeof(s_wifi_ssid) - 1);
    s_wifi_ssid[sizeof(s_wifi_ssid) - 1] = '\0';
    strncpy(s_wifi_pass, password, sizeof(s_wifi_pass) - 1);
    s_wifi_pass[sizeof(s_wifi_pass) - 1] = '\0';
    wifi_manager_save_credentials_to_nvs(ssid, password);
}

void wifi_manager_start_provisioning(void) {
    // Start WiFi AP and provisioning web server
    provisioning_server_start();
}

bool wifi_manager_is_provisioned(void) {
    char nvs_ssid[64] = {0};
    char nvs_pass[64] = {0};
    esp_err_t nvs_err = wifi_manager_load_credentials_from_nvs(nvs_ssid, sizeof(nvs_ssid), nvs_pass, sizeof(nvs_pass));
    return (nvs_err == ESP_OK && strlen(nvs_ssid) > 0);
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW("wifi_manager", "Disconnected. Retrying...");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        char ipbuf[16];
    ESP_LOGI("wifi_manager", "Got IP: %s", esp_ip4addr_ntoa(&event->ip_info.ip, ipbuf, sizeof(ipbuf)));
    ESP_LOGI("wifi_manager", "Calling mqtt_manager_connect() after IP_EVENT_STA_GOT_IP");
    extern void mqtt_manager_connect(void);
    mqtt_manager_connect();
    }
}

void wifi_manager_connect(void) {
    ESP_LOGI("wifi_manager", "Connecting to WiFi: SSID='%s', PASS='%s'", s_wifi_ssid, s_wifi_pass);

    // Create default netif if not already done (safe to call multiple times)
    static esp_netif_t *sta_netif = NULL;
    if (!sta_netif) {
        sta_netif = esp_netif_create_default_wifi_sta();
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

    wifi_config_t wifi_config = {0};
    strncpy((char*)wifi_config.sta.ssid, s_wifi_ssid, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char*)wifi_config.sta.password, s_wifi_pass, sizeof(wifi_config.sta.password) - 1);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI("wifi_manager", "WiFi STA started, waiting for connection...");
}
