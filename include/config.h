/*
 * Project: Galendae Popup Calendar
 * URL:     https://github.com/chris-marsh/galendae
 * License: The MIT License
 */


#ifndef CONFIG_H
#define CONFIG_H


#define CONFIG_NAME APP_NAME "rc"


/* Option pairs */
typedef struct {
    char *key;
    char *value;
} Option;


/* Config stack of options */
typedef struct Config {
    Option option;
    struct Config *next;
} Config;


int push_option(Config **head, const char *key, const char *value);
Option pop_option(Config **head);
void free_option(Option option);
Config *read_config_file(const char *filename);

#endif
