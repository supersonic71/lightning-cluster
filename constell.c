#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<time.h>
/* Goals of the constellation project : 
   	To create multiple lightning-c (lnc) nodes
  	To fund them
   	To connect them
   	To establish channels between them
   	To transact between them
*/
struct lnc{ // lnc = lightning-c node
	int alias;
	int npeers;
	char id[300];
	struct lnc *peer[10]; 
};

void fund(char full_node_path[], int amount); //TODO modify to use lightning_cli
char* lightning_cli(char node_path[], int alias);
int roll(int below);
void establish_channel(char node_path[], struct lnc from, struct lnc to, int sat);

int main(int argc, char *argv[]){ // ./a.out <number of cln nodes>
	srand(time(NULL));

	if(argc != 2)
		return 1;
	int n_nodes; // number of cln nodes
	char *arg_trail; //anything after the integer 
	n_nodes = strtol(argv[1], &arg_trail, 10); // number of cln nodes

	if(!n_nodes)
		return 2;  //it is not a number or user entered 0

//	printf("%d", n_nodes);

	
	char base_lightningd[300] = "lightning/lightningd/lightningd"
	       			    " --network=regtest"
				    " --log-level=debug"
				    " --lightning-dir";
	char node_path[30] = " ~/sim/cln/"; //TODO: REMOVE SPACE
	strcat(base_lightningd, node_path);

	char terminal_lightningd[400];


	int i; //for loops, yes thats a pun

	char terminal_folder[100];	

	for(i = 0; i < n_nodes; i++){ //create folder, start lightning-d
		sleep(1);
		/* Create folder for node */
		sprintf(terminal_folder, "mkdir %s%d", node_path, i);
		system(terminal_folder); 
		printf("\n%s", terminal_folder);	
		
		/* Start lightningd for this node */
		strcpy(terminal_lightningd, base_lightningd);
		sprintf(terminal_lightningd, "%s%d %s %d %s %d", terminal_lightningd, i, "--alias", i, "--port", 10000+i ); 
		sprintf(terminal_lightningd, "%s %s %d%s", terminal_lightningd, ">", i, ".log &");
		system(terminal_lightningd);
		printf("\n%s", terminal_lightningd);
		printf("\n");
	}

//	char base_peer_path[200];
//	char peer_path[200]; 
//	strcpy(base_peer_path, path_to_peer);

	//TODO:warn for no bitcoin-qt found
	/* add aliases in file .clnalias
	 * run source .clnalias
	 */
	//TODO :char array size
	// alias lnc<i>='lightning/cli/lightning-cli --lightning-dir ~/sim/cln/<i>'
	FILE *alias = fopen(".clnalias", "w");
	char terminal_alias[300];
	for(i = 0; i < n_nodes; i++){
		sprintf(terminal_alias, "%s%d%s %s%d%s", "alias lnc", i, "=\'lightning/cli/lightning-cli --lightning-dir", node_path, i, "\'");
		printf("%s", terminal_alias);
		fprintf(alias, "%s\n", terminal_alias);
		printf("\n");
	}
	fclose(alias);
	sleep(1);
	/* fund nodes */
	char full_node_path[200];
	for(i = 0; i < n_nodes; i++){
		strcpy(full_node_path, node_path);
		sprintf(full_node_path, "%s%d", node_path, i);
		fund(full_node_path, 1); // 1 xbt  
		printf("\n");
	} 
	printf("\n");

	//get id	
	struct lnc nodes[100]; // TODO find better number
	char id[300];
	FILE *jq_id;
	char terminal_id[300];

	for(i = 0; i < n_nodes; i++){
		nodes[i].alias = i; // for now
		strcpy(terminal_id, lightning_cli(node_path, i));	
		strcat(terminal_id, " getinfo | jq --raw-output .id");
		jq_id = popen(terminal_id, "r");
		fscanf(jq_id, "%s", id);
		strcpy(nodes[i].id, id);
	}
	fclose(jq_id);

	for(i = 0; i < n_nodes; i++){
		printf("%s\n", nodes[i].id);
	}


	/* Connecting nodes
	 * wave #1 - 100 nodes
	 * each node tries to open 3 channels with other nodes
	 * run stats to see routes
	 * send transactions
	 * use eclair on 29000 to export .dot and chart it
	 * check ram usage
	 *
	 */
	int attempt;
	int dice;
	int flag;
	int j;
	struct lnc peer;
	for(i = 0; i < n_nodes; i++)
		nodes[i].npeers = 0;
	for(i = 0; i < n_nodes; i++){
		attempt = 0;
		while(attempt < 10 && nodes[i].npeers <= 3){
			attempt++;
			flag = 0;
			dice = roll(n_nodes);
			if(i == dice)
				continue;

			for(j = 0; j < nodes[i].npeers; j++){
				if(nodes[i].peer[j]->alias == dice)
					flag = 1;
			}
			for(j = 0; j < nodes[dice].npeers; j++){
				if(nodes[dice].peer[j]->alias == i)
					flag = 1;
			}
			if(flag == 0){
				nodes[i].peer[ nodes[i].npeers++ ] = &nodes[dice];	
				establish_channel(node_path, nodes[i], nodes[dice], 1000000); //1mbtc
				printf("\n%d -> %d\n", i, dice);
			}
			
			// connect, push amount TODO
		}


	}
	return 0;
}

