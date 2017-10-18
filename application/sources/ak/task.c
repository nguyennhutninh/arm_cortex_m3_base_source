/**
 ******************************************************************************
 * @author: ThanNT
 * @date:   13/08/2016
 ******************************************************************************
**/

#include "ak.h"
#include "ak_dbg.h"

#include "task.h"
#include "timer.h"
#include "message.h"

#include "../sys/sys_dbg.h"
#include "../sys/sys_ctrl.h"

#include "../common/utils.h"
#include "../common/log_queue.h"

#include "../app/task_list.h"

typedef struct {
	task_pri_t  pri;
	uint8_t     mask;
	ak_msg_t*   qhead;
	ak_msg_t*   qtail;
} tcb_t;

static task_id_t current_task_id;
static task_t	current_task_info;
static ak_msg_t	current_active_object;

log_queue_t log_task_dbg_object_queue;
log_queue_t log_irq_queue;

static uint8_t task_dbg_active_obj_queue[LOG_QUEUE_OBJECT_SIZE];
static uint8_t irq_queue[LOG_QUEUE_IRQ_SIZE];

static tcb_t	task_pri_queue[TASK_PRI_MAX_SIZE];
static task_t*	task_table = (task_t*)0;
static uint8_t	task_table_size = 0;
static uint8_t	task_current = 0;
static uint8_t	task_ready = 0;

static void task_sheduler();

/* function MUST-BE redefine */
__AK_WEAK void ak_irq_io_entry_trigger() {
#if defined(AK_IO_IRQ_ANALYZER)
	FATAL("TK", 0x03);
#endif
}

/* function MUST-BE redefine */
__AK_WEAK void ak_irq_io_exit_trigger() {
#if defined(AK_IO_IRQ_ANALYZER)
	FATAL("TK", 0x04);
#endif
}

uint8_t task_mutex_lock(task_pri_t cancel_pri) {
	uint8_t temp_pri = 0;
	ENTRY_CRITICAL();
	temp_pri = task_current;
	if (cancel_pri > task_current) {
		task_current = cancel_pri;
	}
	EXIT_CRITICAL();
	return temp_pri;
}

void task_mutex_unlock(task_pri_t pri) {
	ENTRY_CRITICAL();
	if (task_current < pri) {
		task_current = pri;
		task_sheduler();
	}
	EXIT_CRITICAL();
}

void task_create(task_t* task_tbl) {
	uint8_t idx = 0;
	if (task_tbl) {
		task_table = task_tbl;
		while(task_tbl[idx].id != AK_TASK_EOT_ID){
			idx++;
		}
		task_table_size = idx;
	}
	else {
		FATAL("TK", 0x01);
	}
}

void task_post(task_id_t des_task_id, ak_msg_t* msg) {
	task_t* task;
	tcb_t* t_tcb;

	if (des_task_id >= task_table_size) {
		FATAL("TK", 0x02);
	}

	task = &task_table[des_task_id];
	t_tcb = &task_pri_queue[task->pri - 1];

	ENTRY_CRITICAL();

	msg->next = AK_MSG_NULL;
	msg->des_task_id = des_task_id;

#if (AK_TASK_DEBUG == AK_ENABLE)
	if (get_msg_ref_count(msg) <= 1) {
		msg->dbg_handler.start_exe = 0;
		msg->dbg_handler.stop_exe = 0;
		msg->dbg_handler.start_post = sys_ctrl_millis();
	}
#endif

	if (t_tcb->qtail == AK_MSG_NULL) {
		/* put message to queue */
		t_tcb->qtail = msg;
		t_tcb->qhead = msg;

		/* change status task to ready*/
		task_ready |= t_tcb->mask;

#if (AK_PREEMPTIVE == AK_ENABLE)
		task_sheduler();
#endif

	}
	else {
		/* put message to queue */
		t_tcb->qtail->next = msg;
		t_tcb->qtail = msg;
	}

	EXIT_CRITICAL();
}

void task_entry_interrupt() {
	ENTRY_CRITICAL();

	ak_irq_io_entry_trigger();

	exception_info_t exception_info;
	exception_info.except_number = sys_ctr_get_exception_number();
	exception_info.timestamp = sys_ctrl_millis();

	log_queue_put(&log_irq_queue, &exception_info);

	current_task_id = AK_TASK_INTERRUPT_ID;

	EXIT_CRITICAL();
}

void task_exit_interrupt() {
	ENTRY_CRITICAL();

	current_task_id = current_task_info.id;

	ak_irq_io_exit_trigger();

	EXIT_CRITICAL();
}

int task_init() {
	uint8_t pri = 1;
	tcb_t* t_tcb = (tcb_t*)0;

	/* init task manager variable */
	task_current = 0;
	task_ready = 0;

	/* init kernel queue */
	for (pri = 1; pri <= TASK_PRI_MAX_SIZE; pri++) {
		t_tcb = &task_pri_queue[pri - 1];
		t_tcb->mask     = (1 << (pri - 1));
		t_tcb->qhead    = AK_MSG_NULL;
		t_tcb->qtail    = AK_MSG_NULL;
	}

	/* message manager must be initial fist */
	msg_init();

	/* init timer manager */
	timer_init();

	return 0;
}

