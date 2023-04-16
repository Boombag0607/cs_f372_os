# Projects for Operating System Course

---

## Assignment 1

&rarr; You are given a 2D array of size n\*n, where n is a positive non-zero number and 4 <= n <= 10. Each number (say x) in the 2D array is a positive non-zero number such that a <= x <=b and p is the count of prime numbers on either side of x, we want to accomplish the following complex mathematical operation as follows. The sample data assumes n=4, a=10, b=99, p=5

● Step 1. Read in the input (Say the 2D array is)

| 17  | 28  | 67  | 65  |
| --- | --- | --- | --- |
| 22  | 19  | 11  | 77  |
| 89  | 78  | 45  | 40  |
| 20  | 10  | 90  | 76  |

● Step 2. We fetch p positive prime numbers before & after x, and including x, such that we have
the following set px - <br>
o For x=17; px={3,5,7,11,13,17, 19,23,29,31,37} <br>
o For x=28; px={11,13,17, 19,23,29,31,37,41,43} <br>
o For x=10; px={2,3,5,11,13,17,19,23} <br>

(Note: In this case, there are only 3 prime numbers before x and only these 3 have been included in px. However, we have included p=5 prime numbers after x in the set px. Observe, we do not go less than 0 in the px.)

● Step 3. Calculate average (ignore any decimal portion) of each px, say we call it `thapx`, such that
now we have 1 `thapx` for each x.

▪ For x = 17; px = {3,5,7,11,13,17, 19,23,29,31,37};
`thapx` = 17 <br>
▪ For x = 28; px = {11,13,17, 19,23,29,31,37,41,43};
`thapx` = 26 <br>
▪ For x = 10; px = {2,3,5,11,13,17,19,23}; `thapx` = 11 <br>

● Step 4. Calculate the average (ignore any decimal portion) of n such values of `thapx`, say we call
it `wpapx`

● Step 5. Calculate the average of n such values of `wpapx`, say we call it fapx
The value of fapx is our desired result.

&rarr; The code is a POSIX compliant program in C where the above steps are accomplished in row major approach,
with the following considerations.

1. The program should execute with the following arguments: <prog_name> n a b p e x~1~ x~2~ ... x~n\*n~
   a. Example ./prog_name 4 10 99 5 17 28 67 65 22 19 11 77 89 78 45 40 20 10 90 76 <br>
   i. In the above example, n is 4, a & b are 10 and 99 respectively and p is 5 <br>
   ii. The numbers are the values of x in the 2D array, where the count of such numbers is n^2, that is 16 in this case. <br>

2. The process (say controller/parent/main) executed from <prog_name> will generate the n*n 2D
   array using the command line input. The program should validate the count of x and construct
   the n*n array, however the validation of individual x (that it is in the range of a and b) will be
   done at a later stage by the worker (child) process.

3. The controller will spawn n worker processes in a loop and have a 1-1 pipe communication link
   with the worker processes. Now, the controller will wait for all the n worker processes to finish
   execution.

4. Each worker process i is aware of the index i of the loop when it is forked and will be responsible
   for validating the contents of only row number i of the 2D array. Assume 0-based indexing in this
   case. If any of the row values is invalid, the worker process will report error and terminate.

5. Each worker process i after being spawned will do the following:
   a. Create n threads (say worker threads) where each thread will take a value x from row i
   and
   i. Create the set px, subject to the value of p.
   ii. Calculate the `thapx`
   iii. Report it back to the worker process and terminate the thread gracefully.
   b. The main thread of each worker process i will wait for all spawned worker threads to
   return the `thapx`.
   c. Once all n values of `thapx` are available, and all worker threads have joined, the main
   thread in the worker process i will calculate the `wpapx` and write it back to the controller
   process in the pipe.

6. Next, the controller which was waiting to read/receive n values of `wpapx` from the worker
   processes will get unblocked after receiving all the n values of `wpapx`.

7. If a worker process is terminated before reporting the `wpapx`, the same should be handled by
   the controller by handling the SIGCHLD signal. The controller then will report the error and kill
   and clean up all worker processes.

8. Finally, the controller process will calculate the average of n values of `wpapx` as `fapx` and
   report/print to the console the value of `fapx`.

The program should report the intermediate results/ progress at the following stages -

