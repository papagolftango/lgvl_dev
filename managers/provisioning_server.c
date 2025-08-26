#include "provisioning_server.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "nvs_flash.h"
#include <string.h>
#include <ctype.h>

static const char *TAG = "provisioning_server";
static httpd_handle_t server = NULL;
static provisioning_server_credentials_cb_t credentials_cb = NULL;

void provisioning_server_set_callback(provisioning_server_credentials_cb_t cb) {
    credentials_cb = cb;
}

// Simple HTML form for WiFi and MQTT
static const char *form_html =
    "<html><body><h2>Provision WiFi & MQTT</h2>"
    "<form method='POST' action='/submit'>"
    "WiFi SSID: <input name='ssid'><br>"
    "WiFi Password: <input name='password' type='password'><br>"
    "MQTT Host: <input name='mqtt_host'><br>"
    "MQTT User: <input name='mqtt_user'><br>"
    "MQTT Password: <input name='mqtt_pass' type='password'><br>"
    "<input type='submit' value='Provision'>"
    "</form></body></html>";

static esp_err_t root_get_handler(httpd_req_t *req) {
    httpd_resp_send(req, form_html, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t submit_post_handler(httpd_req_t *req) {
    char buf[512];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    buf[ret] = '\0';

    // URL decode helper
    char decoded[512];
    int di = 0;
    for (int si = 0; buf[si] && di < (int)sizeof(decoded) - 1; ++si) {
        if (buf[si] == '%') {
            if (isxdigit((unsigned char)buf[si+1]) && isxdigit((unsigned char)buf[si+2])) {
                char hex[3] = { buf[si+1], buf[si+2], 0 };
                decoded[di++] = (char)strtol(hex, NULL, 16);
                si += 2;
            }
        } else if (buf[si] == '+') {
            decoded[di++] = ' ';
        } else {
            decoded[di++] = buf[si];
        }
    }
    decoded[di] = '\0';

    // Robust key-value parsing for each field
    char ssid[64] = "", password[64] = "", mqtt_host[64] = "", mqtt_user[64] = "", mqtt_pass[64] = "";
    char *p, *end;

    // Helper macro to extract value for a key
#define EXTRACT_FIELD(key, buf, out, maxlen) \
    if ((p = strstr(buf, key "="))) { \
        p += strlen(key) + 1; \
        end = strchr(p, '&'); \
        size_t len = end ? (size_t)(end - p) : strlen(p); \
        if (len >= maxlen) len = maxlen - 1; \
        strncpy(out, p, len); \
        out[len] = '\0'; \
    }

    EXTRACT_FIELD("ssid", decoded, ssid, sizeof(ssid));
    EXTRACT_FIELD("password", decoded, password, sizeof(password));
    EXTRACT_FIELD("mqtt_host", decoded, mqtt_host, sizeof(mqtt_host));
    EXTRACT_FIELD("mqtt_user", decoded, mqtt_user, sizeof(mqtt_user));
    EXTRACT_FIELD("mqtt_pass", decoded, mqtt_pass, sizeof(mqtt_pass));

    ESP_LOGI(TAG, "Parsed fields:");
    ESP_LOGI(TAG, "  SSID: '%s'", ssid);
    ESP_LOGI(TAG, "  WiFi PASS: '%s'", password);
    ESP_LOGI(TAG, "  MQTT HOST: '%s'", mqtt_host);
    ESP_LOGI(TAG, "  MQTT USER: '%s'", mqtt_user);
    ESP_LOGI(TAG, "  MQTT PASS: '%s'", mqtt_pass);

    if (credentials_cb) credentials_cb(ssid, password, mqtt_host, mqtt_user, mqtt_pass);
    httpd_resp_sendstr(req, "<html><body><h2>Provisioning Complete</h2><p>You may now disconnect.</p></body></html>");
    return ESP_OK;
}

void provisioning_server_start(void) {
    // Prevent double-start
    if (server) {
        ESP_LOGW(TAG, "HTTP server already running, stopping before restart.");
        provisioning_server_stop();
    }
    // Start WiFi AP
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    wifi_config_t ap_config = {
        .ap = {
            .ssid = "ESP32-Setup",
            .ssid_len = 0,
            .channel = 1,
            .password = "",
            .max_connection = 4,
            .authmode = WIFI_AUTH_OPEN
        }
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "WiFi AP started. Connect and browse to http://192.168.4.1");

    // Start HTTP server
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    httpd_uri_t root = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = root_get_handler
    };
    httpd_uri_t submit = {
        .uri = "/submit",
        .method = HTTP_POST,
        .handler = submit_post_handler
    };
    ESP_ERROR_CHECK(httpd_start(&server, &config));
    httpd_register_uri_handler(server, &root);
    httpd_register_uri_handler(server, &submit);
    ESP_LOGI(TAG, "Provisioning web server started.");
}

void provisioning_server_stop(void) {
    if (server) {
        httpd_stop(server);
        server = NULL;
    }
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_LOGI(TAG, "Provisioning server stopped.");
}

void provisioning_server_reset(void) {
    // Open NVS handle
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("provision", NVS_READWRITE, &nvs_handle);
    if (err == ESP_OK) {
        nvs_erase_all(nvs_handle); // Erase all keys in this namespace
        nvs_close(nvs_handle);
        nvs_flash_erase(); // Optionally erase all NVS (if you want a full reset)
    }
    // Optionally, add a log message
    // ESP_LOGI(TAG, "Provisioning credentials erased. Device will require re-provisioning.");
}
