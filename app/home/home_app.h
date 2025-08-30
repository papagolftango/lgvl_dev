#ifdef __cplusplus
extern "C" {
#endif

// Bin state machine
typedef enum {
	BIN_STATE_IDLE,        // Bin is in normal/idle state
	BIN_STATE_PREPARING,   // Bin is being prepared for emptying
	BIN_STATE_EMPTYING     // Bin is currently being emptied
} bin_state_t;
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void home_app_init(void);
void home_app_tick(void);
void home_app_cleanup(void);
void home_app_destroy(void);
// Called always, even when not active
void home_app_process(void);

#ifdef __cplusplus
}
#endif
