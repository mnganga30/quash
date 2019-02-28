/**
 * @file execute.c
 * @ edited by Martin
 * @brief Implements interface functions between Quash and the environment and
 * functions that interpret an execute commands.
 *
 * @note As you add things to this file you may want to change the method signature
 */

#include "execute.h"
#include <stdio.h>
#include "quash.h"
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <deque.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#define WRITE_END 1
#define READ_END 0
//set up the queues
IMPLEMENT_DEQUE_STRUCT(pid_queue, pid_t);
IMPLEMENT_DEQUE(pid_queue, pid_t);

// pipe Arrays
static int pipes[2][2];
static int prv_pipe = -1;
static int nex_pipe = 0;

//Set up job structure
typedef struct job_struct
{
  int job_id;
  pid_queue pids;
  char *cmd;
} job_t;

IMPLEMENT_DEQUE_STRUCT(job_queue, job_t);
IMPLEMENT_DEQUE(job_queue, job_t);

job_queue job_q;

bool is_job_queue_made = false;
// Remove this and all expansion calls to it
/**
 * @brief Note calls to any function that requires implementation
 */
//#define IMPLEMENT_ME() 
  fprintf(stderr, "IMPLEMENT ME: %s(line %d): %s()\n", __FILE__, __LINE__, __FUNCTION__)

/***************************************************************************
 * Interface Functions
 ***************************************************************************/

// Return a string containing the current working directory.
char *get_current_directory(bool *should_free)
{
  // TODO: Get the current working directory. This will fix the prompt path.
  // HINT: This should be pretty simple

  char *result = NULL;
  result = getcwd(NULL, 0);

  if (result == NULL)
  {
    perror("failed to get current directory");
  }
  // Change this to true if necessary
  should_free = true;

  return result;
}

// Returns the value of an environment variable env_var
const char *lookup_env(const char *env_var)
{
  // TODO: Lookup environment variables. This is required for parser to be able
  // to interpret variables from the command line and display the prompt
  // correctly
  // HINT: This should be pretty simple
  char *result = getenv(env_var); // get an environment variable

  if (result == NULL)
  {
    perror("failed to get current directory");
  }

  return result;
}

// Check the status of background jobs
void check_jobs_bg_status()
{
  // TODO: Check on the statuses of all processes belonging to all background
  // jobs. This function should remove jobs from the jobs queue once all
  // processes belonging to a job have completed.
  //IMPLEMENT_ME();

  // TODO: Once jobs are implemented, uncomment and fill the following line
  //print_job_bg_complete(curr_job.job_id, curr_job.pid, curr_job.cmd);
}

// Prints the job id number, the process id of the first process belonging to
// the Job, and the command string associated with this job
void print_job(int job_id, pid_t pid, const char *cmd)
{
  printf("[%d]\t%8d\t%s\n", job_id, pid, cmd);
  fflush(stdout);
}

// Prints a start up message for background processes
void print_job_bg_start(int job_id, pid_t pid, const char *cmd)
{
  printf("Background job started: ");
  print_job(job_id, pid, cmd);
}

// Prints a completion message followed by the print job
void print_job_bg_complete(int job_id, pid_t pid, const char *cmd)
{
  printf("Completed: \t");
  print_job(job_id, pid, cmd);
}

/***************************************************************************
 * Functions to process commands
 ***************************************************************************/
// Run a program reachable by the path environment variable, relative path, or
// absolute path
void run_generic(GenericCommand cmd)
{
  // Execute a program with a list of arguments. The `args` array is a NULL
  // terminated (last string is always NULL) list of strings. The first element
  // in the array is the executable
  char *exec = cmd.args[0];
  char **args = cmd.args;

  // TODO: Remove warning silencers
  //(void)exec; // Silence unused variable warning
  //(void)args; // Silence unused variable warning

  // TODO: Implement run generic
  int ifError = execvp(exec, args);
  if (ifError == -1)
  {
    perror("ERROR: Failed to execute program");
  }
}

// Print strings
void run_echo(EchoCommand cmd)
{
  // Print an array of strings. The args array is a NULL terminated (last
  // string is always NULL) list of strings.
  char **str = cmd.args;

  // TODO: Remove warning silencers
  //(void)str; // Silence unused variable warning

  // TODO: Implement echo
  int i = 0;
  while (*(str + i) != NULL)
  {
    fprintf(stdout, "%s\n", str[i]);
    i++;
  }
  fflush(stdout); // Flush the buffer before returning
}

