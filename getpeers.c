#include<stdio.h>
#include<string.h>
#include<stdlib.h>
int main(int argc, char *argv[]){
	if(argc != 2)
		return 1;
	char terminal_getpeers[500];
	char terminal_npeers[500];

	char *arg_trail;
	int n_nodes = strtol(argv[1], &arg_trail, 10);
	int n_peers;
	int i, j;

	FILE *f_n_peers;

	for(i = 0; i < n_nodes; i++){
		printf("Node %d\n", i);
		sprintf(terminal_npeers, "lightning/cli/lightning-cli --lightning-dir=/home/ephraim/sim/cln/%d getpeers | jq .peers | jq length", i);	
		f_n_peers = popen(terminal_npeers, "r");
		fscanf(f_n_peers, "%d", &n_peers);
			
		for(j = 0; j < n_peers; j++){
			sprintf(terminal_getpeers, "lightning/cli/lightning-cli --lightning-dir=/home/ephraim/sim/cln/%d getpeers | jq .peers[%d].connected;", i, j);
			system(terminal_getpeers);
		}
		printf("\n");
	}
	 	
}
