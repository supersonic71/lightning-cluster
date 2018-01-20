#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<time.h>
/* Creates a cluster of c-lightning nodes 
 * This program aims 
   	To create multiple c-lightning (cln) nodes 
  	To fund them
	To establish connections and channels between them FIXME: separate connections(only gossip) and channels
   	To transact between them
	Simulate the above when blocks are full, with realtime main-net data
*/
//A given cluster has one node_path
struct cln{ //cln stands for a c-lightning node 
	char alias[40]; //A non-unique name given to the node
	char id[300]; //A unique way to refer to a node
	int short_id; //Short hand notation for a id for a given cluster
		      //lightning-dir, bash aliases are based on this as it is unique for a cluster
	
	//Peer data is required as c-lightning only supports one channel for given two peers
	//TODO: modify after c-lightning supports multiple channels
	int npeers;
	struct cln *peer[10]; 
};

void fund(char node_path[], int short_id, int amount); //TODO modify to use lightning_cli
char* lightning_cli(char node_path[], int alias);
int roll(int below);
void establish_channel(char node_path[], struct cln from, struct cln to, int sat);
void read_alias(char alias_list[200][32]){
	FILE *aliases = fopen("aliases", "r");
		int i = 0, j = 0;
		int ch;
	
		while( (ch = fgetc(aliases) ) != EOF) {
			if(ch == '\n'){
				if(j>32-1)
					alias_list[i][32-1] = '\0';
				else
					alias_list[i][j] = '\0';
				i++;
				j=0;
				continue;
			}
			alias_list[i][j++] = ch;
		}
		fclose(aliases);
}
int main(int argc, char *argv[]){ // cluster <number of cln nodes>

	if(argc != 2)
		return 1;
	int n_nodes; // number of cln nodes
	char *arg_trail; //anything after the integer 
	n_nodes = strtol(argv[1], &arg_trail, 10); // number of cln nodes

	if(!n_nodes)
		return 2;  //it is not a number or user entered 0

//	printf("%d", n_nodes);

	srand(time(NULL)); //for automatic channel creatiion between nodes
	
		
	char base_lightningd[300] = "lightning/lightningd/lightningd" //FIXME : constant
	       			    " --network=regtest"
				    " --log-level=debug"
				    " --lightning-dir ";
	char node_path[30] = "~/sim/cln/"; //FIXME : constant 
	strcat(base_lightningd, node_path);



	int i; //for loops, yes thats a pun
	char terminal_lightningd[400];
	char terminal_folder[100];	
	char alias_list[200][32];
	read_alias(alias_list);
	for(i = 0; i < n_nodes; i++ ){ //i is same as short_id 
		sleep(1); // Or else c-lightning crashes
		/* Create folder for node */
		sprintf(terminal_folder, "mkdir %s%d", node_path, i);
		system(terminal_folder); 
		printf("\n%s", terminal_folder);	
		/* Start lightningd for this node */
		strcpy(terminal_lightningd, base_lightningd);
		sprintf(terminal_lightningd, "%s%d %s '%s' %s %d", terminal_lightningd, i, "--alias", alias_list[roll(70)] , "--port", 10000+i ); 
		sprintf(terminal_lightningd, "%s %s %d%s", terminal_lightningd, ">", i, ".log &");
		system(terminal_lightningd);
		printf("\n%s", terminal_lightningd);
		printf("\n"); 
		//FIXME : system() doesn't get executed in order in order
	}
	//TODO:warn for no bitcoin-qt found
	
	/* Create bash aliases for easy access to each node from the terminal */	
	// alias cln<i>='lightning/cli/lightning-cli --lightning-dir=~/sim/cln/<i>'
	
	FILE *alias = fopen(".clnalias", "w");
	char terminal_alias[300];

	int short_id;
	for(short_id = 0; short_id < n_nodes; short_id++){ //i stands for short_id
		sprintf(terminal_alias,  "alias cln%d=\'%s\'", short_id, lightning_cli(node_path, short_id));
	//	printf("%s", terminal_alias);
		fprintf(alias, "%s\n", terminal_alias);
	//	printf("\n");
	}
	fclose(alias);

	sleep(1);


	struct cln cluster[200]; 
	char id[300];
	FILE *jq_id;
	char terminal_id[300];
	//fill up cluster structure array with info
	for(i = 0; i < n_nodes; i++){

		cluster[i].short_id = i; 

		strcpy(terminal_id, lightning_cli(node_path, i));	
		strcat(terminal_id, " getinfo | jq --raw-output .id");
		//printf("\n%s\n", terminal_id);
		jq_id = popen(terminal_id, "r");
		fscanf(jq_id, "%s", id);
		strcpy(cluster[i].id, id);

		//TODO aliases
	}
	fclose(jq_id);

	for(i = 0; i < n_nodes; i++){
		printf("%d : %s\n", i, cluster[i].id);
	}

	/* fund nodes */
	
	for(short_id = 0; short_id < n_nodes; short_id++){//short-id
		printf("\nFunding %d...", short_id);
		fund(node_path, short_id, 1); // 1 xbt  
		
	} 
	sleep(1);

	system("bitcoin-cli --regtest generate 6");
	sleep(1);
	int attempt;
	int dice;
	int flag;
	int j;
	struct cln peer;
	int max_peers = 3;
	for(i = 0; i < n_nodes; i++)
		cluster[i].npeers = 0;
	
	/*
	 User enters connections, like
	 * 1 2 3 5
	 * which creates channel from 1 to 2, 2 to 3, 3 to 5
	 */
	/*	
	int from, to;
	scanf("%d", &from);
	while( scanf("%d", &to)  ){
		if(to == -1){
			scanf("%d", &from);
			continue;
		}
		establish_channel(node_path, cluster[from], cluster[to], 1000000);
		from = to;	
	}
	*/
	int n_peers;	
	for(i = 0; i < n_nodes; i++){
		attempt = 0;
		n_peers = roll(max_peers + 1);
		while(attempt < 10 && cluster[i].npeers <= n_peers) {
			attempt++;
			flag = 0;
			dice = roll(n_nodes);
			if(i == dice)
				continue;

			for(j = 0; j < cluster[i].npeers; j++){
				if(cluster[i].peer[j]->short_id == dice)
					flag = 1;
			}
			for(j = 0; j < cluster[dice].npeers; j++){
				if(cluster[dice].peer[j]->short_id == i)
					flag = 1;
			}
			if(flag == 0){
				cluster[i].peer[ cluster[i].npeers++ ] = &cluster[dice];	
				establish_channel(node_path, cluster[i], cluster[dice], 1000000); //1mbtc
				printf("\n%d -> %d\n", i, dice);
			}
			
		}
		printf("\n");


	}
	
	return 0;
}

