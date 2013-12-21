/*
 * inet.h
 *
 *  Created on: Dec 6, 2013
 *      Author: root
 */

#ifndef INET_H_
#define INET_H_
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/time.h>
#define MAX_LINE 1<<10
extern int init_cli(void);


#endif /* INET_H_ */
