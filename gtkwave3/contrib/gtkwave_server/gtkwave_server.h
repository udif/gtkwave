#ifndef GTKWAVESERVER_H
#define GTKWAVESERVER_H

/*
 * includes
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h> 
#include <string.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>


/*
 * defines
 */
#define ID if(!terminal_detached)
#define PID_STRLEN_MAX 10		/* max strlen of a pid */
#define until(x) while(!(x))		
#define RPC_MATCHWORD "RPCM"


/*
 * structs
 */
struct safe
{
struct safe *next;
char val;
};


struct gtkwave_rpc_ipc_t
{
char matchword[4];                      /* match against RPC_MATCHWORD string */
unsigned valid : 1;
unsigned resp_valid : 1;
char membuf[65536];
char resp_membuf[65536];
};
  


/*
 * globals
 */
extern pid_t server_pid;
extern char *gtkwaveserver_id;
extern char *comm_dir;   
extern char *home_dir;
extern char fatal_error; 
extern char terminal_detached;

extern int lock_fd;
extern struct flock flock;


/*
 * function protos
 */
void getenv_wrap(const char *, char **);
void fatal_test(void);
void die_quick(void); 
void listen_requests(void);
void spawn_server_fork(int);
void spawn_server(int);
char *saferead(int);
int good_query(char *, int);
void *calloc2(size_t, size_t);
void spawn_gtkwave_fork(unsigned int rpcid);

#endif
