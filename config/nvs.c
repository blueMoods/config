//
// Created by baohongde on 18-11-20.
//
#include <stdio.h>
#include <string.h>
#include "nvs.h"

esp_err_t nvs_open(const char* name, nvs_open_mode open_mode, nvs_handle *out_handle){
    FILE *fp = NULL;
    char f_path[FILE_PATH_LEN + MAX_FILE_NAME_LEN + 1] = {'\0'};
    memcpy(f_path, FILE_PATH, FILE_PATH_LEN);
    size_t f_name_len = strlen(name);
    if (f_name_len > MAX_FILE_NAME_LEN) {
        printf("File name is too long, change it no more than %d!\n", MAX_FILE_NAME_LEN);
        return 1;
    }

    memcpy(f_path + FILE_PATH_LEN, name, f_name_len);

    if (open_mode == NVS_READONLY) {
        fp = fopen(f_path, "r");
    } else if (open_mode == NVS_READWRITE) {
        fp = fopen(f_path, "w+");
    } else {
        fp = NULL;
    }
    if (fp == NULL) {
        return 1;
    }
    *out_handle = (nvs_handle)fp;
    return 0;
}

void nvs_close(nvs_handle handle){
    fclose((FILE *)handle);
    return;
}
esp_err_t nvs_set_blob(nvs_handle handle, const char* key, const void* value, size_t length){
//    printf("%s unused param key: '%s'\n", __func__, key);
    fwrite(value, length, 1, (FILE *)handle);
    return 0;
}
esp_err_t nvs_get_blob(nvs_handle handle, const char* key, void* out_value, size_t* length){
//    printf("%s unused param key: '%s'\n", __func__, key);
    size_t size = fread(out_value, 1, *length, (FILE *)handle);
    if (size > *length) {
        *length = size;
        return 1;
    } else {
        *length = size;
    }
    return 0;
}

esp_err_t nvs_erase(const char* name) {
    char f_path[FILE_PATH_LEN + MAX_FILE_NAME_LEN + 1] = {'\0'};
    memcpy(f_path, FILE_PATH, FILE_PATH_LEN);
    size_t f_name_len = strlen(name);
    if (f_name_len > MAX_FILE_NAME_LEN) {
        printf("File name is too long, change it no more than %d!\n", MAX_FILE_NAME_LEN);
        return 1;
    }

    memcpy(f_path + 14, name, f_name_len);
    return remove(f_path);
}
