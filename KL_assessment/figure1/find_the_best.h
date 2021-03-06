#include "extended_KL.h"

//#define ADJ_SIZE 18

struct rank_ans {
	double rank_rating;
	long int KL_time;

};

static int srand_first_time = 0;

struct score {
	double edge_cut;
	double workload_diff;
};



long long fact(int n);
long long Cmn(int m, int n);
void printstar(void);
void newline(void);
int next_set(int set_edge, int* set1, int* set2, int old_edge);
struct score get_score(int new_edge, double adj_array[new_edge][new_edge], double point_weight[new_edge], int set_edge, int set1[set_edge], int set2[set_edge]);

double random_percentage();//generate a random percentage from 0.00 to 1.00.
int range_rand(int range);
double score_cmp(struct score old, struct score new);
struct rank_ans rank(int ADJ_SIZE);


struct rank_ans rank(int ADJ_SIZE) {
	struct rank_ans rank_re;
	struct timeval start;
	struct timeval end;
	int i = 0;
	int j = 0;
	
	//generate a random adj_array[ADJ_SIZE][ADJ_SIZE];
	double raw_adj_array[ADJ_SIZE][ADJ_SIZE];
	clear_double_array(ADJ_SIZE, raw_adj_array, "raw_adj_array");
	int connection_count[ADJ_SIZE];
	clear_int_series(ADJ_SIZE, connection_count, "connection_count");
	int degree_size = ADJ_SIZE / 2;
	for(i = 0;i < ADJ_SIZE;i++) {
		for(j = i + 1;j < ADJ_SIZE;j++) {
			//every row and column has at most "degree_size" connections with others
			if((connection_count[i] <= degree_size) && (connection_count[j] <= degree_size) && (random_percentage() > 0.6)) {
				raw_adj_array[i][j] = range_rand(100);
				connection_count[i] += 1;
				connection_count[j] += 1;
			}
			else {
				raw_adj_array[i][j] = 0;
			}
		}
	}
		
	for(i = 0;i < ADJ_SIZE;i++) {//to make sure that the array is correct.
		raw_adj_array[i][i] = 0;
		for(j = i + 1;j < ADJ_SIZE;j++) {
			raw_adj_array[j][i] = raw_adj_array[i][j];
		}
	}
	#ifdef PRINT_MODE
	show_double_array(ADJ_SIZE, raw_adj_array, "raw_adj_array");
	printstar();
	#endif
	
	
	
	//generate a random point_weight[ADJ_SIZE].
	double raw_point_weight[ADJ_SIZE];	
	double point_sum = 0;
	for(i = 0;i < ADJ_SIZE;i++) {
		raw_point_weight[i] = random_percentage();
		point_sum += raw_point_weight[i];	
	}
	point_sum *= 0.65;//so the real point weight sum ~=1/0.65=1.53846154
	for(i = 0;i < ADJ_SIZE;i++) {
		raw_point_weight[i] = raw_point_weight[i] / point_sum;
	}
	#ifdef PRINT_MODE
	show_double_series(ADJ_SIZE, raw_point_weight, "raw_point_weight");
	printstar();
	#endif
	
	
	//start KL preparation part
	gettimeofday(&start, 0);
	int old_edge = ADJ_SIZE;
	int new_edge = 0;
	if(old_edge%2 == 1) {
		new_edge = old_edge + 1;
	}
	else {
		new_edge = old_edge;
	}
	
	double adj_array[new_edge][new_edge];
	double point_weight[new_edge];
	clear_double_series(new_edge, point_weight, "point_weight");
	clear_double_array(new_edge, adj_array, "adj_array");
	copy_double_series(old_edge, raw_point_weight, point_weight);
	if(old_edge % 2 == 0) {
		copy_double_array(new_edge, raw_adj_array, adj_array);
	}
	else {//old_edge % 2 == 1
		for(i = 0;i < old_edge;i++) {
			for(j = 0;j < old_edge;j++) {
				adj_array[i][j] = raw_adj_array[i][j];
			}
		}
		for(i = 0;i < new_edge;i++) {//let the last column of array equal 0.
			adj_array[i][old_edge] = 0;
		}
		for(j = 0;j < new_edge;j++) {//let the last row of array equal 0.
			adj_array[old_edge][j] = 0;
		}
		point_weight[new_edge - 1] = 0;
	}
	#ifdef PRINT_MODE
	show_double_array(new_edge, adj_array, "new_adj_array");
	show_double_series(new_edge, point_weight, "new_point_weight");
	#endif
	int set_edge = new_edge / 2;

	int* set1 = NULL;
	int* set2 = NULL;
	set1 = (int*) calloc(set_edge, sizeof(int));
	set2 = (int*) calloc(set_edge, sizeof(int));
	
	int sum = 0;
	for(i = 0;i < set_edge;i++) {
		set1[i] = i;
		set2[i] = i + set_edge;
		//show_int_series(set_edge, set1, "set1");
		//show_int_series(set_edge, set2, "set2");
	}
	#ifdef PRINT_MODE
	show_int_series(set_edge, set1, "set1");
	show_int_series(set_edge, set2, "set2");
	#endif
	
