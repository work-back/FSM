/*
* Copyright (c) 2020 WAYCLOUDS, Inc.2019 - langyj, langyanjun@wayclouds.com
* All Rights Reserved.
* WAYCLOUDS Confidential and Proprietary.
*
*/

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/eventfd.h>
#include <inttypes.h>

#include <stdarg.h>

//#include <wmi_sys.h>
//#include <wmi.h>
#include "fsm.h"

#define WMI_FSM_LOCK(fsm) pthread_mutex_lock(&((fsm)->mutex))
#define WMI_FSM_UNLOCK(fsm) pthread_mutex_unlock(&((fsm)->mutex))

#define __WMI_FSM_LOG(fmt, ...)

#include <iostream>

void WMI_FSM_LOG(wmi_fsm *fsm, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

//    std::cout << "[" << function << "][" << line << "]";
    vprintf(fmt, args);
    std::cout << std::endl;

    va_end(args);
}

void WMI_fsm_state_time_out_set(wmi_fsm *fsm, int state, WMI_TIME_MS new_out_ms)
{
    WMI_TIME_MS now_ms = 0;
    wmi_fsm_state_info *state_info = fsm->state_info;
    wmi_fsm_state_info *state_info_cur = &(state_info[state]);

    now_ms = WMI_sys_get_boot_time_ms();
    if (!now_ms) {
        return;
    }

    state_info_cur->time_start_ms = now_ms;
    state_info_cur->out_ms = new_out_ms;

    return;
}

void WMI_fsm_curr_state_time_out_set(wmi_fsm *fsm, WMI_TIME_MS new_out_ms)
{
    WMI_fsm_state_time_out_set(fsm, fsm->cur_state, new_out_ms);

    return;
}

void WMI_fsm_time_out_reset(wmi_fsm *fsm)
{
    WMI_TIME_MS now_ms = 0;
    wmi_fsm_state_info *state_info = fsm->state_info;

    now_ms = WMI_sys_get_boot_time_ms();
    if (!now_ms) {
        return;
    }

    state_info[fsm->cur_state].time_start_ms = now_ms;

    return;
}

void WMI_fsm_time_out_check(wmi_fsm *fsm)
{
    WMI_TIME_MS now_ms = 0;

    if (!fsm) {
        return;
    }

    /* all fsm action is do at fsm-thread so no need lock now */
    //WMI_FSM_LOCK

    wmi_fsm_state_info *state_info = fsm->state_info;
    wmi_fsm_state_info *state_info_cur = &(state_info[fsm->cur_state]);

    if (!state_info_cur->time_out || !state_info_cur->out_ms) {
        //WMI_FSM_LOG(fsm, "time_out:%"PRIu64 ", out_msg:%"PRIu64 "", state_info_cur->time_out, state_info_cur->out_ms);
        goto exit;
    }

    now_ms = WMI_sys_get_boot_time_ms();
    if (!now_ms) {
        WMI_FSM_LOG(fsm, "get boot time ms error");
        goto exit;
    }

    WMI_FSM_LOG(fsm, "current state %s, TIME OUT (%"PRIu64 ") < (%"PRIu64 ")", state_info[fsm->cur_state].name,
            state_info_cur->time_start_ms + state_info_cur->out_ms, now_ms);
    if (state_info_cur->time_start_ms + state_info_cur->out_ms < now_ms) {
        WMI_FSM_LOG(fsm, "current state %s, TIME OUT (%"PRIu64 ") + (%"PRIu64 ") < (%"PRIu64 ")", state_info[fsm->cur_state].name,
                state_info_cur->time_start_ms, state_info_cur->out_ms, now_ms);
        state_info_cur->time_out(fsm);
    }
exit:
    //WMI_FSM_UNLOCK

    return;
}

