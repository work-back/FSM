#include <stdio.h>       
#include <unistd.h>      
#include <stdlib.h>      
#include <string.h>      
#include <time.h>        
#include <sys/time.h>    
#include <sys/resource.h>

#include <iostream>

#include "fsm.h"

WMI_TIME_MS WMI_sys_get_boot_time_ms(void)
{
    WMI_TIME_MS now_ms = 0;
    struct timespec now = {0};
    if (clock_gettime(CLOCK_MONOTONIC, &now) < 0) {
        std::cout << "get boot time failed." << std::endl;
        return 0;
    }

    now_ms = now.tv_sec * 1000 + now.tv_nsec / (1000 * 1000);

    return now_ms;
}
