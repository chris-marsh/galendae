/*
 * Project: Galendae Popup Calendar
 * URL:     https://github.com/chris-marsh/galendae
 * License: The MIT License
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "config.h"
#include "common.h"


/*
 * Allocate and add an option to the config stack
 */
int push_option(Config **head, const char *key, const char *value)
{
    Config *stack = malloc(sizeof(*stack));
    if (stack) {
        Option option = { strdup(key), strdup(value) };
        stack->option = option;
        stack->next = *head;

        *head = stack;
        return TRUE;
    }
    else {
        return FALSE;
    }
}


/*
 * Remove and return an option from the config stack 
 */
Option pop_option(Config **head)
{
    Option option = {0, 0};
    if (*head) {
        Config *prevConfig;
        prevConfig = *head;
        *head = (*head)->next;

        option = prevConfig->option;
        free(prevConfig);
    }
    return option;
}


/*
 * Convenience function to cleanup an option node
 */
void free_option(Option option)
{
    free(option.key);
    free(option.value);
}


/*
 *  Given a filename, check if it exists in the order of ...
 *          filename
 *          /home/user/.config/APP_NAME/filename
 *          /usr/share/galendae/config/filename
 *  If filename is not given or found, look for
 *          CONFIG_NAME
 *          /home/user/.config/APP_NAME/CONFIG_NAME
 *          /usr/share/APP_NAME/config/CONFIG_NAME
 */
char *expand_config_filename(const char *user_filename) {
    char *filename;

    if (user_filename != NULL)
        /* If user provided a filename then use it */
        filename = strdup(user_filename);
    else {
        /* otherwise use our default config filename */
        filename = strdup(CONFIG_NAME);
    }
    if (access(filename, R_OK) == 0) {
        /* if filename exists & is readable in the current directory then use it */
        return filename;
    } else {
        /* otherwise, get the users config directory and use our folder */
        char *filepath = user_config_dir();
        char *qual_filename = malloc(strlen(filepath)+strlen(APP_NAME)+strlen(filename)+3);
        sprintf(qual_filename, "%s%c%s%c%s", filepath, '/', APP_NAME, '/', filename);
        free(filepath);
        if (access(qual_filename, R_OK) == 0) {
            free(filename);
            return qual_filename;
        } else {
            /* last attempt, try /usr/share/APP_NAME/config */
            free(qual_filename);
            qual_filename = malloc(strlen(APP_NAME)+strlen(filename)+20);
            sprintf(qual_filename, "%s%s%s%s", "/usr/share/", APP_NAME, "/config/", filename);
            free(filename);
            return qual_filename;
        }
    }
    return NULL;
}


/*
 * Config file format;
 *      Comments start with a '#' or ';' on the first character of the line
 *      
 *      Empty lines are ignored
 *      A valid config line contains '=' seperating vale & key
 */
static int parse_line(const char *line, char **key, char **value)
{
    char *value_str;
    /*  Skip empty lines and comments */
    if ((line[0] == ';') || (line[0] == '#') || (line[0] == '\n'))
        return FALSE;
    /* Get pointer to position of '=' if present */
    if (!(value_str = strchr(line, '=')))
        return FALSE;

    /* Overwrite the leading '=' and inc pointer */
    value_str[0] = '\0';
    value_str++;
    *key = strdup(line);

    /* Overwrite '\n' with '\0' if '\n' present */
    char *temp_str;
    if ((temp_str = strchr(value_str, '\n')))
        temp_str[0] = '\0';
    *value = strdup(value_str);

    trim_spaces(*key);
    trim_spaces(*value);
    return TRUE;
}


/*
 * Read the config file and populate a config stack with 
 * option key/value pairs. Returns a pointer to the stack.
 * Max line length read is 256 characters
 */
Config *read_config_file(const char *filename)
{
    FILE *fp = NULL;
    char line[256];
    char *fn = NULL, *key, *value;
    Config *config = NULL;

    fn = expand_config_filename(filename);

    if( access(fn, F_OK ) != -1 ) {
        /* Return if cannot open the file */
        if ((fp = fopen(fn, "r")) != NULL) {

            /* Parse each line and store as key, value pair */
            while (fgets(line, sizeof(line), fp) != NULL) {
                if (parse_line(line, &key, &value)) {
                    push_option(&config, key, value);
                    free(key);
                    free(value);
                }
            }
        }
    } else {
        printf("Could not load config file: %s\n", fn);
    }
    if (fp) fclose(fp);
    if (fn) free(fn);
    return config;
}
