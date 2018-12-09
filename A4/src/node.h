#ifndef VS_NODE_H
#define VS_NODE_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "log.h"

#include "net/gcoap.h"
#include "kernel_types.h"
#include "random.h"

#include "msg.h"
#include "evtimer_msg.h"
#include "xtimer.h"

#include "elect.h"
#include "vector.h"
#include "avg.h"

#define MS_TO_US 1000

extern evtimer_msg_event_t interval_event;
extern evtimer_msg_event_t leader_timeout_event;
extern evtimer_msg_event_t leader_threshold_event;

typedef int state_t;

enum
{
	STATE_ADVERTISING,
	STATE_WAITING,
	STATE_COORD_SKIP,
	STATE_COORD_STARTUP,
	STATE_COORD_RUN,
	STATE_PART
};

enum
{
	TIMER_ADV = 0,
	TIMER_ELECT,
	TIMER_LIFE,
	NTIMERS
};

typedef struct
{
	kernel_pid_t this_thread;
	ipv6_addr_t my_ip, coord_ip;
	state_t state;
	xtimer_ticks64_t last_elec;
	xtimer_t timers[NTIMERS];
	vector_t clients;
	avg_t average;
} Node_t;

void Node_init(Node_t *);
void Node_delete(Node_t *);
void Node_elect_int(Node_t *);
void Node_elect_broadcast(Node_t *, const char *);
void Node_elect_done(Node_t *);
void Node_register(Node_t *, const char *);
void Node_addSensor(Node_t *, uint16_t);
void Node_alive(Node_t *);
void Node_dead(Node_t *);

#endif

