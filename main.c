#include <stdio.h>

#include "config/config.h"

#define FILE_NAME "test.txt"
#define SECTION1 "section_1"
#define SECTION2 "section_2"

int main() {
    printf("Hello, World!\n");
    config_t *fp;
    fp = config_new(FILE_NAME);
    if (fp == NULL) {
        printf("open failed");
    }

    printf("config_get_string(BHD): %s\n", config_get_string(fp, SECTION1, "BHD", "not find"));
    printf("config_get_string(TH): %s\n", config_get_string(fp, SECTION1, "TH", "not find"));

    config_set_int(fp, SECTION1, "count", 2);
    config_set_string(fp, SECTION1, "BHD", "baohongde", true);
    config_set_string(fp, SECTION1, "WMY", "wangmengyang", true);

    printf("set config...\n\n");
    printf("config_get_string(BHD): %s\n", config_get_string(fp, SECTION1, "BHD", "not find"));
    printf("config_get_string(TH): %s\n", config_get_string(fp, SECTION1, "TH", "not find"));

    config_set_bool(fp, SECTION2, "BOOL", false);

    config_save(fp, FILE_NAME);
    return 0;
}