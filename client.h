#ifndef _CLIENT_H
#define _CLIENT_H

#include "network.h"

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void conect_to_server(const char *, const uint16_t, const char *);

#endif