1. After reading the input. <br>
2. After creating the pipe and the worker process. <br>
3. After the worker process begins execution, it should report the row it is processing. <br>
4. After creation of worker threads. 5. After the worker thread starts executing. <br>
5. At each level of discovery of a prime number. <br>
6. After all, px is calculated. <br>
7. After thapx is calculated. <br>
8. After thread termination / thread join. <br>
9. After all thapx is calculated <br>
10. After wpapx is calculated <br>
11. After wpapx is written to the controller <br>
12. After Controller captures a wpapx <br>
13. After the controller calculates the fapx <br>
14. All invalidation errors. <br>

---

## Assignment 2

A POSIX compliant C program(s) for the following scenario -
&rarr; There is a client server application for message passing and logging, which comprises of 1 server and “n” clients. The applications are meant to be running in a single system and exchange data using stateless communication (i.e., single message block should be passed and completed in a single communication loop). Every request from the clients to the server is replied to the client by the server. Every action is initiated by the client and acted upon by the server. The system follows a request-response mechanism and provides for the following set of actions for the client and server. The client and server communicate with the following actions -

### 1. REGISTER

This is a client-initiated action where the client connects with the server in the server’s
connect channel, for the very first time with a unique name. The server will verify that the
client’s name is unique and unused, and return with a key (string). The server will allocate
the comm channel, shared memory and create the worker thread for the client.
The client may disconnect, but the registration will continue to be active.
The key will be the common shared link for all further communication, which will be used to
communicate further on the comm channel.

### 2. CLIENT_REQUEST

The client can send a request to the server, on the comm channel for the client, which will
be referenced using the key from the register request for the following actions.

a. Arithmetic: This action can take the following values: **Int N1 Int N2 Int Operation (+, -, \*, /)**

b. EvenOrOdd: This action can take the following values: **Int N1**

c. IsPrime: This action can take the following values: **Int N1**

d. IsNegative: **Not supported**

The server will validate the request, with the unique name and key, and the requested
operation, and the respond to the client with the result as appropriate.
The design for this request should be scalable for additional operations.

### 3. SERVER_RESPONSE

The server response may be designed as follows. Students can decide on any alternative
schema.

| Response Code Success: 0 | Custom Error Code:                                                                                            |
| ------------------------ | ------------------------------------------------------------------------------------------------------------- |
| Client Response Seq No   | The counter for the client, for instance a successful registration will have this value as 1 for each client. |
| Server Response Seq No   | The counter for the server for the number of responses already served.                                        |
| Action Specific Response | To be designed by the students                                                                                |

The server will respond to the client’s request, by acting on the request.

### 4. UNREGISTER

At any point of time the client can make this request to the server in the comm channel,
which will be acted up on by the server by cleaning up all resources allocated for the client.
The implementation will be based on the following -

    1. Inter process communication.
    Must be accomplished by using shared memory only.

    2. Shared memory
        a. The server will maintain a connect channel for the connect request of clients
        b. The server will create a comm channel for all successful connect request.

    The shared memory will have three segments, with constant sizes for all channels.
    Differential sizing will complicate you code, but with abundance of caution may be used as
    well.

    MUTEX / RD-WR Lock REQUEST RESPONSE
    Note the server will have n+1 shared memory objects, where n is the number of registered
    clients.

    3. Synchronization
    The students may use busy-waiting along with a mutex or read write lock or condition
    variables.
    This must be achieved using pthread mutex in shared process scope. The server will be
    multithreaded, and each client will be single threaded.

    4. The server implementation should be a simple multithreaded process.

    5. The client implementation should be an interactive menu driven, with the options of client
    request and unregister only.
        a. The implementation should provide for single client operations.
        b. The implementation should provide for multiple client operations concurrently.

The following intermediate states of the server/client must be logged.

1. The server initiates and creates the connect channel.
2. The server receives a register request on the connect channel.
   a. On successful creation of a comm channel for a client
   b. On successful response made to the client’s register request.
3. The server receives an unregister request
   a. On successful cleanup of the comm channel.
4. The server receives a service request on the comm channel.
5. The server responds to the client on the comm channel.
6. The server should maintain and print the summary info.
   a. The list of registered clients
   b. The count of requests serviced for the client. Note that failed requests which are responded to are also counted in.
   c. The total count of requests serviced for all clients.
7. The client makes register request to the server on the connect channel.
8. The client connects to the server on the comm channel.
9. The client sends a request to the server on the comm channel
10. The client receives a response on the comm channel.
11. All error states, and their mitigations/counter actions.
