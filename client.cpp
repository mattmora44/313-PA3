/*
    Based on original assignment by: Dr. R. Bettati, PhD
    Department of Computer Science
    Texas A&M University
    Date  : 2013/01/31
 */


#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <sstream>
#include <iomanip>

#include <sys/time.h>
#include <cassert>
#include <assert.h>

#include <cmath>
#include <numeric>
#include <algorithm>

#include <list>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include "reqchannel.h"
#include "SafeBuffer.h"
#include "Histogram.h"
using namespace std;

pthread_mutex_t mtx;

struct arguments
{
	string name;
	int n;
	int w;
	SafeBuffer* request_buffer;
	Histogram* hist;
};

void* request_thread_function(void* arg) {
	/*
		Fill in this function.

		The loop body should require only a single line of code.
		The loop conditions should be somewhat intuitive.

		In both thread functions, the arg parameter
		will be used to pass parameters to the function.
		One of the parameters for the request thread
		function MUST be the name of the "patient" for whom
		the data requests are being pushed: you MAY NOT
		create 3 copies of this function, one for each "patient".
	 */
	arguments* args = (arguments*) arg;
	string name = args->name;
	int n = args->n;
	int w = args->w;
	string Data_with_name = "data "+ args->name;
	cout <<"pushing: "<< Data_with_name << endl;
    for(int i = 0; i < n; ++i) {
    	pthread_mutex_lock(&mtx);
        args->request_buffer->push(Data_with_name);
        pthread_mutex_unlock(&mtx);
    }
    cout << "Done populating request buffer" << endl;

    return NULL;
}

void* worker_thread_function(void* arg) {
    /*
		Fill in this function. 

		Make sure it terminates only when, and not before,
		all the requests have been processed.

		Each thread must have its own dedicated
		RequestChannel. Make sure that if you
		construct a RequestChannel (or any object)
		using "new" that you "delete" it properly,
		and that you send a "quit" request for every
		RequestChannel you construct regardless of
		whether you used "new" for it.
     */

    while(true) {

    }
}

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

int main(int argc, char * argv[]) {
    int n = 100; //default number of requests per "patient"
    int w = 1; //default number of worker threads
    int opt = 0;
    while ((opt = getopt(argc, argv, "n:w:")) != -1) {
        switch (opt) {
            case 'n':
                n = atoi(optarg);
                break;
            case 'w':
                w = atoi(optarg); //This won't do a whole lot until you fill in the worker thread function
                break;
			}
    }
    // get args ready
    arguments* args = new arguments();
    args->n = n;
    args->w = w;
    SafeBuffer* request_buffer = new SafeBuffer();
	Histogram* hist = new Histogram();
	args->request_buffer = request_buffer;
	args->hist = hist;
	args->name = "John Smith";
	arguments* args1 = new arguments();
    args1->n = n;
    args1->w = w;
	args1->request_buffer = request_buffer;
	args1->hist = hist;
	args1->name = "Jane Smith";
	arguments* args2 = new arguments();
    args2->n = n;
    args2->w = w;
	args2->request_buffer = request_buffer;
	args2->hist = hist;
	args2->name = "Joe Smith";
    int pid = fork();
	if (pid == 0){
		execl("dataserver", (char*) NULL);
	}
	else {

        cout << "n == " << n << endl;
        cout << "w == " << w << endl;

        cout << "CLIENT STARTED:" << endl;
        cout << "Establishing control channel... " << flush;
        RequestChannel *chan = new RequestChannel("control", RequestChannel::CLIENT_SIDE);
        cout << "done." << endl<< flush;

        // request thread

        // create threads
        pthread_mutex_init(&mtx, NULL);
        pthread_t thread1;
        pthread_t thread2;
        pthread_t thread3;
        
        pthread_create(&thread1,NULL,request_thread_function, args);
		pthread_create(&thread2,NULL,request_thread_function, args1);
		pthread_create(&thread3,NULL,request_thread_function, args2);

        pthread_join(thread1, NULL);
        pthread_join(thread2, NULL);
        pthread_join(thread3, NULL);
        cout <<"All threads joined.\n";
		// worker thread

        cout << "Pushing quit requests... ";	    
	    for(int i = 0; i < w; ++i) {
	        args->request_buffer->push("quit");
	    }
	    cout << "done." << endl;

        chan->cwrite("newchannel");
		string s = chan->cread ();
        RequestChannel *workerChannel = new RequestChannel(s, RequestChannel::CLIENT_SIDE);

        while(true) {
            string request = args->request_buffer->pop();
			workerChannel->cwrite(request);

			if(request == "quit") {
			   	delete workerChannel;
                break;
            }else{
				string response = workerChannel->cread();
				args->hist->update (request, response);
			}
        }
        chan->cwrite ("quit");
        delete chan;
        cout << "All Done!!!" << endl; 

		args->hist->print ();
    }
}