char* lightning_cli(char node_path[], int short_id){
	static char cli[200]; //how does this static work? well TODO 
	sprintf(cli, "lightning/cli/lightning-cli --lightning-dir %s%d", node_path, short_id);
	return cli;
}
int roll(int below){
	return rand() % below;


}
void establish_channel(char node_path[], struct cln from, struct cln to, int sat){
	char terminal_connect[200];
	char terminal_fundchannel[200];
	strcpy(terminal_connect, lightning_cli(node_path, from.short_id));

	strcpy(terminal_fundchannel, terminal_connect);
	sprintf(terminal_connect, "%s %s %s %s %d", terminal_connect, "connect", to.id, "127.0.0.1", to.short_id+10000);
	//printf("\n%s\n", terminal_connect);
	system(terminal_connect);

	sprintf(terminal_fundchannel, "%s %s %s %d", terminal_fundchannel, "fundchannel", to.id, sat);
	//printf("\n%s\n", terminal_fundchannel);
	system(terminal_fundchannel);


}

void fund(char node_path[], int short_id, int amount){ 
	char terminal_address[200];
	char terminal_sendto[200];
	char terminal_getraw[200];
	char terminal_addfunds[4000];
	char address[100];
	char txid[1000];
	char rawtx[4000];
	
	//TODO: improve by using a function to change node number to full expanded command	
	sprintf(terminal_address, "%s %s", lightning_cli(node_path, short_id), "newaddr | jq --raw-output .address");
//	printf("%s\n", terminal_address);
	FILE *terminal_op = popen(terminal_address, "r");	

	fscanf(terminal_op, "%s", address);
	fclose(terminal_op);

	
	sprintf(terminal_sendto, "%s%s %d", "bitcoin-cli -regtest sendtoaddress ", address, amount);	//FIXME: a constant
//	printf("\n%s", terminal_sendto);
	terminal_op = popen(terminal_sendto, "r");	
	fscanf(terminal_op, "%s", txid);
	fclose(terminal_op);
	
	sprintf(terminal_getraw, "%s%s", "bitcoin-cli -regtest getrawtransaction ", txid);
//	printf("\n%s", terminal_getraw);
	terminal_op = popen(terminal_getraw, "r");
	fscanf(terminal_op, "%s", rawtx);
	fclose(terminal_op);

	sprintf(terminal_addfunds, "%s %s %s", lightning_cli(node_path, short_id),  "addfunds", rawtx);
//	printf("\n%s", terminal_addfunds);
	terminal_op = popen(terminal_addfunds, "r");
	fclose(terminal_op);
//	printf("\nfunded\n");	
	
} 





























