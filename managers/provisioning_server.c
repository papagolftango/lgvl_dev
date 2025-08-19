#include "provisioning_server.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "nvs_flash.h"
#include <string.h>

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

    // Parse form data (very basic, not robust)
    char ssid[64] = "", password[64] = "", mqtt_host[64] = "", mqtt_user[64] = "", mqtt_pass[64] = "";
    sscanf(buf, "ssid=%63[^&]&password=%63[^&]&mqtt_host=%63[^&]&mqtt_user=%63[^&]&mqtt_pass=%63s", ssid, password, mqtt_host, mqtt_user, mqtt_pass);
    ESP_LOGI(TAG, "Received: SSID=%s, PASS=%s, MQTT_HOST=%s, MQTT_USER=%s, MQTT_PASS=%s", ssid, password, mqtt_host, mqtt_user, mqtt_pass);
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
