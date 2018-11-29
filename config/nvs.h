//
// Created by baohongde on 18-11-20.
//

#ifndef CONFIG_NVS_H
#define CONFIG_NVS_H

#include <stdint.h>
#include <stddef.h>

#define MAX_FILE_NAME_LEN   25
#define FILE_PATH_LEN       15
#define FILE_PATH           "../file_system/"

typedef int32_t nvs_handle;

#define ESP_OK          0
#define ESP_FAIL        -1
typedef int32_t esp_err_t;

typedef enum {
    NVS_READONLY,  /*!< Read only */
    NVS_READWRITE  /*!< Read and write */
} nvs_open_mode;

esp_err_t nvs_open(const char* name, nvs_open_mode open_mode, nvs_handle *out_handle);
void nvs_close(nvs_handle handle);
esp_err_t nvs_set_blob(nvs_handle handle, const char* key, const void* value, size_t length);
esp_err_t nvs_get_blob(nvs_handle handle, const char* key, void* out_value, size_t* length);
esp_err_t nvs_erase(const char* name);

#endif //CONFIG_NVS_H
