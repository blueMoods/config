// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "list.h"
#include "config.h"
#include "nvs.h"

//#define CONFIG_FILE_MAX_SIZE             (1536)//1.5k
//#define CONFIG_FILE_DEFAULE_LENGTH       (2048)
//#define CONFIG_KEY                       "bt_cfg_key"

//typedef struct {
//    char *key;
//    char *value;
//} entry_t;
//
//typedef struct {
//    char *name;
//    list_t *entries;
//} section_t;
//
//typedef struct {
//    list_t *sections;
//}config_t;
//
//typedef struct {
//
//}config_section_node_t;

// Empty definition; this type is aliased to list_node_t.
//struct config_section_iter_t {};

static void config_parse(const char *filename, config_t *config);

/*********************************************************************
 * APIs of section                                                   *
 *********************************************************************/
static section_node_t *section_node_find(const config_t *config, const char *section_name);
static void section_node_free(void *ptr);

static section_t *section_new(void);
static void section_free(section_t *section);
static section_t *section_get(const char *name);
static bool section_set(const char *name, section_t *section);

/*********************************************************************
 * APIs of entry                                                     *
 *********************************************************************/
static entry_t *entry_new(section_t *section, config_type_t config_type, uint8_t *value, uint16_t length);
static bool entry_free(section_t *section, entry_t *entry);
static entry_t *entry_get(section_t *section, config_type_t config_type);
static entry_t *entry_set(section_t *section, config_type_t config_type, uint8_t *value, uint16_t length);

char *osi_strdup(const char *str)
{
    size_t size = strlen(str) + 1;  // + 1 for the null terminator
    char *new_string = (char *)malloc(size);

    if (!new_string) {
        return NULL;
    }

    memcpy(new_string, str, size);
    new_string[size - 1] = 0;
    return new_string;
}

config_t *config_new_empty(void)
{
    config_t *config = malloc(sizeof(config_t));
    if (!config) {
        printf("%s unable to allocate memory for config_t.\n", __func__);
        goto error;
    }

    config->sections = list_new(section_node_free);
    if (!config->sections) {
        printf("%s unable to allocate list for sections.\n", __func__);
        goto error;
    }

    return config;

error:;
    config_free(config);
    return NULL;
}

config_t *config_new(const char *filename)
{
    assert(filename != NULL);

    config_t *config = config_new_empty();
    if (!config) {
        return NULL;
    }

    config_parse(filename, config);

    return config;
}

void config_free(config_t *config)
{
    if (!config) {
        return;
    }

    list_free(config->sections);
    free(config);
}

bool config_has_section(const config_t *config, const char *section_name)
{
    assert(config != NULL);
    assert(section_name != NULL);

    return (section_node_find(config, section_name) != NULL);
}

bool config_has_key(const config_t *config, const char *section_name, config_type_t key)
{
    assert(config != NULL);
    assert(section_name != NULL);

    section_node_t *sec = section_node_find(config, section_name);
    if (sec == NULL) {
        return false;
    }
    if (sec->bit_mask & (1 << key)) {
        return true;
    }
    return false;
}

bool config_get(const config_t *config, const char *section_name, const config_type_t key, void *value,  uint16_t *length)
{
    assert(config != NULL);
    assert(section_name != NULL);

    if (!config_has_key(config, section_name, key)) {
        *length = 0;
        return false;
    }

    section_t *section = section_get(section_name);
    if (section == NULL) {
        return false;
    }
    entry_t *entry = entry_get(section, key);
    if (entry == NULL) {
        return false;
    }
    if (entry->length > *length) {
        // Buf for value is not long enough
        *length = entry->length;
        return false;
    }

    *length = entry->length;
    memcpy(value, entry->value, entry->length);

    section_free(section);
    return true;
}

void config_set(config_t *config, const char *section_name, const config_type_t key, void *value, uint16_t length)
{
    assert(config != NULL);
    assert(section_name != NULL);
    assert(key != CONFIG_SECTONS);

    section_t *section;
    section_node_t *sec = section_node_find(config, section_name);
    if (!sec) {
        size_t len = strlen(section_name);
        if (len > 25){
            printf("ERROR: name is too long.\n");
            return;
        }

        sec = malloc(sizeof(section_node_t));
        sec->name = osi_strdup(section_name);
        sec->bit_mask = 0;
        list_append(config->sections, sec);

        section = section_new();
    } else {
        section = section_get(section_name);
    }

    if(section == NULL) {
        printf("ERROR: %s failed.\n", __func__);
        return;
    }

    sec->bit_mask |= (1 << key);
    entry_set(section, key, value, length);
    section_set(section_name, section);
    section_free(section);
}