int task_run() {
	/* init active object log queue */
	log_queue_init(&log_task_dbg_object_queue					\
				   , (uint32_t)task_dbg_active_obj_queue		\
				   , (LOG_QUEUE_OBJECT_SIZE / sizeof(ak_msg_t))	\
				   , sizeof(ak_msg_t)							\
				   , mem_write									\
				   , mem_read);

	/* init irq log queue */
	log_queue_init(&log_irq_queue								\
				   , (uint32_t)irq_queue						\
				   , (LOG_QUEUE_IRQ_SIZE / sizeof(exception_info_t))	\
				   , sizeof(exception_info_t)							\
				   , mem_write									\
				   , mem_read);

	SYS_PRINT("active objects is ready\n\n");

	for(;;) {
		task_sheduler();

		/* idle handler */
		ENTRY_CRITICAL();
		current_task_id = AK_TASK_IDLE_ID;
		sys_ctr_sleep_wait_for_irq();
		EXIT_CRITICAL();
	}
}

void task_sheduler() {
	static uint8_t const log2lkup[] = {
		0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
	};
	uint8_t t_task_current = task_current;
	uint8_t t_task_new;

	while((t_task_new = log2lkup[task_ready]) > t_task_current) {
		/* get task */
		tcb_t* t_tcb = &task_pri_queue[t_task_new - 1];

		/* get message */
		ak_msg_t* t_msg = t_tcb->qhead;
		t_tcb->qhead = t_msg->next;

		/* last message of queue */
		if (t_msg->next == AK_MSG_NULL) {
			t_tcb->qtail = AK_MSG_NULL;
			/* change status of task to inactive */
			task_ready &= ~t_tcb->mask;
		}

		/* update current task */
		task_current = t_task_new;

		/* start task debug */
#if (AK_TASK_DEBUG == AK_ENABLE)
		t_msg->dbg_handler.start_exe = sys_ctrl_millis();
#endif
		/* update current ak object */
		memcpy(&current_task_info, &task_table[t_msg->des_task_id], sizeof(task_t));
		memcpy(&current_active_object, t_msg, sizeof(ak_msg_t));

		/* update curren task id NOTE: current task id will be change when entry interrupt handler */
		current_task_id = t_msg->if_des_task_id;

		/* execute task */
#if (AK_PREEMPTIVE == AK_ENABLE)
		EXIT_CRITICAL();
#endif
		task_table[t_msg->des_task_id].task(t_msg);

#if (AK_PREEMPTIVE == AK_ENABLE)
		ENTRY_CRITICAL();
#endif

#if (AK_TASK_DEBUG == AK_ENABLE)
		/* reject msg of timer task */
		if (current_active_object.des_task_id > 0) {
			current_active_object.dbg_handler.stop_exe = sys_ctrl_millis();

			/* put current object to log queue */
			log_queue_put(&log_task_dbg_object_queue, &current_active_object);

#if defined(AK_TASK_LOG_ENABLE)
			{
				uint32_t exe_time;
				uint32_t wait_time;
				if (current_active_object.dbg_handler.start_exe >= current_active_object.dbg_handler.start_post) {
					wait_time = current_active_object.dbg_handler.start_exe - current_active_object.dbg_handler.start_post;
				}
				else {
					wait_time = current_active_object.dbg_handler.start_exe + ((uint32_t)0xFFFFFFFF - current_active_object.dbg_handler.start_post);
				}

				if (current_active_object.dbg_handler.stop_exe >= current_active_object.dbg_handler.start_exe) {
					exe_time = current_active_object.dbg_handler.stop_exe - current_active_object.dbg_handler.start_exe;
				}
				else {
					exe_time = current_active_object.dbg_handler.stop_exe + ((uint32_t)0xFFFFFFFF - current_active_object.dbg_handler.start_exe);
				}

				xprintf("task_id: %d\tmsg_type:0x%x\tref_count:%d\tsig:%d\t\twait_time:%d\texe_time:%d\n"\
						, current_active_object.des_task_id								\
						, (current_active_object.ref_count & AK_MSG_TYPE_MASK)		\
						, (current_active_object.ref_count & AK_MSG_REF_COUNT_MASK)	\
						, current_active_object.sig									\
						, (wait_time)	\
						, (exe_time));
			}
#endif
		}

		if (get_msg_ref_count(t_msg) > 1) {
			t_msg->dbg_handler.start_post = current_active_object.dbg_handler.stop_exe ;
			t_msg->dbg_handler.start_exe = 0;
			t_msg->dbg_handler.stop_exe = 0;
		}

		/* clear current active object */
		memset(&current_active_object, 0, sizeof(ak_msg_t));

#endif

		/* check and free message */
		msg_free(t_msg);
	}

	task_current = t_task_current;
}

task_id_t task_self() {
	return current_task_info.id;
}

task_id_t get_current_task_id() {
	return current_task_id;
}

task_t*	get_current_task_info() {
	return &current_task_info;
}

ak_msg_t* get_current_active_object() {
	return &current_active_object;
}