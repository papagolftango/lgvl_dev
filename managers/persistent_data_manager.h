#pragma once
#include <stddef.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

// Initialize persistent data manager
void persistent_data_manager_init(void);

// Save a key-value pair (string)
bool persistent_data_manager_set(const char *key, const char *value);

// Load a value by key (string)
bool persistent_data_manager_get(const char *key, char *out_value, size_t max_len);

// Remove a key
bool persistent_data_manager_remove(const char *key);

#ifdef __cplusplus
}
#endif
