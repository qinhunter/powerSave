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





	clearqueue("/sendtop1");
	clearqueue("/p1top2");
	clearqueue("/p1top3");
	clearqueue("/p2top5");
	clearqueue("/p2top4");
	clearqueue("/p3top4");
	clearqueue("/p3top7");
	clearqueue("/p5top6");
	clearqueue("/p4top6");
	clearqueue("/p4top7");
	clearqueue("/p3top7");
	clearqueue("/p6top8");
	clearqueue("/p7top8");


	clearqueue("/ctrltop1");
	clearqueue("/p1toctrl");
	clearqueue("/ctrltop2");
	clearqueue("/p2toctrl");
	clearqueue("/p3toctrl");
	clearqueue("/ctrltop3");
	clearqueue("/p4toctrl");
	clearqueue("/ctrltop4");
	clearqueue("/p5toctrl");
	clearqueue("/ctrltop5");
	clearqueue("/p6toctrl");
	clearqueue("/ctrltop6");
	clearqueue("/p7toctrl");
	clearqueue("/ctrltop7");
	clearqueue("/p8toctrl");
	clearqueue("/ctrltop8");







	func_quit(proname);
	exit(0);

}



void clearqueue(char * queuename) {
	int flags = O_CREAT | O_RDWR | O_NONBLOCK;
	mqd_t mqd_clear;

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
	while((func_re = mq_receive(mqd_clear, msg_buffer, 2048, 0)) >= 0) {
		i++;
		//if(i < 500) printf("%s:receive %lld times \n", queuename, i);

	}
	if(i > 0) {
		printf("ATTENTION PLEASE!!!ATTENTION PLEASE!!!ATTENTION PLEASE!!!\n\n");
		printf("In queue %s, there are %lld packets left. \n", queuename, i);
		printf("\nATTENTION PLEASE!!!ATTENTION PLEASE!!!ATTENTION PLEASE!!!\n");
	}
	else {
		printf("There is nothing left in queue %s. \n", queuename);
	}
	func_re = mq_close(mqd_clear);//returns 0 on success, or -1 on error.
	check_return(func_re, queuename, "mq_close");
	func_re = mq_unlink(queuename);//returns 0 on success, or -1 on error.
	check_return(func_re, queuename, "mq_unlink");
	printnewline();
}

