/******************************************************************************
 *                                                                            *
 *                galandae copyright (c) 2016 Chris Marsh                     *
 *               <https://github.com/chris-marsh/gcalendar                    *
 *                                                                            *
 * this program is free software: you can redistribute it and/or modify it    *
 * under the terms of the gnu general public license as published by the      *
 * free software foundation, either version 3 of the license, or any later    *
 * version.                                                                   *
 *                                                                            *
 * this program is distributed in the hope that it will be useful, but        *
 * without any warranty; without even the implied warranty of merchantability *
 * or fitness for a particular purpose.  see the gnu general public license   *
 * at <http://www.gnu.org/licenses/> for more details.                        *
 *                                                                            *
 ******************************************************************************/


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
