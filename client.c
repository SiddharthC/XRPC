#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <xmlrpc-c/base.h>
#include <xmlrpc-c/client_global.h>
#include <string.h>
#include "config.h"

#define NAME "Multi - XML-RPC C Client"
#define VERSION "1.0"

typedef struct request_array{
	int response_count;
}request_info;

int summation_a=0;

request_info *req_info;
int number_of_servers;

static void
die_if_fault_occurred(xmlrpc_env * const envP) {
    if (envP->fault_occurred) {
        fprintf(stderr, "XML-RPC Fault: %s (%d)\n",
                envP->fault_string, envP->fault_code);
        exit(1);
    }
}

//for time caalculation.
int timeval_subtract (result, x, y)
      struct timeval *result, *x, *y;
 {
   /* Perform the carry for the later subtraction by updating y.*/
   if (x->tv_usec < y->tv_usec) {
     int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
     y->tv_usec -= 1000000 * nsec;
     y->tv_sec += nsec;
   }
   if (x->tv_usec - y->tv_usec > 1000000) {
     int nsec = (y->tv_usec - x->tv_usec) / 1000000;
     y->tv_usec += 1000000 * nsec;
     y->tv_sec -= nsec;
   }
 
 /* Compute the time remaining to wait.
           tv_usec is certainly positive.*/
        result->tv_sec = x->tv_sec - y->tv_sec;
        result->tv_usec = x->tv_usec - y->tv_usec;
 
        /* Return 1 if result is negative.*/
        return x->tv_sec < y->tv_sec;
 }

int check_server()
{
 	FILE *tmp;
	char *mode="r";
	char from_file[5];
	int max_servers;
        tmp = fopen("tmp.info", mode);
        if(tmp)
        {
                fscanf(tmp, "%s", from_file);
                fclose(tmp);
        }
        else
        {
                fprintf(stderr, "Servers not started. Client will terminate.\nPlease read README\n");
                exit(1);
        }
        max_servers = atoi(from_file);
	return max_servers;
}

int 
main_sync(int           const argc, 
     const char ** const argv) {

    	xmlrpc_env env;
	xmlrpc_int32 reqId;
 	const char * const methodName = "server.ping";
	int number_of_requests;
	int number_of_servers_s;
	int max_servers;
	int i;
	long time_in_ms;
	long time_sum=0;
	int summation=0;

	struct timeval starttime, endtime, timediff;

	if(argc == 3)
        {
        	number_of_requests = atoi(argv[1]);
                number_of_servers_s = atoi(argv[2]); 
                for(i=1; i <= number_of_servers_s; i++)
                         summation=summation+i;
		max_servers = check_server();
		if(number_of_servers_s > max_servers)
		{
			fprintf(stderr, "Invalid number of servers\n");
                	exit(1);
		}
        }
        else{
                fprintf(stderr, "This program has two arguments: <number_of_requests> <number_of_servers>\n");
                exit(1);
        }
 

        char **serverUrl;
        serverUrl = malloc(number_of_servers_s * sizeof(char *));
        for(i=0; i<number_of_servers_s; i++)
               serverUrl[i] = malloc(50 * sizeof(char));

        for(i=0; i<number_of_servers_s; i++)
                sprintf(serverUrl[i], "http://localhost:%d/RPC2\0", (8080 + i));


        xmlrpc_value ** resultP = malloc(number_of_servers_s * sizeof(xmlrpc_value *));

        xmlrpc_int *sum = malloc(number_of_servers_s * sizeof(xmlrpc_int));


    	xmlrpc_client_init(XMLRPC_CLIENT_NO_FLAGS, NAME, VERSION);

    	xmlrpc_env_init(&env);


	for(reqId=1; reqId<=number_of_requests; ++reqId)
	{
		gettimeofday(&starttime, 0x0);
    		resultP = xmlrpc_client_call(&env, (const char ** const)serverUrl, number_of_servers_s, methodName,
                                			 "(i)", (xmlrpc_int32) 0, reqId);
		gettimeofday(&endtime, 0x0);
    		die_if_fault_occurred(&env);

		for(i=0; i<number_of_servers_s; i++)
		{
			xmlrpc_read_int(&env, resultP[i], &sum[i]);
			die_if_fault_occurred(&env);
		}

		timeval_subtract(&timediff, &endtime, &starttime);
		time_in_ms = ((&timediff)->tv_sec * 1000) + ((&timediff)->tv_usec / 1000);
		time_sum += time_in_ms;
    	}

	printf("sync|%d|%d|%4.2f\n", number_of_servers_s, (summation * number_of_requests), ((float)time_sum * 50/(float)(number_of_servers_s*number_of_requests)));	
   	for(i=0; i<number_of_servers_s; i++)
    		xmlrpc_DECREF(resultP[i]);

    	xmlrpc_env_clean(&env);
    
    	xmlrpc_client_cleanup();

    	return 0;
}


