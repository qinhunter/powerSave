#include"nfv.h"

void clearqueue(char * queuename);//give it a name of a queue where packets left will be cleared.


int main() {
	char proname[] = "clear_queues";

	struct rlimit rlim;

	getrlimit(RLIMIT_MSGQUEUE, &rlim);
	printf("RLIMIT_MSGQUEUE:rlim_cur = %d, rlim_max = %d\n", (int) rlim.rlim_cur, (int) rlim.rlim_max);

	rlim.rlim_cur = RLIM_INFINITY;
	rlim.rlim_max = RLIM_INFINITY;

	setrlimit(RLIMIT_MSGQUEUE, &rlim); 
	printf("RLIMIT_MSGQUEUE:rlim_cur = %d, rlim_max = %d\n", (int) rlim.rlim_cur, (int) rlim.rlim_max);	





	clearqueue("/sendtol3");
	clearqueue("/l3top2");
	clearqueue("/l3top3");

	clearqueue("/ctrltosend");
	clearqueue("/sendtoctrl");

	clearqueue("/ctrltop1");
	clearqueue("/p1toctrl");
	clearqueue("/ctrltop2");
	clearqueue("/p2toctrl");
	clearqueue("/ctrltop3");
	clearqueue("/p3toctrl");






	func_quit(proname);
	exit(0);

}



void clearqueue(char * queuename) {
	int flags = O_CREAT | O_RDWR | O_NONBLOCK;
	mqd_t mqd_clear;
	int clear_return = 0;

	struct mq_attr attr, *attrp;
	attrp = NULL;
	attr.mq_maxmsg = 10;//maximum is 382.
	attr.mq_msgsize = 2048;
	attr.mq_flags = 0;
	attrp = &attr;

	long long int i = 0;
	int func_re = 0;
	char msg_buffer[2048];



	mqd_clear = mq_open(queuename, flags, PERM, attrp);
	check_return(mqd_clear, queuename, "mq_open");

	i = 0;
	while((clear_return = mq_receive(mqd_clear, msg_buffer, 2048, 0)) >= 0) {
		i++;
		if(i < 500) printf("%s:receive %lld times \n", queuename, i);

	}
	if(i >= 500) printf("in queue %s, there are %lld packets left. \n", queuename, i);

	clear_return = mq_close(mqd_clear);//returns 0 on success, or -1 on error.
	check_return(clear_return, queuename, "mq_close");
	clear_return = mq_unlink(queuename);//returns 0 on success, or -1 on error.
	check_return(clear_return, queuename, "mq_unlink");
	printstar();
}
