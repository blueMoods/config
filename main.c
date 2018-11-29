#include <stdio.h>

#include "config/config.h"

#define FILE_NAME "config"
#define SECTION1 "remote"
#define SECTION2 "local"
#define SECTION3 "DDDDDUUUUU"

void print_hex(uint8_t *value, uint16_t length) {
    for (int i = 0; i < length; ++i) {
        printf("%02x", value[i]);
    }
    printf("\n");
}

int main() {
    config_t *fp;
    fp = config_new(FILE_NAME);
    if (fp == NULL) {
        printf("open failed");
    }
    config_type_t cod = CONFIG_DEV_CLASS;
    config_type_t key = CONFIG_LINK_KEY;
    config_type_t irk = CONFIG_LE_LOCAL_KEY_IRK;

    uint8_t cod_val[3];
    uint8_t key_val[16];
    uint8_t irk_val[16];
    uint16_t cod_len = 3;
    uint16_t key_len = 16;
    uint16_t irk_len = 16;
    bool cod_err, key_err, irk_err;

    cod_err = config_get(fp, SECTION1, cod, cod_val, &cod_len);
    key_err = config_get(fp, SECTION1, key, key_val, &key_len);
    irk_err = config_get(fp, SECTION1, irk, irk_val, &irk_len);

    if (cod_err) {
        printf("cod: ");
        print_hex(cod_val, cod_len);
    }
    if (key_err) {
        printf("key: ");
        print_hex(key_val, key_len);
    }
    if (irk_err) {
        printf("irk: ");
        print_hex(irk_val, irk_len);
    }

    for (int i = 0; i < 16; ++i) {
        key_val[i] = (uint8_t)i;
        irk_val[i] = (uint8_t)(15 - i);
    }
    cod_val[0] = 0x11;
    cod_val[1] = 0x22;
    cod_val[2] = 0x33;
    cod_len = 3;
    key_len = 16;
    irk_len = 16;

    config_set(fp, SECTION1, cod, cod_val, cod_len);
    config_set(fp, SECTION1, key, key_val, key_len);
    config_set(fp, SECTION2, irk, irk_val, irk_len);

    config_save(fp, FILE_NAME);
    return 0;
}