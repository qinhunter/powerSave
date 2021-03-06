#include "nfv.h"
#include "fan.h"
#include "../posix/ndpi_api.h" //iphdr
#include <pcap.h>

int main() {
	/*initialization about mqueue*/
	char proname[] = "p2_IDS";
	setcpu(P2_STARTING_CPU);

	struct mq_attr attr, attr_ctrl;
	attr.mq_maxmsg = MAXMSG;//maximum is 382.
	attr.mq_msgsize = 2048;
	attr.mq_flags = 0;

	attr_ctrl.mq_maxmsg = MAXMSGCTOP;
	attr_ctrl.mq_msgsize = 2048;
	attr_ctrl.mq_flags = 0;


	int flags = O_CREAT | O_RDWR;
	int flags_ctrl = O_CREAT | O_RDWR | O_NONBLOCK;
	mqd_t mqd_p1top2, mqd_p2top5, mqd_p2top4;
	int mq_return = 0;
	char p1top2[] = "/p1top2";
	char p2top5[] = "/p2top5";
	char p2top4[] = "/p2top4";
	
	mqd_p1top2 = mq_open(p1top2, flags, PERM, &attr);
	check_return(mqd_p1top2, p1top2, "mq_open");

	mqd_p2top5 = mq_open(p2top5, flags, PERM, &attr);
	check_return(mqd_p1top2, p2top5, "mq_open");

	mqd_p2top4 = mq_open(p2top4, flags, PERM, &attr);
	check_return(mqd_p1top2, p2top4, "mq_open");


	/*control part*/
	mqd_t mqd_ctrltop2, mqd_p2toctrl;
	char ctrltop2[] = "/ctrltop2";
	char p2toctrl[] = "/p2toctrl";
	mqd_ctrltop2 = mq_open(ctrltop2, flags_ctrl, PERM, &attr_ctrl);
	check_return(mqd_ctrltop2, ctrltop2, "mq_open");
	mqd_p2toctrl = mq_open(p2toctrl, flags_ctrl, PERM, &attr_ctrl);
	check_return(mqd_p2toctrl, p2toctrl, "mq_open");


	char buffer[2048];
	struct ndpi_iphdr * iph;
	long long int i = 0;

	/*pthread*/
	struct transfer noti_tran;
	noti_tran.mqd_ctop = mqd_ctrltop2;
	noti_tran.mqd_ptoc = mqd_p2toctrl;
	//noti_tran.mqd_p[0] = mqd_p1top2;
	noti_tran.qds = 1;
	noti_tran.i[0] = &i;
	notifysetup(&noti_tran);


//////////////////////////////////////////fw///////////////////////////
    setupDetection();    //ndpi setup
/////////////////////////////////////////////////////////////////////	

	

	writeAcl(15);
	struct timeval timestamp;
	gettimeofday( &timestamp, NULL);

	FILE *filp = NULL; 
	char fileDir[] = "./log_IDS1.txt";
	filp = fopen(fileDir,"w");

/////////////////////////////////////////////////////////////////////


	//int port = 0;

	for(i = 0;i < PACKETS*10;i += 2) {
		mq_return = mq_receive(mqd_p1top2, buffer, 2048, 0);
		if(mq_return == -1) {
			printf("%s:%s receive %lld times fails:%s, errno = %d \n", proname, p1top2, i, strerror(errno), errno);
			return -1;
		}
		iph = (struct ndpi_iphdr *) buffer;
		if((i%SHOW_FREQUENCY == 0) || (i < 400)) {
			printf("i = %lld, packet length = %d, iph->daddr = %8X \n", i, mq_return, iph->daddr);
			printf("pid = %d , working on CPU %d \n", getpid(), getcpu());
		}
		if(i%CHECKQUEUE_FREQUENCY == 0) {
			checkqueue(mqd_p1top2, p1top2, &noti_tran);
		}

		/////////////////////////////////////IDS
		IDS(timestamp, mq_return, iph, filp, i);
		mq_return = mq_send(mqd_p2top5, (char *) iph, mq_return, 0);
		if(mq_return == -1) {
			printf("%s:%s send %lld times fails:%s, errno = %d \n", proname, p2top5, i, strerror(errno), errno);
			return -1;
		}		

		mq_return = mq_receive(mqd_p1top2, buffer, 2048, 0);
		if(mq_return == -1) {
			printf("%s:%s receive %lld times fails:%s, errno = %d \n", proname, p1top2, i, strerror(errno), errno);
			return -1;
		}
		iph = (struct ndpi_iphdr *) buffer;
		if((i%SHOW_FREQUENCY == 0) || (i < SHOW_THRESHOLD)) {
			printf("i = %lld, packet length = %d, iph->daddr = %8X \n", i, mq_return, iph->daddr);
			printf("pid = %d , working on CPU %d \n", getpid(), getcpu());
		}
		if(i%CHECKQUEUE_FREQUENCY == 0) {
			checkqueue(mqd_p1top2, p1top2, &noti_tran);
		}
		
		/////////////////////////////////////IDS
		IDS(timestamp, mq_return, iph, filp, i);
		mq_return = mq_send(mqd_p2top4, (char *) iph, mq_return, 0);
		if(mq_return == -1) {
			printf("%s:%s send %lld times fails:%s, errno = %d \n", proname, p2top4, i, strerror(errno), errno);
			return -1;
		}	


		

	}
	printf("%s has transfered %lld packets. \n", proname, i);

	//p1top2
	mq_return = mq_close(mqd_p1top2);//returns 0 on success, or -1 on error.
	check_return(mq_return, proname, "mq_close");
	mq_return = mq_unlink(p1top2);//returns 0 on success, or -1 on error.
	check_return(mq_return, proname, "mq_unlink");

	//p2top5
	mq_return = mq_close(mqd_p2top5);//returns 0 on success, or -1 on error.
	check_return(mq_return, proname, "mq_close");
	mq_return = mq_unlink(p2top5);//returns 0 on success, or -1 on error.
	check_return(mq_return, proname, "mq_unlink");

	//p2top4
	mq_return = mq_close(mqd_p2top4);//returns 0 on success, or -1 on error.
	check_return(mq_return, proname, "mq_close");
	mq_return = mq_unlink(p2top4);//returns 0 on success, or -1 on error.
	check_return(mq_return, proname, "mq_unlink");

	//ctrltop2
	mq_return = mq_close(mqd_ctrltop2);//returns 0 on success, or -1 on error.
	check_return(mq_return, proname, "mq_close");
	mq_return = mq_unlink(ctrltop2);//returns 0 on success, or -1 on error.
	check_return(mq_return, proname, "mq_unlink");
	//p2toctrl
	mq_return = mq_close(mqd_p2toctrl);//returns 0 on success, or -1 on error.
	check_return(mq_return, proname, "mq_close");
	mq_return = mq_unlink(p2toctrl);//returns 0 on success, or -1 on error.
	check_return(mq_return, proname, "mq_unlink");


	exit(0);

}