bool config_remove_section(config_t *config, const char *section_name)
{
    assert(config != NULL);
    assert(section_name != NULL);

    section_node_t *sec = section_node_find(config, section_name);
    if (!sec) {
        return false;
    }

    section_set(section_name, NULL);

    free(sec->name);
    list_remove(config->sections, sec);
    return true;
}

bool config_remove_key(config_t *config, const char *section_name, const config_type_t key)
{
    assert(config != NULL);
    assert(section_name != NULL);
    assert(key != CONFIG_SECTONS);

    if (!config_has_key(config, section_name, key)) {
        return false;
    }

    section_node_t *sec = section_node_find(config, section_name);
    if (sec == NULL) {
        return false;
    }
    if (sec->bit_mask == (1 << key)) {
        // only have one entry in section, remove it.
        return config_remove_section(config, section_name);
    }

    section_t *section = section_get(section_name);
    if (section ==NULL) {
        return false;
    }

    sec->bit_mask &= ~(1 << key);
    entry_t *entry = entry_get(section, key);
    entry_free(section, entry);

    section_set(section_name, section);
    section_free(section);
    return true;
}

const config_section_node_t *config_section_begin(const config_t *config)
{
    assert(config != NULL);
    return (const config_section_node_t *)list_begin(config->sections);
}

const config_section_node_t *config_section_end(const config_t *config)
{
    assert(config != NULL);
    return (const config_section_node_t *)list_end(config->sections);
}

const config_section_node_t *config_section_next(const config_section_node_t *node)
{
    assert(node != NULL);
    return (const config_section_node_t *)list_next((const list_node_t *)node);
}

const char *config_section_name(const config_section_node_t *node)
{
    assert(node != NULL);
    const list_node_t *lnode = (const list_node_t *)node;
    const section_node_t *sec = (const section_node_t *)list_node(lnode);
    return sec->name;
}

bool config_save(const config_t *config, const char *filename)
{
    assert(config != NULL);
    assert(filename != NULL);
    assert(*filename != '\0');

    section_t *section = section_new();
    if (section == NULL) {
        return false;
    }

    uint8_t entry_value[25 + 4];
    uint8_t *p;
    uint8_t entry_length = 0;

    for (const list_node_t *node = list_begin(config->sections); node != list_end(config->sections); node = list_next(node)) {
        p = entry_value;
        entry_length = 0;
        section_node_t *sec = list_node(node);
        memcpy(p, sec->name, strlen(sec->name));
        p += strlen(sec->name);
        entry_length += strlen(sec->name);
        *p = 0;
        p ++;
        entry_length ++;
        memcpy(p, &(sec->bit_mask), sizeof(config_mask_t));
        entry_length += sizeof(config_mask_t);
        entry_set(section, CONFIG_SECTONS, entry_value, entry_length);
    }

    section_set(filename, section);
    section_free(section);

    return true;
}

static void config_parse(const char *filename, config_t *config)
{
    assert(filename != 0);
    assert(config != NULL);

    section_t *section = section_get(filename);
    if (section == NULL || !(section->bit_mask & CONFIG_SECTONS_MASK)) {
        return;
    }

    config_mask_t *p;
    section_node_t *sec;
    entry_t *entry = entry_get(section, CONFIG_SECTONS);
    while (entry) {
        sec = malloc(sizeof(section_node_t));
        sec->name = osi_strdup((const char*)entry->value);
        p = (config_mask_t *)(entry->value + entry->length - sizeof(config_mask_t));
        sec->bit_mask = *p;
        list_append(config->sections, sec);

        entry_free(section, entry);
        entry = entry_get(section, CONFIG_SECTONS);
    }

    section_free(section);
}

/*********************************************************************
 * APIs of section                                                   *
 *********************************************************************/
static section_node_t *section_node_find(const config_t *config, const char *section_name)
{
    for (const list_node_t *node = list_begin(config->sections); node != list_end(config->sections); node = list_next(node)) {
        section_node_t *sec = list_node(node);
        if (!strcmp(sec->name, section_name)) {
            return sec;
        }
    }

    return NULL;
}

static void section_node_free(void *ptr)
{
    section_node_t *sec = ptr;
    free(sec->name);
    free(sec);
}
//static section_node_t *section_node_new(const config_t *config, const char *section_name)
//{
//    section_node_t *sec = malloc(sizeof(section_node_t));
//    size_t len = strlen(section_name);
//    if (len > 25){
//        printf("ERROR: name is too long.\n");
//        return NULL;
//    }
//
//    sec->name = osi_strdup(section_name);
//
//    list_append(config->sections, sec);
//}
//
//static section_node_t *section_node_free(const config_t *config, const char *section_name)
//{
//    section_node_t *sec = section_node_find(config, section_name);
//    if (!sec) {
//        return false;
//    }
//    free(sec->name);
//    list_remove(config->sections, sec);
//}

