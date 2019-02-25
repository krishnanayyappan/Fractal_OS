#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <string.h>
#include <signal.h>

#define MAX_COMMAND_SIZE 255
#define WHITESPACE " \t\n"

int runfork = 0;

int main(int argc, char* argv[])
{
//char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE);
  char *cmd_str;
  cmd_str = (char*) malloc( 100 );
  memset(cmd_str, 0, 100 );
  double s_val = 2;
  int i = 1, status = 0;

  int forknum = atoi(argv[1]);

  struct timeval start, stop;
      /* Parse input */
    char *token[100];
    int index;
    for( index = 0; index < 100 ; index++ )
    {
      token[i] = NULL;
    }

    int   token_count = 0;
    pid_t forkval;

    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;

    gettimeofday(&start, NULL);

// while (i<=50)
   for (i=1; i<=50; i++)
   {
     //cmd_str == "";
     snprintf(cmd_str, 100,"mandel -x 0.286932 -y 0.014287 -s %f -m 2000 -o mandel%d.bmp", s_val, i);
#if 1
    char *working_str  = strdup( cmd_str );
      // Tokenize the input stringswith whitespace used as the delimiter
    token_count = 0;
    while ( ( (arg_ptr = strsep(&working_str, " " ) ) != NULL) )
    {
      token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }
#endif

    if ( runfork < forknum )
    {
     runfork++; i++;

     forkval = fork();
     if ( forkval < 0 )
     {
       printf("\nError. Exiting...\n");
       fflush(NULL);
       exit(0);
     }
     else if ( forkval == 0 )
     {
    // int execval = execvp ("./mandel", token);
       int execval = execvp ("./mandel", &cmd_str);
           if (execval == -1)
           {
             perror("Error: ");
             exit (0);
           }
     /*    else
           {
             exit (EXIT_SUCCESS);
           } */
     }
     else
     {
       int status;
       wait( &status );
       runfork--;
     }
     s_val = s_val/1.19111;
   }
   }

    gettimeofday(&stop, NULL);
    printf("\nTime taken = %lu\n", ( stop.tv_sec - start.tv_sec ) * 1000000 + (stop.tv_usec - start.tv_usec));

 return 0;
}
