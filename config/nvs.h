//
// Created by baohongde on 18-11-20.
//

#ifndef CONFIG_NVS_H
#define CONFIG_NVS_H

#include <stdint.h>
#include <stddef.h>

typedef int32_t nvs_handle;
typedef int32_t esp_err_t;

typedef enum {
    NVS_READONLY,  /*!< Read only */
    NVS_READWRITE  /*!< Read and write */
} nvs_open_mode;

esp_err_t nvs_open(const char* name, nvs_open_mode open_mode, nvs_handle *out_handle);
void nvs_close(nvs_handle handle);
esp_err_t nvs_set_blob(nvs_handle handle, const char* key, const void* value, size_t length);
esp_err_t nvs_get_blob(nvs_handle handle, const char* key, void* out_value, size_t* length);
esp_err_t nvs_commit(nvs_handle handle);


#endif //CONFIG_NVS_H