char* lightning_cli(char node_path[], int alias){
	static char full[200]; //how does this static work? well TODO
	sprintf(full, "%s %s%d", "lightning/cli/lightning-cli --lightning-dir", node_path, alias);
	return full;
}
int roll(int below){
	return rand() % below;


}
void establish_channel(char node_path[], struct lnc from, struct lnc to, int sat){
	char terminal_connect[200];
	char terminal_fundchannel[200];
	strcpy(terminal_connect, lightning_cli(node_path, from.alias));

	strcpy(terminal_fundchannel, terminal_connect);
	sprintf(terminal_connect, "%s %s %s %s %d", terminal_connect, "connect", to.id, "127.0.0.1", to.alias+10000);
	system(terminal_connect);
	printf("\n%s", terminal_connect);

	sprintf(terminal_fundchannel, "%s %s %s %d", terminal_fundchannel, "fundchannel", to.id, sat);
	system(terminal_fundchannel);
	printf("\n%s", terminal_fundchannel);

}

void fund(char full_node_path[], int amount){ 
	char terminal_address[200];
	char terminal_sendto[200];
	char terminal_getraw[200];
	char terminal_addfunds[200];
	char address[100];
	char txid[1000];
	char rawtx[1000];
	
	//TODO: improve by using a function to change node number to full expanded command	
	sprintf(terminal_address, "lightning/cli/lightning-cli --lightning-dir%s %s %s", full_node_path, "newaddr", " | jq --raw-output .address");
	printf("%s\n", terminal_address);
	FILE *terminal_op = popen(terminal_address, "r");	

	fscanf(terminal_op, "%s", address);
	fclose(terminal_op);

	
	sprintf(terminal_sendto, "%s%s %d", "bitcoin-cli -regtest sendtoaddress ", address, amount);	
	printf("\n%s", terminal_sendto);
	terminal_op = popen(terminal_sendto, "r");	
	fscanf(terminal_op, "%s", txid);
	fclose(terminal_op);

	sprintf(terminal_getraw, "%s%s", "bitcoin-cli -regtest getrawtransaction ", txid);
	printf("\n%s", terminal_getraw);
	terminal_op = popen(terminal_getraw, "r");
	fscanf(terminal_op, "%s", rawtx);
	fclose(terminal_op);

	sprintf(terminal_addfunds, "%s%s %s %s", "lightning/cli/lightning-cli --lightning-dir", full_node_path, "addfunds", rawtx);
	printf("\n%s", terminal_addfunds);
	terminal_op = popen(terminal_addfunds, "r");
	fclose(terminal_op);
	
} 





























