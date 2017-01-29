/*
 * Project: Galendae Popup Calendar
 * URL:     https://github.com/chris-marsh/galendae
 * License: The MIT License
 */


#ifndef COMMON_H
#define COMMON_H

#define APP_NAME "galendae"

#ifndef VERSION
#define VERSION "unknown"
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define SPACE 32

char *strdup (const char *source_str);
int strfcpy(const char *source_str, char **dest_str);
int trim_spaces(char *source_str);
char *user_home_dir();
char *user_config_dir();

#endif