	//KL part
	struct KL_return KL_re;
	KL_re = KL_step(new_edge, adj_array, point_weight, set_edge, set1, set2);
	gettimeofday(&end, 0);
	
	#ifdef PRINT_MODE
	show_int_series(set_edge, KL_re.set1, "KL_re.set1");
	show_int_series(set_edge, KL_re.set2, "KL_re.set2");
	#endif
	struct score ours;
	ours = get_score(new_edge, adj_array, point_weight, set_edge, KL_re.set1, KL_re.set2);
	#ifdef PRINT_MODE
	printf("ours.edge_cut = %f, ours.workload_diff = %f \n", ours.edge_cut, ours.workload_diff);
	printstar();
	#endif
	
	
	long long count_better = 0;
	long long count_all = 0;
	long long count_edge = 0;
	long long count_workload = 0;

	double gain_max = 0;
	struct score re;
	#ifdef PRINT_MODE
	struct score best;
	best = ours;
	struct KL_return best_sets;
	best_sets = KL_re;
	struct score* better = NULL;
	int*** better_sets = NULL;
	#endif
	double gain = 0;
	do{
		#ifdef PRINT_MODE
		printf("count_all = %lld \n", count_all);
		show_int_series(set_edge, set1, "set1");
		show_int_series(set_edge, set2, "set2");
		#endif
		
		re = get_score(new_edge, adj_array, point_weight, set_edge, set1, set2);
		
		#ifdef PRINT_MODE
		printf("re.edge_cut = %f, re.workload_diff = %f \n", re.edge_cut, re.workload_diff);
		#endif
		gain = score_cmp(ours, re);
		if(gain > EPSILON) {
			#ifdef PRINT_MODE
			//record the better combination
			if(gain > gain_max) {
				gain_max = gain;
				best = re;
				copy_int_series(set_edge, set1, best_sets.set1);
				copy_int_series(set_edge, set2, best_sets.set2);
			}
			better = (struct score*) realloc(better, sizeof(struct score) * (count_better + 1));
			better[count_better] = re;
			better_sets = (int***) realloc(better_sets, sizeof(int**) * (count_better) + 1);
			better_sets[count_better] = (int**) malloc(sizeof(int*) * 2);
			better_sets[count_better][0] = (int*) malloc(sizeof(int) * set_edge);
			better_sets[count_better][1] = (int*) malloc(sizeof(int) * set_edge);
			memcpy(better_sets[count_better][0], set1, sizeof(int) * set_edge);
			memcpy(better_sets[count_better][1], set2, sizeof(int) * set_edge);
			printf("There is one combination totally better than ours.!!! \n");
			printstar();
			printstar();
			printstar();
			#endif
			count_better++;
		
		}
		/*
		if((re.edge_cut < ours.edge_cut) && (re.workload_diff < ours.workload_diff)) {
			better = (struct score*) realloc(better, sizeof(struct score) * (count_better + 1));
			better[count_better] = re;
			better_sets = (int***) realloc(better_sets, sizeof(int**) * (count_better) + 1);
			better_sets[count_better] = (int**) malloc(sizeof(int*) * 2);
			better_sets[count_better][0] = (int*) malloc(sizeof(int) * set_edge);
			better_sets[count_better][1] = (int*) malloc(sizeof(int) * set_edge);
			memcpy(better_sets[count_better][0], set1, sizeof(int) * set_edge);
			memcpy(better_sets[count_better][1], set2, sizeof(int) * set_edge);
			printf("There is one combination totally better than ours.!!! \n");
			printstar();
			printstar();
			printstar();
			count_better++;
			count_edge++;
			count_workload++;
			//printf("re.edge_cut = %f, re.workload_diff = %f \n", re.edge_cut, re.workload_diff);
		}
		if(re.edge_cut < ours.edge_cut) {
			count_edge++;
			printf("this one's edge_cut is better than us but workload_diff worse than ours. \n");
			printstar();
		}
		if(re.workload_diff < ours.workload_diff) {
			count_workload++;
			printf("this one's edge_cut is worse than us but workload_diff better than ours. \n");
			printstar();
		}*/					//old algorithm.
		
		
		count_all++;
	}while(next_set(set_edge, set1, set2, new_edge));
	
