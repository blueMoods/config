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

#ifndef __CONFIG_H__
#define __CONFIG_H__

// This module implements a configuration parser. Clients can query the
// contents of a configuration file through the interface provided here.
// The current implementation is read-only; mutations are only kept in
// memory. This parser supports the INI file format.

// Implementation notes:
// - Key/value pairs that are not within a section are assumed to be under
//   the |CONFIG_DEFAULT_SECTION| section.
// - Multiple sections with the same name will be merged as if they were in
//   a single section.
// - Empty sections with no key/value pairs will be treated as if they do
//   not exist. In other words, |config_has_section| will return false for
//   empty sections.
// - Duplicate keys in a section will overwrite previous values.
// - All strings are case sensitive.

#include <stdbool.h>
#include <stdint.h>
#include "list.h"


#define CONFIG_FILE_MAX_SIZE             (1536)//1.5k
#define CONFIG_FILE_DEFAULE_LENGTH       (2048)
#define CONFIG_KEY                       "bt_cfg_key"

/**
 * Bit mask
 */
#define CONFIG_SECTONS                  0

#define CONFIG_DEV_CLASS                1
#define CONFIG_PIN_LENGTH               2
#define CONFIG_LINK_KEY_TYPE            3
#define CONFIG_LINK_KEY                 4

#define CONFIG_DEV_TYPE                 5
#define CONFIG_ADDR_TYPE                6
#define CONFIG_LE_KEY_PENC              7
#define CONFIG_LE_KEY_PID               8
#define CONFIG_LE_KEY_PCSRK             9
#define CONFIG_LE_KEY_LENC              10
#define CONFIG_LE_KEY_LID               11
#define CONFIG_LE_KEY_LCSRK             12

#define CONFIG_LE_LOCAL_KEY_IR          13
#define CONFIG_LE_LOCAL_KEY_IRK         14
#define CONFIG_LE_LOCAL_KEY_DHK         15
#define CONFIG_LE_LOCAL_KEY_ER          16
typedef uint8_t config_type_t;

#define CONFIG_SECTONS_MASK             (1 << CONFIG_SECTONS)

#define CONFIG_DEV_CLASS_MASK           (1 << CONFIG_DEV_CLASS)
#define CONFIG_PIN_LENGTH_MASK          (1 << CONFIG_PIN_LENGTH)
#define CONFIG_LINK_KEY_TYPE_MASK       (1 << CONFIG_LINK_KEY_TYPE)
#define CONFIG_LINK_KEY_MASK            (1 << CONFIG_LINK_KEY)

#define CONFIG_DEV_TYPE_MASK            (1 << CONFIG_DEV_TYPE)
#define CONFIG_ADDR_TYPE_MASK           (1 << CONFIG_ADDR_TYPE)
#define CONFIG_LE_KEY_PENC_MASK         (1 << CONFIG_LE_KEY_PENC)
#define CONFIG_LE_KEY_PID_MASK          (1 << CONFIG_LE_KEY_PID)
#define CONFIG_LE_KEY_PCSRK_MASK        (1 << CONFIG_LE_KEY_PCSRK)
#define CONFIG_LE_KEY_LENC_MASK         (1 << CONFIG_LE_KEY_LENC)
#define CONFIG_LE_KEY_LID_MASK          (1 << CONFIG_LE_KEY_LID)
#define CONFIG_LE_KEY_LCSRK_MASK        (1 << CONFIG_LE_KEY_LCSRK)

#define CONFIG_LE_LOCAL_KEY_IR_MASK     (1 << CONFIG_LE_LOCAL_KEY_IR)
#define CONFIG_LE_LOCAL_KEY_IRK_MASK    (1 << CONFIG_LE_LOCAL_KEY_IRK)
#define CONFIG_LE_LOCAL_KEY_DHK_MASK    (1 << CONFIG_LE_LOCAL_KEY_DHK)
#define CONFIG_LE_LOCAL_KEY_ER_MASK     (1 << CONFIG_LE_LOCAL_KEY_ER)
typedef uint32_t config_mask_t;


#define CONFIG_LOCAL_ADAPTER_STR           "Adapter"

// The default section name to use if a key/value pair is not defined within
// a section.
#define CONFIG_DEFAULT_SECTION "Global"