void WMI_fsm_dispatch_event(wmi_fsm *fsm, int event, int event_data_len, void *event_data)
{
    const char *event_name = NULL;
    char* cur_state_name;
    wmi_fsm_state_info *state_info = fsm->state_info;

    cur_state_name = state_info[fsm->cur_state].name;

    if (fsm->event_names) {
        if(event < fsm->num_event_names) {
            event_name = fsm->event_names[event];
        } else {
            event_name = "UNKNOW_EVENT";
        }
    } else {
        event_name = "UNSHOW_EVENT";
    }
    WMI_FSM_LOG(fsm, "current state %s, event %s ++++++++", cur_state_name, event_name);

    if (state_info[fsm->cur_state].event) {
        state_info[fsm->cur_state].event(fsm, event, event_data_len, event_data);
    }

    WMI_FSM_LOG(fsm, "current state %s, event %s --------", cur_state_name, event_name);

    return;
}

void WMI_fsm_transition_to(wmi_fsm *fsm, int state)
{
    WMI_TIME_MS now_ms = 0;
    wmi_fsm_state_info *state_info = fsm->state_info;

    if (fsm->in_state_transition) {
        WMI_FSM_LOG(fsm, "in state transition. skip ...");
        //TODO: or just lock?
        return;
    }

    if (state > fsm->num_states) {
        return;
    }

    now_ms = WMI_sys_get_boot_time_ms();
    if (!now_ms) {
        return;
    }

    fsm->in_state_transition = 1;

    fsm->next_state = state;
    WMI_FSM_LOG(fsm, "fsm state transition %s ===>> exit()",
            state_info[fsm->cur_state].name);

    if (state_info[fsm->cur_state].exit) {
        state_info[fsm->cur_state].exit(fsm);
    }

    WMI_FSM_LOG(fsm, "fsm state transition %s ===>> %s",
            state_info[fsm->cur_state].name, state_info[state].name);

    state_info[state].time_start_ms = now_ms;

    fsm->pre_state = fsm->cur_state;
    fsm->next_state = WMI_FSM_STATE_INVALID;
    fsm->cur_state = state;
    if (state_info[state].entry) {
        state_info[state].entry(fsm);
    }

    fsm->in_state_transition = 0;

    return ;
}

wmi_fsm *WMI_fsm_create(const char *name,
                        void *ctx,
                        int init_state,
                        wmi_fsm_state_info *state_info,
                        int num_states,
                        wmi_fsm_debug_print_cb_t debug_print,
                        const char **event_names,
                        int num_event_names)
{
    wmi_fsm_state_info *state_info_dup = NULL;
    int state_info_dup_size = 0;

    wmi_fsm *fsm = (wmi_fsm *)malloc(sizeof(wmi_fsm));
    if (!fsm) {
        WMI_FSM_LOG(fsm, "oom");
        return NULL;
    }
    memset(fsm, 0, sizeof(wmi_fsm));

    state_info_dup_size = sizeof(wmi_fsm_state_info) * num_states;
    state_info_dup = (wmi_fsm_state_info *)malloc(state_info_dup_size);
    if (!state_info_dup) {
        WMI_FSM_LOG(fsm, "oom");
        goto error;
    }
    memcpy(state_info_dup, state_info, state_info_dup_size);

    snprintf(fsm->name, sizeof(fsm->name), "%s", name);
    fsm->ctx             = ctx;
    fsm->num_states      = num_states;
    fsm->cur_state       = init_state;
    fsm->state_info      = state_info_dup;
    fsm->debug_print     = debug_print;
    fsm->event_names     = event_names;
    fsm->num_event_names = num_event_names;
    fsm->debug_print     = debug_print;
    WMI_fsm_time_out_reset(fsm);

    pthread_mutex_init(&(fsm->mutex), NULL);

    return fsm;

error:
    if (fsm) {
        free(fsm);
    }

    return NULL;
}

void WMI_fsm_destroy(wmi_fsm *fsm)
{
    if (fsm) {
        if (fsm->state_info) {
            free(fsm->state_info);
            fsm->state_info = NULL;
        }
        pthread_mutex_destroy(&(fsm->mutex));
        free(fsm);
        fsm = NULL;
    }

    return;
}

