#ifndef _CLIENT_H
#define _CLIENT_H

#include "network.h"

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void start_client(const char *address, const uint16_t port, const char *username);

#endif