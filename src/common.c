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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/wait.h>
#include "common.h"


/*
 * Implementation of string duplicate, ie allocate, copy & return
*/
char *strdup(const char *source_str)
{
    char *dest_str = malloc (strlen(source_str) + 1);   // Allocate memory
    if (dest_str != NULL) strcpy(dest_str,source_str);  // Copy string if okay
    return dest_str;                                    // Return new memory
}


int strfcpy(const char *source_str, char **dest_str) {
    if (*dest_str) {
        free(*dest_str);
        *dest_str = NULL;
    }
    *dest_str = strdup(source_str);
    if (*dest_str == NULL)
        return 1;
    else
        return 0;
}


/*
 * Remove leading and trailing spaces (Ascii 32). As the result will be less
 * than or equal in size of the input, its safe to edit the string inplace
*/
int trim_spaces(char *source_str)
{
    char *dest_str = source_str;
    int length = strlen(dest_str);

    /* zero trailing spaces */
    while (dest_str[length - 1] == SPACE)
        dest_str[--length] = 0;

    /* Increment pointer skipping leading spaces */
    while (*dest_str && *dest_str == SPACE)
        ++dest_str, --length;

    /* Copy the trimmed string back to the input string */
    memmove(source_str, dest_str, length + 1);

    return length;
}


/*
 * Check the $HOME variable is valid and return it, otherwise use
 * getpwuid to return data for the user (getuid)
*/
char *user_home_dir()
{
	char *s = getenv("HOME");
	if (s)
		return s;
	struct passwd *pw = getpwuid(getuid());
	if (!pw)
		return NULL;
	return pw->pw_dir;
}


/*
 * Append "/.config" to the the users home dir and return it.
 * TODO: find a better way getting an actual XDG specified dir?
*/
char *user_config_dir()
{
    char *home = user_home_dir();
    if (home == NULL)
        return NULL;
    char *config = malloc(strlen(home)+9);
    strcpy(config, home);
    strcat(config, "/.config");
    return config;
}
