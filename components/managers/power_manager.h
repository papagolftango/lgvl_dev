#pragma once
#ifdef __cplusplus
extern "C" {
#endif

// Power/inactivity state
typedef enum {
	POWER_ACTIVE = 0,
	POWER_IDLE   = 1,
} power_state_t;

typedef void (*power_state_cb_t)(power_state_t state, void *user);

// Initialize power manager with inactivity timeout (seconds).
// If no activity (touch or rotary) occurs for this period, state transitions to POWER_IDLE.
void power_manager_init(uint32_t inactivity_seconds);

// Change inactivity timeout at runtime (seconds)
void power_manager_set_timeout(uint32_t inactivity_seconds);

// Notify the power manager of user activity to keep the system ACTIVE.
// Call this from input handlers (touch, rotary, key, etc.).
void power_manager_notify_activity(void);

// Optional: register a callback to be notified on state changes (ACTIVE<->IDLE).
void power_manager_register_state_cb(power_state_cb_t cb, void *user);

// Query current idle/active state
bool power_manager_is_idle(void);

#ifdef __cplusplus
}
#endif
