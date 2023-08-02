#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/resource.h>

#define MAX_PRIME 100

struct thread_args
{
    int x;
    int p;
    int a;
    int b;
    int thread_idx;
    int thapx;
};

void proc_exit()
{
    int status;
    pid_t pid;

    while (1)
    {
        pid = wait3(&status, WNOHANG, (struct rusage *)NULL);
        if (status == 0)
            return;
        else if (status == -1)
            return;
        else
        {
            printf("\n---------------------------------------------------------------------------------------------------------------\n");
            fprintf(stderr, "Process with PID : %d exited with return value : %d\nTerminating all processes!", pid, status);
            kill(0, 9);
        }
    }
}

// function to determine if a number is prime or not
int is_prime(int num)
{
    if (num <= 1)
        return 0;

    for (int i = 2; i <= sqrt(num); i++)
    {
        if (num % i == 0)
            return 0;
    }

    return 1;
}

// function to calculate thapx
void *calculate_thapx(void *args)
{
    // destructuring values from the struct passed as a parameter to the thread
    struct thread_args *par = args;
    int x = par->x;
    int p = par->p;
    int a = par->a;
    int thread_idx = par->thread_idx;
    int b = par->b;

    // checkpoint 5: thread starts executing
    printf("\n---------------------------------------------------------------------------------------------------------------\n");
    printf("Thread #%d started executing", thread_idx);

    // checking if the current element (x) is within bounds or not
    if (x < a || x > b)
    {
        printf("\n---------------------------------------------------------------------------------------------------------------\n");
        printf("Element %d is outside the bounds: [%d, %d]. Terminating the thread and sending exit code of 1 through the SIGCHLD signal.\n", x, a, b);
        pthread_exit(NULL);
    }

    // initializing the px array, and the before and after arrays to store prime numbers
    int count = 0, px[200];
    int before[100];
    int after[100];

    // checkpoint 6: adding prime numbers to the px array and reporting the addition of each number
    printf("\n---------------------------------------------------------------------------------------------------------------\n");
    printf("For the number %d, adding these prime numbers to the px array => ", x);

    // fetching p prime numbers before x
    for (int i = x - 1; i >= 2; i--)
    {
        if (is_prime(i))
        {
            printf("%d ", i);
            before[count++] = i;
            if (count == p)
                break;
        }
    }
    int beforelen = count;

    // resetting count and fetching p prime numbers after x
    count = 0;
    for (int i = x + 1;; i++)
    {
        if (is_prime(i))
        {
            printf("%d ", i);
            after[count++] = i;
            if (count == p)
                break;
        }
    }
    int afterlen = count;

    for (int j = 0; j < beforelen; j++)
        px[j] = before[j];
    for (int j = 0; j < afterlen; j++)
        px[j + beforelen] = after[j];

    int totalen = beforelen + afterlen;
    if (is_prime(x))
    {
        printf("%d", x);
        px[totalen++] = x;
    }

    int thapx_temp = 0;
    // checkpoint 7: px has been calculated, printing the px array (and calculating thapx_temp for calculating thapx later on)
    printf("\n---------------------------------------------------------------------------------------------------------------\n");
    printf("px array for the element %d => [", x);
    for (int i = 0; i < totalen; i++)
    {
        thapx_temp += px[i];
        if (i == totalen - 1)
            printf("%d", px[i]);
        else
            printf("%d, ", px[i]);
    }
    printf("]");

    // calculating thapx and putting it on the heap as a member of the struct defined earlier
    int thapx = thapx_temp / totalen;
    par->thapx = thapx;

    // exiting the thread
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    // checking if there are enough arguments
    if (argc < 5)
    {
        printf("Not enough arguments. Please provide n, a, b, p and arr.\n");
        return 1;
    }

    // converting the first four arguments to integers
    int n = atoi(argv[1]);
    int a = atoi(argv[2]);
    int b = atoi(argv[3]);
    int p = atoi(argv[4]);

    // allocating memory for the array
    int **arr = (int **)malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++)
    {
        arr[i] = (int *)malloc(n * sizeof(int));
    }

    // converting the rest of the arguments to integers and saving them in the array
    int arg_index = 5;
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            arr[i][j] = atoi(argv[arg_index]);
            arg_index++;
        }
    }

    // checkpoint 1: inputs taken
    printf("\n---------------------------------------------------------------------------------------------------------------\n");
    printf("Inputs taken: n = %d, a = %d, b = %d, p = %d, arr = [ ", n, a, b, p);
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            printf("%d ", arr[i][j]);
        }
        if (i != n - 1)
            printf(", ");
    }
    printf("]");

    int wpapx_arr[n];

    // creating n processes
    for (int i = 0; i < n; i++)
    {
        int p1[2]; // file descriptor used to store two ends of pipe
        pipe(p1);

        // checkpoint 2.1: pipe either created/failed
        if (pipe(p1) == -1)
        {
            printf("\n---------------------------------------------------------------------------------------------------------------\n");
            fprintf(stderr, "Pipe failed");
            return 1;
        }
        else
        {
            printf("\n---------------------------------------------------------------------------------------------------------------\n");
            printf("Pipe created");
        }

        signal(SIGCHLD, proc_exit);
        pid_t pid = fork();

        // checkpoints 2.2, 3: worker process created, reports the row it is processing as the worker number (eg: worker #1 processes row 1)
        if (pid < 0)
        {
            printf("\n---------------------------------------------------------------------------------------------------------------\n");
            fprintf(stderr, "Fork failed");
            exit(1);
        }

        else if (pid == 0)
        {
            printf("\n---------------------------------------------------------------------------------------------------------------\n");
            printf("Worker process #%d created", i);

            // closing the read end of the pipe
            close(p1[0]);

            void *thapx;
            int thapx_arr[n];
            pthread_t thread_id;

            // creating n threads
            for (int j = 0; j < n; j++)
            {
                // checkpoints 4: creation of jth thread
                printf("\n---------------------------------------------------------------------------------------------------------------\n");
                printf("Worker thread #%d created for worker process #%d", j, i);

                // creating arguments and attributes for the thread
                struct thread_args args = {arr[i][j], p, a, b, j, INT_MIN};
                pthread_attr_t attr;
                pthread_attr_init(&attr);

                // creating the thread
                pthread_create(&thread_id, NULL, calculate_thapx, &args);

                // joining/closing the thread
                pthread_join(thread_id, NULL);

                // checking if the thread closed without calculating thapx, meaning that an element was out of bounds
                if (args.thapx == INT_MIN)
                {
                    exit(1);
                }

                // storing the thapx value reported by thread in memory
                thapx_arr[j] = args.thapx;

                // checkpoint 8: printing thapx value calculated by jth thread
                printf("\n---------------------------------------------------------------------------------------------------------------\n");
                printf("thapx value for %d => ", arr[i][j]);
                printf("%d", args.thapx);
            }

            int wpapx_temp = 0;
            // checkpoint 10: all values of thapx have been calculated, printing the thapx array (also summing up all values to create wpapx_temp to later calculate wpapx)
            printf("\n---------------------------------------------------------------------------------------------------------------\n");
            printf("thapx array => [");
            for (int i = 0; i < n; i++)
            {
                wpapx_temp += thapx_arr[i];
                if (i == n - 1)
                    printf("%d", thapx_arr[i]);
                else
                {
                    printf("%d, ", thapx_arr[i]);
                }
            }
            printf("]");

            // checkpoint 11: calculating wpapx and printing it
            int wpapx = wpapx_temp / n;
            printf("\n---------------------------------------------------------------------------------------------------------------\n");
            printf("wpapx calculated for row #%d => %d", i, wpapx);

            // using the write end of pipe to send wpapx value to parent process of this worker
            write(p1[1], &wpapx, sizeof(wpapx));
            close(p1[1]);

            // checkpoint 12: wpapx written to controller/parent
            printf("\n---------------------------------------------------------------------------------------------------------------\n");
            printf("Worker #%d sent wpapx value (%d) to parent\n", i, wpapx);

            exit(0);
        }

        // parent process
        else
        {
            // closing the write end of the pipe
            close(p1[1]);

            int wpapx;

            // waiting for the worker process to finish
            // wait(NULL);

            // reading the wpapx value from the worker using the pipe
            read(p1[0], &wpapx, sizeof(wpapx));

            // checkpoint 13: wpapx read by controller/parent
            printf("\n---------------------------------------------------------------------------------------------------------------\n");
            printf("Parent process #%d received wpapx value (%d) sent by worker", i, wpapx);

            // storing the wpapx value to an array to eventually contain all wpapx values
            wpapx_arr[i] = wpapx;
            close(p1[0]);
        }
    }

    int fapx_temp = 0;
    // checkpoint 14.1: printing the wpapx array containing all wpapx values, (and calculating fapx_temp for calculating fapx later on)
    printf("\n---------------------------------------------------------------------------------------------------------------\n");
    printf("wpapx array => [");
    for (int i = 0; i < n; i++)
    {
        fapx_temp += wpapx_arr[i];
        if (i == n - 1)
        {
            printf("%d", wpapx_arr[i]);
        }
        else
        {
            printf("%d, ", wpapx_arr[i]);
        }
    }
    printf("]");

    // checkpoint 14.2: calculating and printing fapx
    printf("\n---------------------------------------------------------------------------------------------------------------\n");
    int fapx = fapx_temp / n;
    printf("fapx => %d", fapx);
}
