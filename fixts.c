#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#define CLOCKSOURCE0_PATH "/sys/devices/system/clocksource/clocksource0"

const char* systemd_service_content = "[Unit]\n"
"Description=Change Timesource in boot-time.\n"
"Wants=network-online.target\n\n"
"[Service]\n"
"Type=simple\n"
"ExecStart=/sbin/stellait-fixts\n\n"
"[Install]\n"
"WantedBy=multi-user.target";

const char* sysvinit_service_content = "#!/bin/sh\n"
"### BEGIN INIT INFO\n"
"# Provides:          fixts\n"
"# Required-Start:    $local_fs\n"
"# Required-Stop:     \n"
"# Default-Start:     S\n"
"# Default-Stop:      \n"
"# Description:       Change Timesource in boot-time.\n"
"### END INIT INFO\n"
". /lib/lsb/init-functions\n"
"case \"$1\" in\n"
"  start)\n"
"        log_action_begin_msg \"Changing Timesource to TSC...\"\n"
"        echo tsc>/sys/devices/system/clocksource/clocksource0/current_clocksource\n"
"        log_cation_end_msg \"$?\"\n"
"        ;;\n"
"  stop|restart|force-reload|status)\n"
"        ;;\n  *)\n        echo \"Usage: $SCRIPTNAME start\" >&2\n        exit 3\n"
"        ;;\nesac\n";


typedef char BYTE;
BYTE isFixNeeded() {
    int startTime, endTime;
    startTime = time(NULL);
    struct timeval tv;
    for(int i=0;i<10000000;i++) {
        gettimeofday(&tv, NULL);
    }
    endTime = time(NULL);
    return (endTime - startTime) > 2; // 2초 이내가 정상
}

void fatal(const char* reason) {
    fprintf(stderr, "Fatal: %s\nPlease consult your system administrator\n", reason);
    exit(1);
}

int main(int argc, char *argv[]) {
    BYTE acstr[256], bccstr[16], grubline[2048], gclc = 0, gclb = 0;
    fprintf(stderr, "Xen Timesource Fix Utility\n"
                    "(c) 2023 Stella IT Inc.\n\n");
    
    if(getuid() && setuid(0)) fatal("Should be run in root (e.g. sudo)");
    FILE* available_clocksource = fopen(CLOCKSOURCE0_PATH "/available_clocksource", "r");
    if(!available_clocksource) fatal("clocksource0 unavailable");
    fgets(acstr, 255, available_clocksource);
    fclose(available_clocksource);
    if(!strstr(acstr, "tsc")) fatal("clocksource \"tsc\" unavailable");
    
    FILE* current_clocksource = fopen(CLOCKSOURCE0_PATH "/current_clocksource", "r");
    if(!current_clocksource) fatal("clocksource0 unavailable");
    fgets(bccstr, 15, current_clocksource);
    
    printf("%s: Diagnosing, It takes some time... ", argv[0]);
    if(!strcmp(bccstr, "tsc") || !isFixNeeded()) {
        fprintf(stdout, "[FAIL]\n");
        fprintf(stderr, "This system is already fixed or enough fast to fix, quitting...\n");
        fclose(current_clocksource);
        return 0;
    }
    freopen(CLOCKSOURCE0_PATH "/current_clocksource", "w", current_clocksource);
    fprintf(current_clocksource, "tsc");
    fclose(current_clocksource);
    if(isFixNeeded()) {
        printf("[FAIL]\n");
        fatal("temporary fix failed; patch won't work on this system");
    }
    fprintf(stdout, "[ OK ]\n");
    printf("%s: Trying to fixing kernel cmdline using GRUB... ", argv[0]);
    FILE* etc_default_grub = fopen("/etc/default/grub", "r");
    if(etc_default_grub) {
        while(fgets(grubline, 2047, etc_default_grub) != NULL) {
            strstr(grubline, "GRUB_CMDLINE_LINUX=") && gclc++;
            strstr(grubline, "GRUB_CMDLINE_LINUX=\"\"") && gclb++;
        }
        if(gclc*gclb == 1) {
            freopen("/etc/default/grub", "a+", etc_default_grub);
            fprintf(etc_default_grub, "# Change timesource to GRUB\nGRUB_CMDLINE_LINUX=\"clocksource=tsc tsc=reliable hongfarm=babo are_you_editing_etc_default_grub=see_from_last_line\"");
            printf("[ OK ]\n");
            fclose(etc_default_grub);
            fprintf(stderr, "Fixed! Thanks for using Stella IT!\n");
            return 0;
        }
        fclose(etc_default_grub);
    }
    printf("[SKIP]\n");
    printf("%s: Writing script to fix timesource... ", argv[0]);
    FILE* sbin_stellait_fixts = fopen("/sbin/stellait-fixts", "w");
    if(!sbin_stellait_fixts) fatal("/sbin unavailable"); else printf("/sbin/stellait-fixts ");
    fprintf(sbin_stellait_fixts, "#!/bin/bash\necho tsc>/sys/devices/system/clocksource/clocksource0/current_clocksource");
    fclose(sbin_stellait_fixts);
    printf("[ OK ]\n");
    if(chmod("/sbin/stellait-fixts", 0700)) fatal("chmod() on /sbin/stellait-fixts failed");
    printf("%s: Applying fix to systemd...", argv[0]);
    FILE* lib_systemd_system_fixts_service = fopen("/lib/systemd/system/fixts.service", "w");
    if(lib_systemd_system_fixts_service) {
        fprintf(lib_systemd_system_fixts_service, systemd_service_content);
        fclose(lib_systemd_system_fixts_service);
        system("systemctl daemon-reload");
        system("systemctl enable --now fixts.service");
        printf("[ OK ]\n");
        fprintf(stderr, "Fixed! Thanks for using Stella IT!\n");
        return 0;
    }
    printf("[SKIP]\n");
    printf("%s: Applying fix in alternative mode(sysvinit)...", argv[0]);
    FILE* etc_init_d_fixts = fopen("/etc/init.d/fixts", "w");
    if(!etc_init_d_fixts) fatal("opening /etc/init.d/fixts failed!");
    fprintf(etc_init_d_fixts, sysvinit_service_content);
    fclose(etc_init_d_fixts);
    system("update-rc.d /etc/init.d/fixts defaults");
    printf("[ OK ]\n");
    fprintf(stderr, "Fixed! Thanks for using Stella IT!\n");
    return 0;
}