//////////////////////////////////////////////////////////////////
static void 
handle_ping_response(const char *   const serverUrl,
                const char *   const methodName,
                xmlrpc_value * const paramArrayP,
                void *         const user_data,
                xmlrpc_env *   const faultP,
                xmlrpc_value * const resultP) {

	xmlrpc_env env;
	xmlrpc_int reqId;
	char serverString[5];
	int serverId=0;
	int time_in_ms;
	int i;
    
   	xmlrpc_env_init(&env);

  	xmlrpc_decompose_value(&env, paramArrayP, "(i)", &reqId);
    	die_if_fault_occurred(&env);

	if(req_info[(reqId-1)].response_count < number_of_servers)
	{
		req_info[(reqId-1)].response_count++;
		strncpy(serverString, (serverUrl+17), 4);
		strcat(serverString,"\0");
		serverId = atoi(serverString);
		serverId -= 8079;
		summation_a += serverId;
	}
}


int 
main_async(int           const argc, 
     const char ** const argv) {

    	const char * const methodName = "server.ping";
    	xmlrpc_env env;
    	xmlrpc_client * clientP;
	xmlrpc_int32 reqId;
	int number_of_requests;
	int max_servers;
	int i;
	int time_in_ms;

	struct timeval astart_time, aend_time, atime_diff;

	if(argc == 3)
        {
        	number_of_requests = atoi(argv[1]);
                number_of_servers = atoi(argv[2]); 
		max_servers = check_server();
		if(number_of_servers > max_servers)
		{
			fprintf(stderr, "Invalid number of servers\n");
                	exit(1);
		}
        }
        else{
                fprintf(stderr, "This program has two arguments: <number_of_requests> <number_of_servers>\n");
                exit(1);
        }

        char **serverUrl;
        serverUrl = malloc(max_servers * sizeof(char *));
        for(i=0; i<max_servers; i++)
               serverUrl[i] = malloc(50 * sizeof(char));

        for(i=0; i<max_servers; i++)
                sprintf(serverUrl[i], "http://localhost:%d/RPC2\0", (8080 + i));


	req_info = (request_info *) calloc(number_of_requests, sizeof(request_info));


    	xmlrpc_env_init(&env);

    	xmlrpc_client_setup_global_const(&env);
    	die_if_fault_occurred(&env);

    	xmlrpc_client_create(&env, XMLRPC_CLIENT_NO_FLAGS, NAME, VERSION, NULL, 0,
         	                &clientP);
    	die_if_fault_occurred(&env);

	gettimeofday(&astart_time, 0x0);

    	for (reqId=1; reqId<=number_of_requests; ++reqId) {

        	xmlrpc_client_call_asynch(&env, clientP, (const char ** const)serverUrl, max_servers, methodName,
                                  		handle_ping_response, NULL,
                                  		"(i)", reqId);
        	die_if_fault_occurred(&env);
    	}
    
    	xmlrpc_client_event_loop_finish(clientP);

	gettimeofday(&aend_time, 0x0);

	timeval_subtract(&atime_diff, &aend_time, &astart_time);

	time_in_ms = ((&atime_diff)->tv_sec * 1000) + ((&atime_diff)->tv_usec / 1000);

	printf("async|%d|%d|%4.2f\n", number_of_servers, summation_a, ((float)time_in_ms/(float)(number_of_servers*number_of_requests)));	

    	xmlrpc_client_destroy(clientP);
    	xmlrpc_client_teardown_global_const();

    	return 0;
}

int 
main(int           const argc, 
     const char ** const argv) {

	main_sync (argc, argv);
	main_async(argc, argv);

}

