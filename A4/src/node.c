#include "node.h"

void Node_setState(Node_t *, state_t);
void Node_setCoord(Node_t *, const ipv6_addr_t *);
void Node_setTimer(Node_t *, int, uint, evtimer_msg_event_t *);
void Node_resetElection(Node_t *);
void Node_resetLife(Node_t *);
void Node_sendRequests(Node_t *);
void Node_deliverResult(Node_t *);

void Node_init(Node_t *self)
{
	memset(self, 0, sizeof(*self));

	vector_init(&self->clients, sizeof(ipv6_addr_t));
	avg_init(&self->average, ELECT_WEIGHT);

	self->this_thread = thread_getpid();

	get_node_ip_addr(&self->my_ip);
	Node_setCoord(self, &self->my_ip);
	Node_setState(self, STATE_ADVERTISING);
}

void Node_delete(Node_t *self)
{
	vector_delete(&self->clients);
}

void Node_elect_int(Node_t *self)
{
	switch(self->state)
	{
		case STATE_ADVERTISING:
			broadcast_id(&self->my_ip);
			break;

		case STATE_COORD_SKIP:
			Node_setState(self, STATE_COORD_STARTUP);
			break;

		case STATE_COORD_STARTUP:
			Node_sendRequests(self);
			Node_setState(self, STATE_COORD_RUN);
			break;

		case STATE_COORD_RUN:
			Node_deliverResult(self);
			Node_sendRequests(self);
			break;
	}

	Node_setTimer(self, TIMER_ADV, ELECT_MSG_INTERVAL, &interval_event);
}

void Node_elect_broadcast(Node_t *self, const char *ip)
{
	bool reset = false;

	switch(self->state)
	{
		case STATE_ADVERTISING:
		case STATE_WAITING:
			break;

		default:
			LOG_DEBUG("Starting coord election\n");
			Node_setState(self, STATE_ADVERTISING);
			reset = true;
	}

	ipv6_addr_t other_ip;

	ipv6_addr_from_str(&other_ip, ip);
	
	int c = ipv6_addr_cmp(&other_ip, &self->coord_ip);

	if(c > 0)
	{
		LOG_DEBUG("New leader candidate!\n");
		Node_setCoord(self, &other_ip);
		Node_setState(self, STATE_WAITING);
	}
	
	if(reset || c)
	{
		Node_resetElection(self);
	}
}

void Node_elect_done(Node_t *self)
{
	xtimer_ticks64_t d = xtimer_diff64(xtimer_now64(), self->last_elec);

	if(xtimer_usec_from_ticks64(d) >= (ELECT_LEADER_THRESHOLD-1) * MS_TO_US)
	{
		switch(self->state)
		{
			case STATE_ADVERTISING:
				Node_setState(self, STATE_COORD_SKIP);
				break;

			case STATE_WAITING:
				Node_setState(self, STATE_PART);
				coap_put_node(self->coord_ip, self->my_ip);
				Node_resetLife(self);
				break;

			default:
				break;
		}
	}
}

void Node_register(Node_t *self, const char *a)
{
	ipv6_addr_t client;

	ipv6_addr_from_str(&client, a);

	vector_add(&self->clients, &client);

	LOG_DEBUG("Added new client %s\n", a);
}

void Node_addSensor(Node_t *self, uint16_t v)
{
	avg_add(&self->average, v);

	LOG_DEBUG("Received new sensor value 0x%04x\n", v);
}

void Node_alive(Node_t *self)
{
	Node_resetLife(self);
}

void Node_dead(Node_t *self)
{
	LOG_DEBUG("Leader died!\n");

	Node_setState(self, STATE_ADVERTISING);
}

// # ----------------------------------------------------------------------------------------------

void Node_sendRequests(Node_t *self)
{
	int i;

	avg_reset(&self->average, sensor_read());

	for(i = 0 ; i < vector_size(&self->clients) ; ++i)
	{
		ipv6_addr_t *client = vector_get(&self->clients, i);

		coap_get_sensor(*client);

		char buf[IPV6_ADDR_MAX_STR_LEN];
		ipv6_addr_to_str(buf, client, sizeof(buf));
		LOG_DEBUG("requesting from %s\n", buf);
	}
}

void Node_deliverResult(Node_t *self)
{
	uint16_t v = avg_get(&self->average);

	LOG_DEBUG("Broadcasting sensor result 0x%04x\n", v);

	broadcast_sensor(v);
}

void Node_setTimer(Node_t *self, int t, uint d, evtimer_msg_event_t *e)
{
	memset(&self->timers[t], 0, sizeof(xtimer_t));
	xtimer_set_msg(&self->timers[t], d * MS_TO_US, &e->msg, self->this_thread);
}

void Node_resetElection(Node_t *self)
{
	xtimer_remove(&self->timers[TIMER_ELECT]);
	self->last_elec = xtimer_now64();
	Node_setTimer(self, TIMER_ELECT, ELECT_LEADER_THRESHOLD+1, &leader_threshold_event);
}

void Node_resetLife(Node_t *self)
{
	xtimer_remove(&self->timers[TIMER_LIFE]);
	Node_setTimer(self, TIMER_LIFE, ELECT_LEADER_TIMEOUT, &leader_timeout_event);
}

void Node_setState(Node_t *self, state_t s)
{
	static const char * const STATE_NAMES[] = {
		"ADVERTISING",
		"WAITING",
		"COORD_SKIP",
		"COORD_START",
		"COORD_RUN",
		"PART"
	};
	self->state = s;

	LOG_DEBUG("Switched to state %s.\n", STATE_NAMES[s]);

	switch(s)
	{
		case STATE_ADVERTISING:
			vector_clear(&self->clients);
			Node_setCoord(self, &self->my_ip);
			Node_resetElection(self);
			break;

		default:
			break;
	}
}

void Node_setCoord(Node_t *self, const ipv6_addr_t *ip)
{
	memcpy(&self->coord_ip, ip, sizeof(*ip));
}

