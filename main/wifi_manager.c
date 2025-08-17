#include <string.h>
#include "esp_log.h"
// For esp_ip4addr_ntoa
#include "esp_netif_ip_addr.h"
// Store credentials in static variables for now
static char s_wifi_ssid[64] = {0};
static char s_wifi_pass[64] = {0};

void wifi_manager_set_credentials(const char *ssid, const char *password) {
    strncpy(s_wifi_ssid, ssid, sizeof(s_wifi_ssid) - 1);
    s_wifi_ssid[sizeof(s_wifi_ssid) - 1] = '\0';
    strncpy(s_wifi_pass, password, sizeof(s_wifi_pass) - 1);
    s_wifi_pass[sizeof(s_wifi_pass) - 1] = '\0';
}
#include "wifi_manager.h"
#include "provisioning_server.h"

void wifi_manager_start_provisioning(void) {
    // Start WiFi AP and provisioning web server
    provisioning_server_start();
}

bool wifi_manager_is_provisioned(void) {
    // TODO: Check NVS for stored WiFi credentials
    return false;
}


#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"

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