typedef struct {
    char *name;
    config_mask_t bit_mask;
} section_node_t;

typedef struct {
    config_type_t config_type;
    uint16_t length;
    uint8_t value[];
} __attribute__((packed)) entry_t;

typedef struct {
    config_mask_t bit_mask;
    uint16_t length;
    uint8_t value[];     //entry_t
} __attribute__((packed)) section_t;

typedef struct {
    list_t *sections;
}config_t;

typedef struct config_section_node_t config_section_node_t;

// Creates a new config object with no entries (i.e. not backed by a file).
// This function returns a config object or NULL on error. Clients must call
// |config_free| on the returned handle when it is no longer required.
config_t *config_new_empty(void);

// Loads the specified file and returns a handle to the config file. If there
// was a problem loading the file or allocating memory, this function returns
// NULL. Clients must call |config_free| on the returned handle when it is no
// longer required. |filename| must not be NULL and must point to a readable
// file on the filesystem.
config_t *config_new(const char *filename);

// Frees resources associated with the config file. No further operations may
// be performed on the |config| object after calling this function. |config|
// may be NULL.
void config_free(config_t *config);

// Returns true if the config file contains a section named |section|. If
// the section has no key/value pairs in it, this function will return false.
// |config| and |section_name| must not be NULL.
bool config_has_section(const config_t *config, const char *section_name);

// Returns true if the config file has a key which type is |key| under |section|.
// Returns false otherwise. |config|, |section_name| must not be NULL.
bool config_has_key(const config_t *config, const char *section_name, config_type_t key);

// Return true if the config file contains a key which type is |key| and the buffer |value| is enough to store it.
// Returns false otherwise. |config|, |section_name| must not be NULL.
bool config_get(const config_t *config, const char *section_name, const config_type_t key, void *value,  uint16_t *length);

// Sets a value for the |key| in |section|. If |key| or |section| do
// not already exist, this function creates them. |config|, |section| and
// |value| must not be NULL.
void config_set(config_t *config, const char *section_name, const config_type_t key, void *value, uint16_t length);

// Removes |section| from the |config| (and, as a result, all keys in the section).
// Returns true if |section| was found and removed from |config|, false otherwise.
// Neither |config| nor |section| may be NULL.
bool config_remove_section(config_t *config, const char *section);

// Removes one specific |key| residing in |section| of the |config|. Returns true
// if the section and key were found and the key was removed, false otherwise.
// None of |config| or |section| may be NULL.
bool config_remove_key(config_t *config, const char *section_name, const config_type_t key);

// Returns an iterator to the first section in the config file. If there are no
// sections, the iterator will equal the return value of |config_section_end|.
// The returned pointer must be treated as an opaque handle and must not be freed.
// The iterator is invalidated on any config mutating operation. |config| may not
// be NULL.
const config_section_node_t *config_section_begin(const config_t *config);

// Returns an iterator to one past the last section in the config file. It does not
// represent a valid section, but can be used to determine if all sections have been
// iterated over. The returned pointer must be treated as an opaque handle and must
// not be freed and must not be iterated on (must not call |config_section_next| on
// it). |config| may not be NULL.
const config_section_node_t *config_section_end(const config_t *config);

// Moves |iter| to the next section. If there are no more sections, |iter| will
// equal the value of |config_section_end|. |iter| may not be NULL and must be
// a pointer returned by either |config_section_begin| or |config_section_next|.
const config_section_node_t *config_section_next(const config_section_node_t *iter);

// Returns the name of the section referred to by |iter|. The returned pointer is
// owned by the config module and must not be freed by the caller. The pointer will
// remain valid until |config_free| is called. |iter| may not be NULL and must not
// equal the value returned by |config_section_end|.
const char *config_section_name(const config_section_node_t *iter);

// Saves |config| to a file given by |filename|. Note that this could be a destructive
// operation: if |filename| already exists, it will be overwritten. The config
// module does not preserve comments or formatting so if a config file was opened
// with |config_new| and subsequently overwritten with |config_save|, all comments
// and special formatting in the original file will be lost. Neither |config| nor
// |filename| may be NULL.
bool config_save(const config_t *config, const char *filename);

#endif /* #ifndef __CONFIG_H__ */
