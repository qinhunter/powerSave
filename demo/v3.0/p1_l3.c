#include "nfv.h"
#include "fan.h"





int main() {
	/*initialization about mqueue*/
	char proname[] = "p1_l3fwd";
	setcpu(2);

	struct mq_attr attr, attr_ctrl;
	struct mq_attr q_attr;
	attr.mq_maxmsg = MAXMSG;//maximum is 382.
	attr.mq_msgsize = 2048;
	attr.mq_flags = 0;

	attr_ctrl.mq_maxmsg = MAXMSGCTOP;
	attr_ctrl.mq_msgsize = 2048;
	attr_ctrl.mq_flags = 0;

	int flags = O_CREAT | O_RDWR;
	int flags_ctrl = O_CREAT | O_RDWR | O_NONBLOCK;
	mqd_t mqd_sdtop1, mqd_p1top2, mqd_p1top3;
	int mq_return = 0;
	char sdtop1[] = "/sendtop1";
	char p1top2[] = "/p1top2";
	char p1top3[] = "/p1top3";

	mqd_sdtop1 = mq_open(sdtop1, flags, PERM, &attr);
	check_return(mqd_sdtop1, sdtop1, "mq_open");

	mqd_p1top2 = mq_open(p1top2, flags, PERM, &attr);
	check_return(mqd_p1top2, p1top2, "mq_open");

	mqd_p1top3 = mq_open(p1top3, flags, PERM, &attr);
	check_return(mqd_p1top3, p1top3, "mq_open");

	/*control part*/
	mqd_t mqd_ctrltop1, mqd_p1toctrl;
	char ctrltop1[] = "/ctrltop1";
	char p1toctrl[] = "/p1toctrl";
	mqd_ctrltop1 = mq_open(ctrltop1, flags_ctrl, PERM, &attr_ctrl);
	check_return(mqd_ctrltop1, ctrltop1, "mq_open");
	mqd_p1toctrl = mq_open(p1toctrl, flags_ctrl, PERM, &attr_ctrl);
	check_return(mqd_p1toctrl, p1toctrl, "mq_open");


	char buffer[2048];
	struct ndpi_iphdr * iph;
	long long int i = 0;


	/*pthread*/
	struct transfer noti_tran;
	noti_tran.mqd_ctop = mqd_ctrltop1;
	noti_tran.mqd_ptoc = mqd_p1toctrl;
	//noti_tran.mqd_p[0] = mqd_sdtop1;
	noti_tran.qds = 1;
	noti_tran.i[0] = &i;
	notifysetup(&noti_tran);






	NODE * g_pRouteTree = createNode();

	int routetable[2] = {0x9000FFFF, 0x0000FFFF};
	int j = 0;
	for(j = 0;j < 2;j++) {
		createRouteTree(g_pRouteTree, routetable[j], 1, j);
		printf("route[%d]:%8X, port:%d\n", j, routetable[j], j);
	}
	createRouteTree(g_pRouteTree, 0, 0, 999);


	int port = 0;

	for(i = 0;i < PACKETS;i++) {//PACKETS = 5000 now.
		mq_return = mq_receive(mqd_sdtop1, buffer, 2048, 0);
		if(mq_return == -1) {
			printf("%s:receive %lld times fails:%s, errno = %d \n", proname, i, strerror(errno), errno);
		}
		iph = (struct ndpi_iphdr *) buffer;
		
		/*
		//////////////////////////////////////////////////////////////
		mq_return = mq_send(mqd_p1top2, (char *) iph, mq_return, 0);
		if(mq_return == -1) {
			printf("%s:send %lld times fails:%s, errno = %d \n", proname, i, strerror(errno), errno);
			printstar();
			printstar();
			printstar();
		}
		i++;
		mq_return = mq_receive(mqd_sdtop1, buffer, 2048, 0);
		if(mq_return == -1) {
			printf("%s:receive %lld times fails:%s, errno = %d \n", proname, i, strerror(errno), errno);
		}
		iph = (struct ndpi_iphdr *) buffer;
		mq_return = mq_send(mqd_p1top3, (char *) iph, mq_return, 0);
		if(mq_return == -1) {
			printf("%s:send %lld times fails:%s, errno = %d \n", proname, i, strerror(errno), errno);
			printstar();
			printstar();
			printstar();
		}*/
		
		
		
		
		
		port = getIpFwdPort(g_pRouteTree, iph->daddr);
		if((i%100000 == 0) || (i < 400)) {
			printf("i = %lld, iph->daddr = %8X, port = %d \n", i, iph->daddr, port);
			printf("pid = %d , working on CPU %d \n", getpid(), getcpu());
		}
		if(i%CHECKQUEUE_FREQUENCY == 0) {

			checkqueue(mqd_sdtop1, sdtop1, &noti_tran);//check if the queue is congested and process need to be splited.
		}
		switch(port)
		{
			case 0:
				mq_return = mq_send(mqd_p1top2, (char *) iph, mq_return, 0);
				if(mq_return == -1) {
					printf("%s:send %lld times fails:%s, errno = %d \n", proname, i, strerror(errno), errno);
					printstar();
					printstar();
					printstar();
				}
				break;
			case 1:
				mq_return = mq_send(mqd_p1top3, (char *) iph, mq_return, 0);
				if(mq_return == -1) {
					printf("%s:send %lld times fails:%s, errno = %d \n", proname, i, strerror(errno), errno);
					printstar();
					printstar();
					printstar();
				}
				break;
		}

		
	}
	printf("i = %lld, iph->daddr = %8X, port = %d \n", i, iph->daddr, port);
	printf("%s has transfered %lld packets. \n", proname, i);
	checkcpu();

	//sdtop1
	mq_return = mq_close(mqd_sdtop1);//returns 0 on success, or -1 on error.
	check_return(mq_return, sdtop1, "mq_close");
	mq_return = mq_unlink(sdtop1);//returns 0 on success, or -1 on error.
	check_return(mq_return, sdtop1, "mq_unlink");
	//p1top2
	mq_return = mq_close(mqd_p1top2);//returns 0 on success, or -1 on error.
	check_return(mq_return, p1top2, "mq_close");
	mq_return = mq_unlink(p1top2);//returns 0 on success, or -1 on error.
	check_return(mq_return, p1top2, "mq_unlink");
	//p1top3
	mq_return = mq_close(mqd_p1top3);//returns 0 on success, or -1 on error.
	check_return(mq_return, p1top3, "mq_close");
	mq_return = mq_unlink(p1top3);//returns 0 on success, or -1 on error.
	check_return(mq_return, p1top3, "mq_unlink");

	//ctrltop1
	mq_return = mq_close(mqd_ctrltop1);//returns 0 on success, or -1 on error.
	check_return(mq_return, ctrltop1, "mq_close");
	mq_return = mq_unlink(ctrltop1);//returns 0 on success, or -1 on error.
	check_return(mq_return, ctrltop1, "mq_unlink");
	//p1toctrl
	mq_return = mq_close(mqd_p1toctrl);//returns 0 on success, or -1 on error.
	check_return(mq_return, p1toctrl, "mq_close");
	mq_return = mq_unlink(p1toctrl);//returns 0 on success, or -1 on error.
	check_return(mq_return, p1toctrl, "mq_unlink");






	exit(0);

}







