#include <stdio.h>       
#include <unistd.h>      
#include <stdlib.h>      
#include <string.h>      

#include "fsm.h"

typedef enum awmd_fsm_state_tag
{
    AWMD_FSM_STATE_INIT = 0,
    AWMD_FSM_STATE_NET_UP,
    AWMD_FSM_STATE_MQTT_UP,
    AWMD_FSM_STATE_DEAD,

    AWMD_FSM_STATE_MAX,
}awmd_fsm_state;

typedef enum AWMD_FSM_event_tag
{
    AWMD_FSM_EVENT_NET_UP,
    AWMD_FSM_EVENT_MQTT_UP,
    AWMD_FSM_EVENT_NET_DOWN,
    AWMD_FSM_EVENT_MQTT_DOWN,
    AWMD_FSM_EVENT_TIME_OUT,
    AWMD_FSM_EVENT_KILL,

    AWMD_FSM_EVENT_NET_CHANGE,

    AWMD_FSM_EVENT_MAX,
}AWMD_FSM_event;


const char *g_awmd_fsm_event_name[AWMD_FSM_EVENT_MAX] = {
    [AWMD_FSM_EVENT_NET_UP]           = "NET_UP",
    [AWMD_FSM_EVENT_MQTT_UP]          = "MQTT_UP",
    [AWMD_FSM_EVENT_NET_DOWN]         = "NET_DOWN",
    [AWMD_FSM_EVENT_MQTT_DOWN]        = "MQTT_DOWN",
    [AWMD_FSM_EVENT_TIME_OUT]         = "TIME_OUT",
    [AWMD_FSM_EVENT_KILL]             = "KILL",
};


void awmd_fsm_entry_def(void *fsm)
{
    WMI_LOG("entry");
    return;
}

void awmd_fsm_exit_def(void *fsm)
{
    return;
}

void awmd_fsm_event_def(void *fsm, int event, int event_data_len, void *event_data)
{
    return;
}

void awmd_fsm_idel_event(void *fsm, int event, int event_data_len, void *event_data)
{
    switch (event) {
        case AWMD_FSM_EVENT_NET_UP:
            WMI_fsm_transition_to(fsm, AWMD_FSM_STATE_NET_UP);
            break;
    }

    return;
}

void awmd_fsm_net_up_entry(void *fsm)
{
    return;
}

void awmd_fsm_net_up_event(void *fsm, int event, int event_data_len, void *event_data)
{
    return;
}

void awmd_fsm_mqtt_up_entry(void *fsm)
{
    return;
}

void awmd_fsm_mqtt_up_exit(void *fsm)
{
    return;
}

void awmd_fsm_mqtt_up_event(void *fsm, int event, int event_data_len, void *event_data)
{
    return;
}

static wmi_fsm_state_info g_awmd_fsm_state_infos[] = {
    {
        .name  = "INIT",
        .state = AWMD_FSM_STATE_INIT,
        .entry = awmd_fsm_entry_def,
        .exit  = awmd_fsm_exit_def,
        .event = awmd_fsm_idel_event,
        //.time_out = awmd_fsm_idle_time_out,
        //.out_ms = AWMD_FSM_IDEL_TIME_OUT_MS,
    }, {
        .name = "NET_UP",
        .state = AWMD_FSM_STATE_NET_UP,
        .entry = awmd_fsm_net_up_entry,
        .exit  = awmd_fsm_exit_def,
        .event = awmd_fsm_net_up_event,
        //.time_out = awmd_fsm_net_up_time_out,
        //.out_ms = AWMD_FSM_NET_UP_TIME_OUT_MS,
    }, {
        .name = "MQTT_UP(AC_RUNNING)",
        .state = AWMD_FSM_STATE_MQTT_UP,
        .entry = awmd_fsm_mqtt_up_entry,
        .exit  = awmd_fsm_mqtt_up_exit,
        .event = awmd_fsm_mqtt_up_event,
    },
};

wmi_fsm *g_fsm;

int main(void)
{
    int ctx = 100;

    g_fsm = WMI_fsm_create("awmd_fsm",
            &ctx,
            AWMD_FSM_STATE_INIT,
            g_awmd_fsm_state_infos,
            ARRAY_SIZE(g_awmd_fsm_state_infos),
            NULL, //awmd_fsm_debug_print,
            g_awmd_fsm_event_name,
            ARRAY_SIZE(g_awmd_fsm_event_name));

            
    WMI_fsm_dispatch_event(g_fsm, AWMD_FSM_EVENT_NET_UP, 0, NULL);

    return 0;
}

