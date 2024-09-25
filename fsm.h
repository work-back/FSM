#ifndef __WMI_FSM__
#define __WMI_FSM__

#include <pthread.h>

//#include <wmi_def.h>
#include <stdint.h>
typedef uint64_t WMI_TIME_MS;
#define WMI_LEN_64    (64)

#define WMI_FSM_MAX_NAME WMI_LEN_64
#define WMI_FSM_STATE_INVALID (0xFFFFFFFF)

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))


#define WMI_LOG(fmt, ...) \
    printf("[%s][%d]"fmt"\n", __func__, __LINE__, ##__VA_ARGS__);
#define WMI_ERR(fmt, ...) \
    printf("[%s][%d]"fmt"\n", __func__, __LINE__, ##__VA_ARGS__);
#define WMI_WARN(fmt, ...) \
    printf("\033[1;33m"fmt"\033[0m\n", ##__VA_ARGS__);

WMI_TIME_MS WMI_sys_get_boot_time_ms(void);

typedef void (*wmi_fsm_entry_cb_t)(void *fsm);
typedef void (*wmi_fsm_exit_cb_t)(void *fsm);
typedef void (*wmi_fsm_event_cb_t)(void *fsm, int event, int event_data_len, void *event_data);
typedef void (*wmi_fsm_debug_print_cb_t)(void *fsm, const char *fmt, ...);
typedef void (*wmi_fsm_time_out_cb_t)(void *fsm);

typedef struct wmi_fsm_state_info_tag
{
    int state;
    char *name;
    wmi_fsm_entry_cb_t entry;
    wmi_fsm_exit_cb_t  exit;
    wmi_fsm_event_cb_t event;
    wmi_fsm_time_out_cb_t time_out;
    WMI_TIME_MS time_start_ms;
    WMI_TIME_MS out_ms;
}wmi_fsm_state_info;

typedef struct wmi_fsm_tag
{
    char name[WMI_FSM_MAX_NAME];
    void *ctx;

    int cur_state;
    int pre_state;
    int next_state;
    int num_states;
    wmi_fsm_state_info *state_info; /* a copy of wmi_fsm_state_info */

    const char **event_names;
    unsigned int num_event_names;
    wmi_fsm_debug_print_cb_t debug_print;

    int in_state_transition;
    pthread_mutex_t mutex;
}wmi_fsm;

#define WMI_FSM_STATE_NEXT(fsm) ((fsm) ? ((wmi_fsm *)fsm)->next_state : WMI_FSM_STATE_INVALID)

void WMI_fsm_state_time_out_set(wmi_fsm *fsm, int state, WMI_TIME_MS new_out_ms);
void WMI_fsm_curr_state_time_out_set(wmi_fsm *fsm, WMI_TIME_MS new_out_ms);
void WMI_fsm_time_out_reset(wmi_fsm *fsm);
void WMI_fsm_time_out_check(wmi_fsm *fsm);
wmi_fsm *WMI_fsm_create(const char *name, 
                        void *ctx, 
                        int init_state, 
                        wmi_fsm_state_info *state_info, 
                        int num_states, 
                        wmi_fsm_debug_print_cb_t debug_print,
                        const char **event_names,
                        int num_event_names);
void WMI_fsm_transition_to(wmi_fsm *fsm, int state);
void WMI_fsm_dispatch_event(wmi_fsm *fsm, int event, int event_data_len, void *event_data);
void WMI_fsm_destroy(wmi_fsm *fsm);

#endif //__WMI_FSM__
