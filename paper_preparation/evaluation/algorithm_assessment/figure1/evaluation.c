#include "find_the_best.h"

#define EDGES_START 24
#define EDGES_END 24 //15
#define PER_CASE_TIMES 10000 //5000

int main(void) {
	int i = 0;
	int edges = 3;
	struct rank_ans ans;
	double temp_rank = 0;
	long int temp_time = 0;
	int temp_loop = 0;
	
	double rank_sum[EDGES_END + 1];
	double rating[EDGES_END + 1];
	
	long int time_sum[EDGES_END + 1];
	double time[EDGES_END + 1];
	
	int loop_sum[EDGES_END + 1];
	double loop[EDGES_END + 1];
	printf("edges starts from %d to %d, per case tested %d times \n", EDGES_START, EDGES_END, PER_CASE_TIMES);
	for(edges = EDGES_START;edges <= EDGES_END;edges++) {
		printf("now start edges = %d KL_assessment \n", edges);
		temp_rank = 0;
		temp_time = 0;
		temp_loop = 0;
		for(i = 0;i < PER_CASE_TIMES;i++) {
			ans = rank(edges);
			temp_rank += ans.rank_rating;
			temp_time += ans.KL_time;
			temp_loop += ans.loop_times;
			//printf("i = %d, rating = %f \n", i, ans);
		}
		rank_sum[edges] = temp_rank;
		time_sum[edges] = temp_time;
		loop_sum[edges] = temp_loop;
		rating[edges] = temp_rank / PER_CASE_TIMES;
		time[edges] = ((double) temp_time) / PER_CASE_TIMES;
		loop[edges] = ((double) temp_loop) / PER_CASE_TIMES;
	}
	for(edges = EDGES_START;edges <= EDGES_END;edges++) {
		printf("edges = %d, rating = %f, rank_sum = %f, KL_time = %f, KL_time sum = %ld, loop_times = %f, loop_sum = %d \n", edges, rating[edges], rank_sum[edges], time[edges], time_sum[edges], loop[edges], loop_sum[edges]);
	}
	return 0;
}
