#include "provisioning_server.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "nvs_flash.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#include "app_manager.h"
#include "power_manager.h"
#include "touch_manager.h"
#include "encoder_manager.h"
#include "lcd_bl_pwm_bsp.h"
#include "haptic_manager.h"
#include "energy_controller.h"

static const char *TAG = "provisioning_server";
static httpd_handle_t server = NULL;
static provisioning_server_credentials_cb_t credentials_cb = NULL;
// Test-mode only: track last rotary target app name when rotating while ACTIVE
#ifdef CONFIG_TEST_MODE
static char s_last_rotary_target[16] = {0};
#endif

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

#ifdef CONFIG_TEST_MODE
    // Minimal JSON helper
    static esp_err_t send_json(httpd_req_t *req, const char *json){
        httpd_resp_set_type(req, "application/json");
        httpd_resp_sendstr(req, json);
        return ESP_OK;
    }

    // GET /test/app/active -> returns current app name as text or JSON
    static esp_err_t test_get_active_app(httpd_req_t *req){
        app_id_t id = app_manager_get_active();
        const app_descriptor_t *desc = app_manager_get_descriptor(id);
        const char *name = desc && desc->name ? desc->name : "Unknown";
        httpd_resp_set_type(req, "text/plain");
        return httpd_resp_sendstr(req, name);
    }
    static httpd_uri_t uri_get_active_app = {
        .uri = "/test/app/active",
        .method = HTTP_GET,
        .handler = test_get_active_app
    };

    // POST /test/app/active {"name":"Energy"}
    static esp_err_t test_post_active_app(httpd_req_t *req){
        char buf[64] = {0};
        int r = httpd_req_recv(req, buf, sizeof(buf)-1);
        if (r <= 0) return httpd_resp_send_500(req);
        // naive parse for name
        char *p = strstr(buf, "\"name\"");
        if (p){
            p = strchr(p, ':');
            if (p){
                while (*p && (*p == ':' || *p == ' ' || *p == '\"')) p++;
                char name[32] = {0};
                int i=0; while (*p && *p!='\"' && i < (int)sizeof(name)-1){ name[i++] = *p++; }
                // map string to app_id
                for (int id=0; id<APP_ID_COUNT; ++id){
                    const app_descriptor_t *d = app_manager_get_descriptor((app_id_t)id);
                    if (d && d->name && strcmp(d->name, name) == 0){ app_manager_set_active((app_id_t)id); break; }
                }
            }
        }
        httpd_resp_set_type(req, "application/json");
        return httpd_resp_sendstr(req, "{\"ok\":true}");
    }
    static httpd_uri_t uri_post_active_app = {
        .uri = "/test/app/active",
        .method = HTTP_POST,
        .handler = test_post_active_app
    };

    // POST /test/input/tap -> simulate a touch (app cycle). For wake semantics, notify power.
    static esp_err_t test_post_input_tap(httpd_req_t *req){
        bool was_idle = power_manager_is_idle();
        power_manager_notify_activity();
        if (!was_idle){
            app_manager_next_app();
        }
        httpd_resp_set_type(req, "application/json");
        return httpd_resp_sendstr(req, "{\"ok\":true}");
    }
    static httpd_uri_t uri_post_input_tap = {
        .uri = "/test/input/tap",
        .method = HTTP_POST,
        .handler = test_post_input_tap
    };

    // POST /test/input/rotate {"dir":"LEFT|RIGHT"}
    static esp_err_t test_post_input_rotate(httpd_req_t *req){
        char buf[64] = {0};
        int r = httpd_req_recv(req, buf, sizeof(buf)-1);
        if (r <= 0) return httpd_resp_send_500(req);
        bool was_idle = power_manager_is_idle();
        power_manager_notify_activity();
        if (!was_idle){
            if (strstr(buf, "LEFT")) {
                extern void app_manager_register_encoder_cb(app_id_t app, app_encoder_cb_t cb);
                // Directly notify active app's encoder through app_manager handler
                // Expose the static handler via a stub: simulate by calling global callback through encoder_manager
                extern void encoder_manager_register_user_cb(encoder_user_cb_t cb);
                // Call app_manager's on-encoder path
                // Since app_manager_on_encoder is static, replicate minimal logic here:
                app_id_t cur = app_manager_get_active();
                const app_descriptor_t *d = app_manager_get_descriptor(cur);
                // Use registered per-app encoder callback table (not exposed). Fallback: switch on app ID to call known handlers where available.
                // For Energy app, simulate by posting LEFT/RIGHT via its public controller APIs.
                if (d && d->name && strcmp(d->name, "Energy") == 0){
                    extern void energy_controller_prev_mode(void);
                    energy_controller_prev_mode();
                    strncpy(s_last_rotary_target, d->name, sizeof(s_last_rotary_target)-1);
                    s_last_rotary_target[sizeof(s_last_rotary_target)-1] = '\0';
                }
            } else {
                app_id_t cur = app_manager_get_active();
                const app_descriptor_t *d = app_manager_get_descriptor(cur);
                if (d && d->name && strcmp(d->name, "Energy") == 0){
                    extern void energy_controller_next_mode(void);
                    energy_controller_next_mode();
                    strncpy(s_last_rotary_target, d->name, sizeof(s_last_rotary_target)-1);
                    s_last_rotary_target[sizeof(s_last_rotary_target)-1] = '\0';
                }
            }
        }
        httpd_resp_set_type(req, "application/json");
        return httpd_resp_sendstr(req, "{\"ok\":true}");
    }
    static httpd_uri_t uri_post_input_rotate = {
        .uri = "/test/input/rotate",
        .method = HTTP_POST,
        .handler = test_post_input_rotate
    };

    // POST /test/power/timeout {"seconds":N}
    static esp_err_t test_post_power_timeout(httpd_req_t *req){
        char buf[64] = {0};
        int r = httpd_req_recv(req, buf, sizeof(buf)-1);
        if (r <= 0) return httpd_resp_send_500(req);
        int sec = 0;
        char *p = strstr(buf, "seconds");
        if (p){ p = strchr(p, ':'); if (p) sec = atoi(p+1); }
        if (sec > 0) power_manager_set_timeout((uint32_t)sec);
        return send_json(req, "{\"ok\":true}");
    }
    static httpd_uri_t uri_post_power_timeout = {
        .uri = "/test/power/timeout",
        .method = HTTP_POST,
        .handler = test_post_power_timeout
    };

    // GET /test/power/state -> ACTIVE|IDLE
    static esp_err_t test_get_power_state(httpd_req_t *req){
        httpd_resp_set_type(req, "text/plain");
        return httpd_resp_sendstr(req, power_manager_is_idle() ? "IDLE" : "ACTIVE");
    }
    static httpd_uri_t uri_get_power_state = {
        .uri = "/test/power/state",
        .method = HTTP_GET,
        .handler = test_get_power_state
    };

    // POST /test/power/state {"state":"ACTIVE|IDLE"}
    static esp_err_t test_post_power_state(httpd_req_t *req){
        char buf[64] = {0};
        int r = httpd_req_recv(req, buf, sizeof(buf)-1);
        if (r <= 0) return httpd_resp_send_500(req);
        if (strstr(buf, "IDLE")){
            // Force IDLE by setting a tiny timeout and not notifying; also fade backlight to 0
            power_manager_set_timeout(1);
            // Let timer worker handle the state; but we can nudge immediately by setting backlight
            lcd_bl_pwm_bsp_fade_to_wait(LCD_PWM_MODE_0, 0, true);
        } else {
            // ACTIVE: notify activity and restore backlight
            power_manager_notify_activity();
            lcd_bl_pwm_bsp_fade_to_wait(LCD_PWM_MODE_50, 0, true);
        }
        return send_json(req, "{\"ok\":true}");
    }
    static httpd_uri_t uri_post_power_state = {
        .uri = "/test/power/state",
        .method = HTTP_POST,
        .handler = test_post_power_state
    };

    // GET /test/power/backlight -> numeric duty (0-255)
    static esp_err_t test_get_power_backlight(httpd_req_t *req){
        char out[16];
        snprintf(out, sizeof(out), "%u", (unsigned)lcd_bl_pwm_bsp_get_duty());
        httpd_resp_set_type(req, "text/plain");
        return httpd_resp_sendstr(req, out);
    }
    static httpd_uri_t uri_get_power_backlight = {
        .uri = "/test/power/backlight",
        .method = HTTP_GET,
        .handler = test_get_power_backlight
    };

    // GET /test/haptics/last -> return empty for now (global haptics not tracked)
    static esp_err_t test_get_haptics_last(httpd_req_t *req){
        uint8_t eff = haptic_manager_get_last_and_clear();
        const char *name = (eff == 1) ? "short" : "";
        httpd_resp_set_type(req, "text/plain");
        return httpd_resp_sendstr(req, name);
    }
    static httpd_uri_t uri_get_haptics_last = {
        .uri = "/test/haptics/last",
        .method = HTTP_GET,
        .handler = test_get_haptics_last
    };

    // GET /test/energy/viewmodel -> JSON of current energy metrics & mode
    static esp_err_t test_get_energy_vm(httpd_req_t *req){
        extern float energy_balance, energy_solar, energy_used;
        extern float energy_peak_solar, energy_peak_used;
        extern int energy_pulse_count, cumulative_pulse;
        extern float energy_tariff_rate_gbp_per_kwh;
#ifdef CONFIG_TEST_MODE
        extern const char *energy_screen_get_mode_name(void);
        const char *mode = energy_screen_get_mode_name();
#else
        const char *mode = "unknown";
#endif
        int delta = energy_pulse_count - cumulative_pulse;
        if (delta < 0) delta = 0;
        char buf[384];
        // Note: balance/solar/used and peaks as integers (rounded) for simplicity
        snprintf(buf, sizeof(buf),
            "{\"mode\":\"%s\",\"balance\":%.0f,\"solar\":%.0f,\"used\":%.0f,\"peak_solar\":%.0f,\"peak_used\":%.0f,\"pulses\":%d,\"baseline\":%d,\"tariff\":%.3f}",
            mode, energy_balance, energy_solar, energy_used, energy_peak_solar, energy_peak_used, energy_pulse_count, cumulative_pulse, energy_tariff_rate_gbp_per_kwh);
        return send_json(req, buf);
    }
    static httpd_uri_t uri_get_energy_vm = {
        .uri = "/test/energy/viewmodel",
        .method = HTTP_GET,
        .handler = test_get_energy_vm
    };

    // POST /test/energy/viewmodel -> mutate subset: {mode:".."} or {baseline:int}
    static esp_err_t test_post_energy_vm(httpd_req_t *req){
        extern int cumulative_pulse, energy_pulse_count;
        extern float energy_tariff_rate_gbp_per_kwh;
        char content[160];
        int len = httpd_req_recv(req, content, sizeof(content)-1);
        if (len < 0) return ESP_FAIL;
        content[len] = '\0';
        // naive parsing (small payloads): look for keys
        const char *p;
        p = strstr(content, "\"baseline\"");
        if (p){
            const char *colon = strchr(p, ':');
            if (colon){ cumulative_pulse = atoi(colon+1); }
        }
        p = strstr(content, "\"tariff\"");
        if (p){
            const char *colon = strchr(p, ':');
            if (colon){ energy_tariff_rate_gbp_per_kwh = (float)atof(colon+1); }
        }
        p = strstr(content, "\"pulses\"");
        if (p){
            const char *colon = strchr(p, ':');
            if (colon){ energy_pulse_count = atoi(colon+1); }
        }
        p = strstr(content, "\"mode\"");
        if (p){
            const char *colon = strchr(p, ':');
            if (colon){
                const char *q1 = strchr(colon, '"');
                if (q1){ const char *q2 = strchr(q1+1, '"');
                    if (q2 && q2 > q1+1){
                        char name[32];
                        size_t n = (size_t)(q2 - (q1+1));
                        if (n >= sizeof(name)) n = sizeof(name)-1;
                        memcpy(name, q1+1, n); name[n] = '\0';
#ifdef CONFIG_TEST_MODE
                        extern bool energy_screen_set_mode_name(const char*);
                        energy_screen_set_mode_name(name);
#endif
                    }
                }
            }
        }
        return send_json(req, "{\"ok\":true}");
    }
    static httpd_uri_t uri_post_energy_vm = {
        .uri = "/test/energy/viewmodel",
        .method = HTTP_POST,
        .handler = test_post_energy_vm
    };

    // POST /test/reset -> restore to known state
    static esp_err_t test_post_reset(httpd_req_t *req){
        // Active app to Energy
        app_manager_set_active(APP_ID_ENERGY);
        // Power ACTIVE and backlight mid
        power_manager_notify_activity();
        lcd_bl_pwm_bsp_fade_to_wait(LCD_PWM_MODE_50, 0, true);
        // Timeout 120s
        power_manager_set_timeout(120);
        // Clear any pending haptic by reading
        (void)haptic_manager_get_last_and_clear();
        return send_json(req, "{\"ok\":true}");
    }
    static httpd_uri_t uri_post_reset = {
        .uri = "/test/reset",
        .method = HTTP_POST,
        .handler = test_post_reset
    };

    // GET /test/input/last_rotary_target
    static esp_err_t test_get_last_rotary_target(httpd_req_t *req){
        httpd_resp_set_type(req, "text/plain");
        return httpd_resp_sendstr(req, s_last_rotary_target);
    }
    static httpd_uri_t uri_get_last_rotary_target = {
        .uri = "/test/input/last_rotary_target",
        .method = HTTP_GET,
        .handler = test_get_last_rotary_target
    };

    httpd_register_uri_handler(server, &uri_get_active_app);
    httpd_register_uri_handler(server, &uri_post_active_app);
    httpd_register_uri_handler(server, &uri_post_input_tap);
    httpd_register_uri_handler(server, &uri_post_input_rotate);
    httpd_register_uri_handler(server, &uri_post_power_timeout);
    httpd_register_uri_handler(server, &uri_get_power_state);
    httpd_register_uri_handler(server, &uri_post_power_state);
    httpd_register_uri_handler(server, &uri_get_power_backlight);
    httpd_register_uri_handler(server, &uri_get_haptics_last);
    httpd_register_uri_handler(server, &uri_get_energy_vm);
    httpd_register_uri_handler(server, &uri_post_energy_vm);
    httpd_register_uri_handler(server, &uri_post_reset);
    httpd_register_uri_handler(server, &uri_get_last_rotary_target);
#endif // CONFIG_TEST_MODE
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
