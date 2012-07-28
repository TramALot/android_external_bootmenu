/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * Changes @ June 2012
 * Project lense (@whirleyes) - Fork Android recovery source code for spyder bootmenu
 *
 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <sys/wait.h>
#include <sys/limits.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <dirent.h>
#include <signal.h>
#include <cutils/properties.h>
#include "common.h"

int exec_and_wait(char** argp) {
    pid_t pid;
    sig_t intsave, quitsave;
    sigset_t mask, omask;
    int pstat;
    
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, &omask);
    switch (pid = vfork()) {
    case -1: /* error */
        sigprocmask(SIG_SETMASK, &omask, NULL);
        return(-1);
    case 0: /* child */
        sigprocmask(SIG_SETMASK, &omask, NULL);
        execve(argp[0], argp, environ);
    	_exit(127);
	}

    intsave = (sig_t) bsd_signal(SIGINT, SIG_IGN);
    quitsave = (sig_t) bsd_signal(SIGQUIT, SIG_IGN);
    pid = waitpid(pid, (int *)&pstat, 0);
    sigprocmask(SIG_SETMASK, &omask, NULL);
    (void)bsd_signal(SIGINT, intsave);
    (void)bsd_signal(SIGQUIT, quitsave);
    return (pid == -1 ? -1 : pstat);
}

void write_sys(const char* file, int value) {
	FILE* f = fopen(file, "w");
	if (f != NULL) {
    	fprintf(f, "%d", value);
    	fclose(f);
  	}
}

void led(const char* color, int value) {
	char led_path[PATH_MAX];
	sprintf(led_path, "/sys/class/leds/%s/brightness", color);
	write_sys(led_path, value);
}

void amoled(int value) { write_sys("/sys/class/backlight/430_540_960_amoled_bl/brightness", value); }
void vibrate(int value) { write_sys("/sys/class/timed_output/vibrator/enable", value); }
void boot_stock() { boot("stock", stock_adbd, stock_init); }
void boot_second() { boot("second", second_adbd, second_init); }
void boot_recovery() { boot("recovery", 0, 0); }
void boot(const char* script, int adbd, int init) {
	char scripts[PATH_MAX];
	sprintf(scripts, "/preinstall/bootmenu/script/boot_%s.sh %d %d", script, adbd, init);
	char* run_args[] = { "/preinstall/bootmenu/binary/busybox", "sh", "-c", scripts, NULL };
	exec_and_wait(run_args);
}