static section_t *section_new()
{
    section_t *section = malloc(CONFIG_FILE_MAX_SIZE);
    if (!section) {
        printf("ERROR: Malloc failed.\n");
        return NULL;
    }
    section->bit_mask = 0;
    section->length = 0;

    return section;
}

static void section_free(section_t *section)
{
    if (!section) {
        return;
    }
    free(section);
}

static section_t *section_get(const char *name)
{
    nvs_handle fp;
    esp_err_t err = nvs_open(name, NVS_READONLY, &fp);
    if (err != ESP_OK) {
        return NULL;
    }

    section_t *section = section_new();
    size_t length = CONFIG_FILE_MAX_SIZE;
    err = nvs_get_blob(fp, NULL, section, &length);
    if (err != ESP_OK) {
        // Never run to here.
        printf("ERROR: nvs_get_blob failed.\n");
        section_free(section);
        nvs_close(fp);
        return NULL;
    }

    if (length != section->length + sizeof(section_t)) {
        // Never run to here.
        printf("ERROR: nvs_get_blob return an error length.\n");
        printf("%d != %d + %d\n", (int)length, section->length, (int)sizeof(section_t));
        section_free(section);
        nvs_close(fp);
        return NULL;
    }

    nvs_close(fp);
    return section;
}

static bool section_set(const char *name, section_t *section)
{
    if (section == NULL) {
        nvs_erase(name);
        return true;
    }

    nvs_handle fp;
    esp_err_t err = nvs_open(name, NVS_READWRITE, &fp);
    if (err != ESP_OK) {
        // Never run to here.
        printf("ERROR: Open nvs failed\n");
        nvs_close(fp);
        return false;
    }

    err = nvs_set_blob(fp, NULL, section, section->length + sizeof(section_t));
    if (err != ESP_OK) {
        // Never run to here.
        printf("ERROR: nvs_set_blob failed.\n");
        nvs_close(fp);
        return false;
    }

    nvs_close(fp);
    return true;
}

/*********************************************************************
 * APIs of entry                                                     *
 *********************************************************************/

static entry_t *entry_new(section_t *section, config_type_t config_type, uint8_t *value, uint16_t length)
{
    uint16_t length_new = section->length + sizeof(entry_t) + length;
    if ((length_new + sizeof(section)) > CONFIG_FILE_MAX_SIZE) {
        printf("ERROR: section full. (current/max) section size: (%d/%d)\n", (int16_t)(section->length + sizeof(section)), length_new);
        return NULL;
    }

    entry_t *entry = (entry_t *)(section->value + section->length);
    section->length = length_new;
    section->bit_mask |= (1 << config_type);

    entry->config_type = config_type;
    entry->length = length;
    memcpy(entry->value, value, length);
    return entry;
}

static bool entry_free(section_t *section, entry_t *entry)
{
    void *entry_end = (uint8_t *)(entry->value + entry->length);
    void *section_end = (uint8_t *)(section->value + section->length);
    if (((void *)entry < (void *)(section->value))
        || (entry_end > section_end)) {
        printf("ERROR: The entry is not in the section.\n");
        return false;
    }

    uint16_t entry_length = entry->length  + sizeof(entry_t);
    config_type_t entry_type = entry->config_type;

    uint32_t move_length = (uint32_t)(section_end - entry_end);
    memmove(entry, entry_end, move_length);

    section->length -= entry_length;

    if(entry_type == CONFIG_SECTONS && entry_get(section, entry_type) != NULL) {
        return true;
    }
    section->bit_mask &= ~(1 << entry_type);
    return true;
}

static entry_t *entry_get(section_t *section, config_type_t config_type)
{
    if (!(section->bit_mask & (1 << config_type))) {
        return NULL;
    }

    entry_t *entry;
    void *p = (void *)section->value;
    void *secton_end = section->value + section->length;
    while (p < secton_end) {
        entry = (entry_t *)p;
        if(entry->config_type == config_type) {
            return entry;
        }
        p += (sizeof(entry_t) + entry->length);
    }

    section->bit_mask &= ~(1 << config_type);
    return NULL;
}

static entry_t *entry_set(section_t *section, config_type_t config_type, uint8_t *value, uint16_t length)
{
    if (config_type == CONFIG_SECTONS) {
        return entry_new(section, config_type, value, length);
    }

    entry_t *entry = entry_get(section, config_type);
    if (entry != NULL) {
        entry_free(section, entry);
    }

    return entry_new(section, config_type, value, length);
}