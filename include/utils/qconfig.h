/**
 * @file qconfig.h
 * @author qiufuyu (3038742090@qq.com)
 * @brief A tool to load config file
 * @version 0.1
 * @date 2022-08-16
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef _H_QCONFIG
#define _H_QCONFIG
#include"types.h"
#include"utils/qhash.h"

/* public data structures */
#define QCONFIG_STR 0
#define QCONFIG_NUMBER 1
#define QCONFIG_POINTER 2
/**
 * @brief NOTICE
 * WE SUPPORT FARRRRRRRRR MOR THAN THE COMMON INI FILE!
 */
typedef struct qconfig_value
{
    union 
    {
        char *pure_str;
        uint32_t number;
        struct qconfig_value *pointer;
    };
    uint8_t type;
}qconfig_value_t;

struct config
{
	char *key;
	char *value;
};

struct section
{
	char *name;
	int num_configs;
	struct config **configs;
};

struct ini
{
	int num_sections;
	struct section **sections;	
};

struct read_ini;                /* private */

/* public API */

/* This will parse a given ini file contained in filename and return
 * the result as an allocated ini structure. The read_ini structure
 * should be declared and then set to NULL beforehand (this is to make
 * the reader thread-safe. */
struct ini* read_ini(struct read_ini **read_inip, char *filename,char *data,uint32_t len);

/* Retrieves a value from a section/key combination and returns that
 * as a string. Note the returned value should not be freed. NULL is
 * returned if the section/key combination can not be found. */
char *ini_get_value(struct ini* ini, char *section, char *key);

/* Pretty print a given ini structure. */
void ini_pp(struct ini* ini);

/* Free memory associated with an ini structure. */
void destroy_ini(struct ini* ini);

qconfig_value_t* qconfig_get_value(struct ini* ini,char *key);



/* Free memory associated with a read_ini structure. */
void cleanup_readini(struct read_ini* read_ini);



#endif