	#ifdef PRINT_MODE
	newline();
	printstar();
	newline();
	if(count_better > 0) {
		printf("Now show the %lld better combination(s). \n", count_better);
		for(i = 0;i < count_better;i++) {
			printf("better %d: edge_cut = %f, workload_diff = %f \n", i, better[i].edge_cut, better[i].workload_diff);
			show_int_series(set_edge, better_sets[i][0], "better set1");
			show_int_series(set_edge, better_sets[i][1], "better set2");
			newline();
			free(better_sets[i][0]);
			free(better_sets[i][1]);
			free(better_sets[i]);
		}
		
		printf("show the best sets.\n");
		show_int_series(set_edge, best_sets.set1, "best_sets.set1");
		show_int_series(set_edge, best_sets.set2, "best_sets.set2");
		printf("gain = %f, best.edge_cut = %f, best.workload_diff = %f \n", gain_max, best.edge_cut, best.workload_diff);
	}
	else {
		printf("There is no better combination than us!!!\n");
	}
	newline();
	printf("ADJ_SIZE = %d, set_edge = %d \n", ADJ_SIZE, set_edge);
	printf("show our sets again. \n");
	show_int_series(set_edge, KL_re.set1, "ours.set1");
	show_int_series(set_edge, KL_re.set2, "ours.set2");
	printf("ours.edge_cut = %f, ours.workload_diff = %f \n", ours.edge_cut, ours.workload_diff);
	newline();
	printf("count_all = %lld, count_better = %lld, ADJ_SIZE = %d, Cmn / 2 = %lld \n", count_all, count_better, ADJ_SIZE, Cmn(new_edge, set_edge) / 2);
	//printf("count_edge = %lld, count_workload = %lld \n", count_edge, count_workload);
	free(better_sets);
	free(better);
	#endif
	
	rank_re.rank_rating = ((double) count_better) / ((double) count_all);
	rank_re.KL_time = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
	free(set1);
	free(set2);
	return rank_re;
	
	
	
}
double score_cmp(struct score old, struct score new) {
	double alpha = KL_ALPHA;
	double gain_edge_cut = 0;
	double gain_workload_diff = 0;
	double gain_all = 0;
	gain_edge_cut = ((double) (old.edge_cut - new.edge_cut) )/ ((double) old.edge_cut);
	gain_workload_diff = ((double) (old.workload_diff - new.workload_diff) )/ ((double) old.workload_diff);
	gain_all = alpha * gain_edge_cut + (1 - alpha) * gain_workload_diff;
	return gain_all;
}


struct score get_score(int new_edge, double adj_array[new_edge][new_edge], double point_weight[new_edge], int set_edge, int set1[set_edge], int set2[set_edge]) {
	int i = 0;
	int j = 0;
	
	double edge_cut = 0;
	double workload_diff = 0;
	
	//edge_cut
	for(i = 0;i < set_edge;i++) {
		for(j = 0;j < set_edge;j++) {
			//printf("edge_cut = %f + adj_array[%d][%d] = %f = ", edge_cut, set1[i], set2[j], adj_array[set1[i]][set2[j]]);
			edge_cut += adj_array[set1[i]][set2[j]];
			//printf("edge_cut = %f \n", edge_cut);
		}
	}
	
	//workload_diff
	for(i = 0;i < set_edge;i++) {
		//printf("workload_diff = %f + point_weight[%d] = %f - point_weight[%d] = %f = ", workload_diff, set1[i], point_weight[set1[i]], set2[i], point_weight[set2[i]]);
		workload_diff = workload_diff + point_weight[set1[i]] - point_weight[set2[i]];
		//printf("workload_diff = %f \n", workload_diff);
	}
	struct score re;
	re.edge_cut = edge_cut;
	re.workload_diff = workload_diff > 0?workload_diff:-workload_diff;
	return re;

}



double random_percentage() {
	if(srand_first_time == 0) {
		srand_first_time = 1;
		srand((int) time(NULL));
	}
	double re = 0;
	double rd = (double) rand();
	re = rd / ((double) RAND_MAX);
	return re;
}

int range_rand(int range) {
	if(srand_first_time == 0) {
		srand_first_time = 1;
		srand((int) time(NULL));
	}
	if(range <= 0) {
		return -1;
	}
	int re = 0;
	while(re == 0) {
		re = rand() % range;
	}
	return re;
}



int next_set(int set_edge, int* set1, int* set2, int old_edge) {
	int i = set_edge - 1;
	int set = 1;
	int j = 1;
	for(;i > 0;i--) {
		//printf("i = %d, (*set1)[%d] = %d, j = %d, old_edge - j = %d \n", i, i, (*set1)[i], j, old_edge - j);
		if(set1[i] < old_edge - j) {//j means the number of elements will be changed.
			int k = 0;

			set1[i] += 1;
			for(k = 1;i + k < set_edge;k++) {//modify set1
				set1[i + k] = set1[i] + k;
			}

			for(k = 0;k < set_edge;k++) {
				if(set2[k] == set1[i]) {
					set2[k] -= 1;//modify set2
					for(i = set_edge - 1;i > k;i--) {//fill the set2 latter part
						set2[i] = set_edge + i;
					}
					break;
					
				}
			}

			return 1;
			break;
		}
		j++;
	
	}
	#ifdef PRINT_MODE
	printf("there is no more new series.\n");
	#endif
	return 0;//no more new sets.
}

long long fact(int n) {
	if(n < 0) {
		printf("wrong arguement n: %d in function fact !!!\n", n);
		return 0;
	}
	int i = 1;
	long long re = 1;
	for(i = 2;i <= n;i++) {
		re *= ((long long) i);
	}
	return re;
}

long long Cmn(int m, int n) {
	long long re = 1;
	int i = 0;
	for(i = 0;i < n;i++) {
		re = re * (m - i);
	}
	re /= fact(n);
}

void printstar(void) {
	printf("*******************************************\n");
}

void newline(void) {
	printf("\n");
}
