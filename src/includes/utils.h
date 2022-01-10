#ifndef _UTILS_H
#define _UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int validate_number(const char *);
int validate_ipv4(const char *);
int validate_port(const char *);
int validate_username(const char *);

#endif