// Sets an environment variable
void run_export(ExportCommand cmd)
{
  // Write an environment variable
  const char *env_var = cmd.env_var;
  const char *val = cmd.val;

  // TODO: Remove warning silencers
  // (void) env_var; // Silence unused variable warning
  // (void) val;     // Silence unused variable warning

  // TODO: Implement export.
  // HINT: This should be quite simple.
  setenv(env_var, val, 1);
}

// Changes the current working directory
void run_cd(CDCommand cmd)
{
  printf("directory start\t%s\n", cmd.dir);
  // Get the directory name
  const char *dir = cmd.dir;
  // Check if the directory is valid
  if (dir == NULL)
  {
    perror("ERROR: Failed to resolve path\n");
  }
  // TODO: Change directoryprintf("directory%c\n",dir );
  char *cwd = get_current_directory(false);
  setenv("OLD_PATH", cwd, 1);

  if (chdir(cmd.dir) < 0)
  {
    printf("Invalid directory\n"); // Incase the user enter a invalid directory
  }
  else
  {
    // Update the PWD environment variable to be the new current working
    // directory and optionally update OLD_PWD environment variable to be the old
    // working directory.
    //printf("new dir:\t%s", dir);
    setenv("PWD", dir, 1);
    // if (setenv("PWD", dir, 1) == -1)
    // {
    //   perror("Error: Unable to change PWD environment variable");
    // }
    //printf("directory finished %s\n", get_current_directory);
  }
  free(cwd);
}

// Sends a signal to all processes contained in a job
void run_kill(KillCommand cmd)
{
  int signal = cmd.sig;
  int job_id = cmd.job;

  // TODO: Remove warning silencers
  (void)signal; // Silence unused variable warning
  (void)job_id; // Silence unused variable warning

  // TODO: Kill all processes associated with a background job
  IMPLEMENT_ME();
}

// Prints the current working directory to stdout
void run_pwd()
{
  // TODO: Print the current working directory
  bool *isFree = false;
  char *dir = get_current_directory(isFree);
  fprintf(stdout, "%s\n", dir);

  // Flush the buffer before returning
  fflush(stdout);
  free(isFree);
}

// Prints all background jobs currently in the job list to stdout
void run_jobs()
{
  // TODO: Print background jobs
  //IMPLEMENT_ME();

  int num_jobs = length_job_queue (&job_q);
	for (int i = 0; i < num_jobs; i++)
	{
		struct Job curr_job = pop_front_job_queue (&jq);
		print_job (curr_job.job_id, curr_job.pid, curr_job.cmd);
		push_back_job_queue (&job_q, curr_job);
	}

  // Flush the buffer before returning
  fflush(stdout);
}

/***************************************************************************
 * Functions for command resolution and process setup
 ***************************************************************************/

/**
 * @brief A dispatch function to resolve the correct @a Command variant
 * function for child processes.
 *
 * This version of the function is tailored to commands that should be run in
 * the child process of a fork.
 *
 * @param cmd The Command to try to run
 *
 * @sa Command
 */
void child_run_command(Command cmd)
{
  CommandType type = get_command_type(cmd);
  switch (type)
  {
  case GENERIC:

    run_generic(cmd.generic);
    break;

  case ECHO:
    run_echo(cmd.echo);
    break;

  case PWD:
    run_pwd();
    break;

  case JOBS:

    run_jobs();
    break;

  case EXPORT:
  case CD:
  case KILL:
  case EXIT:
  case EOC:
    break;

  default:
    fprintf(stderr, "Unknown command type: %d\n", type);
  }
}

/**
 * @brief A dispatch function to resolve the correct @a Command variant
 * function for the quash process.
 *
 * This version of the function is tailored to commands that should be run in
 * the parent process (quash).
 *
 * @param cmd The Command to try to run
 *
 * @sa Command
 */
void parent_run_command(Command cmd)
{
  CommandType type = get_command_type(cmd);

  switch (type)
  {
  case EXPORT:
    run_export(cmd.export);
    break;

  case CD:
    run_cd(cmd.cd);
    break;

  case KILL:
    run_kill(cmd.kill);
    break;

  case GENERIC:
  case ECHO:
  case PWD:
  case JOBS:
  case EXIT:
  case EOC:
    break;

  default:
    fprintf(stderr, "Unknown command type: %d\n", type);
  }
}

/**
 * @brief Creates one new process centered around the @a Command in the @a
 * CommandHolder setting up redirects and pipes where neededun_com
 *
 * @note Processes are not the same as jobs. A single job can have multiple
 * processes running under it. This function creates a process that is part of a
 * larger job.
 *
 * @note Not all commands should be run in the child process. A few need to
 * change the quash process in some way
 *
 * @param holder The CommandHolder to try to run
 *
 * @sa Command CommandHolder
 */
void create_process(CommandHolder holder, job_t *jobs)
{
  // Read the flags field from the parser
  bool p_in = holder.flags & PIPE_IN;
  bool p_out = holder.flags & PIPE_OUT;
  bool r_in = holder.flags & REDIRECT_IN;
  bool r_out = holder.flags & REDIRECT_OUT;
  bool r_app = holder.flags & REDIRECT_APPEND; // This can only be true if r_out
                                               // is true

  // TODO: Remove warning silencers
  //(void)p_in;  // Silence unused variable warning
  //(void)p_out; // Silence unused variable warning
  // (void)r_in;  // Silence unused variable warning
  // (void)r_out; // Silence unused variable warning
  // (void)r_app; // Silence unused variable warning

  //TODO: Setup pipes, redirects, and new process
  if (p_out)
  {
    pipe(pipes[prv_pipe]); // when the the
  }

  pid_t pid;

  pid = fork(); // a fork

  if (pid == 0)
  {
    //child do stuff

    if (r_in)
    {
      FILE *in_file;
      in_file = fopen(holder.redirect_in, "r");
      dup2(fileno(in_file), STDIN_FILENO);
      fclose(in_file);
    }
    if (r_app && r_out)
    {
      FILE *out_file = fopen(holder.redirect_out, "a");
      dup2(fileno(out_file), STDOUT_FILENO);
      fclose(out_file);
    }
    if (r_out)
    {
      FILE *out_file = fopen(holder.redirect_out, "w");
      dup2(fileno(out_file), STDOUT_FILENO);
      fclose(out_file);
    }
    if (p_in)
    {
      dup2(pipes[prv_pipe][READ_END], STDIN_FILENO); // read in from pipes
      close(pipes[prv_pipe][WRITE_END]);             //close pipe
    }
    if (p_out)
    {
      dup2(pipes[prv_pipe][READ_END], STDOUT_FILENO); //output command on the pipes
      close(pipes[nex_pipe][READ_END]);               ///close pipe
    }
    child_run_command(holder.cmd); // This should be done in the child branch of a fork
    exit(0);
  }
  else
  {//Parent do stuff
    if (p_out)
    {
      close(pipes[nex_pipe][READ_END]); //if there is something in hte pipe then close it.
    }
    //printf("before function call. in child cmd:%s\n", holder.cmd);
    push_back_pid_queue(&(jobs->pids), pid);      //push_back_pid_queue(pids, pid);
    parent_run_command(holder.cmd); // This should be done in the parent branch of
  }
}

// Run a list of commands
void run_script(CommandHolder *holders)
{
  if (holders == NULL)
  {
    printf("fail holders is NULL");
    return;
  }

  if (!is_job_queue_made)
  {
    job_q = new_job_queue(1);
    is_job_queue_made = true;
  }

  if (holders == NULL)
  {
    printf("fail holders is NULL");
    return;
  }

  check_jobs_bg_status();

  if (get_command_holder_type(holders[0]) == EXIT &&
      get_command_holder_type(holders[1]) == EOC)
  {
    end_main_loop();
    return;
  }
  job_t curr_job = {.job_id = 0, .pids = new_pid_queue(1), .cmd = get_command_string()};

  CommandType type;
  //create_process(holders[0], &curr_job.pids); //pass in pid for first process (child)
  // Run all commands in the `holder` array
  for (int i = 0; (type = get_command_holder_type(holders[i])) != EOC; ++i)
  {
    //create_process(holders[i]);
    create_process(holders[0], &curr_job); //pass in pid for first process (child)
  }

  if (!(holders[0].flags & BACKGROUND))
  {
    // Not a background Job
    // TODO: Wait for all processes under the job to complete

    while (!is_empty_pid_queue(&curr_job.pids))
    {
      int status;
      pid_t temp_pid = pop_front_pid_queue(&curr_job.pids);
      waitpid(temp_pid, &status, 0);
    }
    destroy_pid_queue(&curr_job.pids);
  }
  else
  {
    // A background job.
    // TODO: Push the new job to the job queue
    if (is_empty_job_queue(&job_q))
    {
      curr_job.job_id = 1;
    }
    else
    {
      curr_job.job_id = peek_back_job_queue(&job_q).job_id + 1;
    }
    push_back_job_queue(&job_q, curr_job); //otherwise assign a new job id
    int job_id = curr_job.job_id;
    pid_t pid = peek_front_pid_queue(&curr_job.pids); // new job is pushed to queue

    //TODO: Once jobs are implemented, uncomment and fill the following line
    print_job_bg_start(job_id, pid, curr_job.cmd);
  }
}
