#include "gtkwave_server.h"


/*
 * globals
 */
pid_t server_pid;

char *gtkwaveserver_id=NULL;
char *comm_dir=NULL;
char *home_dir=NULL;
char fatal_error=0;
char terminal_detached=0;

int lock_fd;
struct flock flock;

unsigned int shmid;
struct gtkwave_rpc_ipc_t *rpc_ctx = NULL;


void selectsleep(int u)
{
struct timeval tv;

tv.tv_sec = 0;
tv.tv_usec = 1000000 / u;
select(0, NULL, NULL, NULL, &tv); 
}


int main(int argc, char **argv)
{
int i;
char *lockfile_name = NULL;
static char *lockprefix="/pid.";

/* collectively report errors w/o dying as much as possible */

getenv_wrap("GTKWAVE_SERVER_ID", &gtkwaveserver_id);
getenv_wrap("GTKWAVE_COMM_DIR", &comm_dir);
getenv_wrap("HOME", &home_dir);

fatal_test();	

/* create child for subsequent setsid() */
server_pid=fork();
if( ((int)server_pid) < 0)
	{
	fprintf(stderr, "Can't fork()\n");
	die_quick();
	}
if(server_pid)
	{
	exit(0);
	}	

server_pid=getpid();

/* create lockfile if it DNE */
umask(022);
mkdir(comm_dir, 0755);

lockfile_name=malloc(strlen(comm_dir)+strlen(lockprefix)+strlen(gtkwaveserver_id)+1);
strcpy(lockfile_name, comm_dir);
strcat(lockfile_name, lockprefix);
strcat(lockfile_name, gtkwaveserver_id);

lock_fd=open(lockfile_name, O_WRONLY|O_CREAT, 0644);
if(lock_fd<0)
	{
	fprintf(stderr, "Could not open '%s' for writing.\n", lockfile_name);
	die_quick();
	}
free(lockfile_name);

/* lock the lockfile if possible, else bail */
flock.l_type=F_WRLCK;
flock.l_whence=0;
flock.l_start=0;
flock.l_len=0;
flock.l_pid=server_pid;
if(!((fcntl(lock_fd, F_SETLK, &flock))==0))
	{
	fprintf(stderr, "Server is already running.\n");
	die_quick();
	}
	else
	{
	char pid[16];
	sprintf(pid,"%d",(int)server_pid);
	ftruncate(lock_fd, 0);
	write(lock_fd, (void *)pid, strlen(pid));
	}

fprintf(stderr,"%d\n", (int)(server_pid));
fflush(NULL);

chdir(home_dir);

/* make a daemon */
terminal_detached=~0;
setsid();		    
umask(0);
/* for(i=0;i<3;i++) close(i); */
close(0);

{
shmid = shmget(0, sizeof(struct gtkwave_rpc_ipc_t), IPC_CREAT | 0600 );
if(shmid >=0)
        {
        struct shmid_ds ds;

        rpc_ctx = shmat(shmid, NULL, 0);
        if(rpc_ctx)
                {
                memset(rpc_ctx, 0, sizeof(struct gtkwave_rpc_ipc_t));
                memcpy(rpc_ctx->matchword, RPC_MATCHWORD, 4);

#ifdef __linux__
                shmctl(shmid, IPC_RMID, &ds); /* mark for destroy */
#endif

		spawn_gtkwave_fork(shmid);
		}
	}

}



/* loop forever on listen requests process because a server's not supposed to die */
for(;;) 
	{
	errno=0; 	/* because we're a server, clear errno on entry                 */
	listen_requests();
	selectsleep(10); /* shouldn't get here unless a fatal error in listen_requests() */
	}		/* sleep() keeps server from going runaway on tons of errors    */

exit(0);
}


/*
 * get environment variables with error checking
 */
void getenv_wrap(const char *env, char **dest)
{
if(!(*dest=getenv(env)))
	{
	fprintf(stderr, "%s environment variable not set\n", env);
	fatal_error=~0;
	}
}


/*
 * multipurpose "i found an error" kill routines
 */
void fatal_test(void)
{
if(fatal_error)
	{
	ID fprintf(stderr, "Errors encountered, exiting.\n");
	exit(1);
	}
}

void die_quick(void)
{
fatal_error=~0;
fatal_test();
}


/*
 * listen for requests on the $COMM_DIR/requests.$GTKWAVESERVER_ID fifo
 * nothing should exit() at all in this function.  Only stuff forked
 * underneat should be allowed that option.
 */
void listen_requests(void)
{
char *listen_fifo_name;
int listen_fd;
static char *reqprefix="/requests.";

listen_fifo_name=(char *)alloca(strlen(comm_dir)+strlen(reqprefix)+strlen(gtkwaveserver_id)+1);
strcpy(listen_fifo_name, comm_dir);
strcat(listen_fifo_name, reqprefix);
strcat(listen_fifo_name, gtkwaveserver_id);

for(;;)
	{
	ID printf("\n\nfifo: %s\n",listen_fifo_name);
	if(((listen_fd=open(listen_fifo_name, O_RDONLY))<0)||(errno))
		{
		ID perror("listen_request()");
		return; /* because we're a server */
		}

	ID printf("Listening..\n");

	{
	char fifo_read[PID_STRLEN_MAX];
	int pnt;
	int comm_pid;

	pnt=-1;

	do 	{
		read(listen_fd, &fifo_read[++pnt], 1);
		} until(((fifo_read[pnt]=='\n')||(pnt==PID_STRLEN_MAX)));

	fifo_read[pnt]=0;

	comm_pid=atoi(fifo_read);
	ID printf("len: %d, '%s'\n",strlen(fifo_read), fifo_read);
	close(listen_fd);
	/* if(comm_pid) spawn_server_fork(comm_pid); */
	if(comm_pid) spawn_server(comm_pid);
	}
	}
}


/*
 * the double-fork (for zombie prevention)
 */
void spawn_gtkwave_fork(unsigned int rpcid)
{
pid_t pid;
char buf[17]; 
char *arglist[4];

pid=fork();
if( ((int)pid) < 0) { return; /* not much we can do about this.. */ }

if(pid)         /* 1st parent==original server_pid */
        {
        int stat;

        wait(&stat);    /* returns immediately as child immediately dies */
        return;
        }

pid=fork();
if( ((int)pid) < 0) { exit(0); /* not much we can do about this.. */ }

if(pid)         /* 2nd parent==process which deliberately dies */
        {
        exit(0);
        }

sprintf(buf, "%x", rpcid);

arglist[0] = "gtkwave";
arglist[1] = "-1";
arglist[2] = buf;
arglist[3] = NULL;

execvp(arglist[0], arglist);

exit(0);
}


/*
 * the double-fork (for zombie prevention) ... currently unused
 */
void spawn_server_fork(int pid_fifo)
{
pid_t pid;

pid=fork();  
if( ((int)pid) < 0) { return; /* not much we can do about this.. */ }

if(pid)		/* 1st parent==original server_pid */
	{
	int stat;

	wait(&stat);	/* returns immediately as child immediately dies */
	return;
	}

pid=fork();  
if( ((int)pid) < 0) { exit(0); /* not much we can do about this.. */ }

if(pid)		/* 2nd parent==process which deliberately dies */
	{
	exit(0);
	}

spawn_server(pid_fifo);	 /* actual server call here is at end of double-fork */
exit(0);
}


/*
 * the actual "private port" server
 */
void spawn_server(int pid)
{
char *listen_fifo_name, *write_fifo_name;
int listen_fd, write_fd;
static char *reqprefix="/request.";
static char *wrtprefix="/response.";
char pidname[PID_STRLEN_MAX+1];
static char *search_failed="page=no match\n";

char *fifo_read=NULL;

sprintf(pidname,".%d",pid);
listen_fifo_name=(char *)alloca(strlen(comm_dir)+strlen(reqprefix)+strlen(gtkwaveserver_id)+
		strlen(pidname)+1);
strcpy(listen_fifo_name, comm_dir);
strcat(listen_fifo_name, reqprefix);
strcat(listen_fifo_name, gtkwaveserver_id);
strcat(listen_fifo_name, pidname);

write_fifo_name=(char *)alloca(strlen(comm_dir)+strlen(wrtprefix)+strlen(gtkwaveserver_id)+
		strlen(pidname)+1);
strcpy(write_fifo_name, comm_dir);
strcat(write_fifo_name, wrtprefix);
strcat(write_fifo_name, gtkwaveserver_id);
strcat(write_fifo_name, pidname);

ID printf("Read fifo: %s\n", listen_fifo_name);
ID printf("Write fifo: %s\n", write_fifo_name);

if(((listen_fd=open(listen_fifo_name, O_RDONLY))<0)||(errno))
	{
	ID perror("spawn_server() listen_fd");
	die_quick();
	}

ID printf("Listening to private read fifo..\n");

if((fifo_read=saferead(listen_fd)))
	{
	ID printf("Request is:\n%s\n", fifo_read);
	close(listen_fd);

	if(((write_fd=open(write_fifo_name, O_WRONLY))<0)||(errno))
		{
		ID perror("spawn_server() write_fd");
		die_quick();
		}

	ID printf("Writing to private write fifo..\n");

	if(!good_query(fifo_read, write_fd))
		{
		write(write_fd, search_failed, strlen(search_failed));
		}
	close(write_fd);
	free(fifo_read);
	}
}


int good_query(char *q, int write_fd)
{
/* do query processing here */

while(rpc_ctx->valid) { selectsleep(100); };

strcpy(rpc_ctx->membuf, q);
rpc_ctx->valid = 1;

while(rpc_ctx->valid) { selectsleep(100); };

while(!rpc_ctx->resp_valid) { selectsleep(100); };
write(write_fd, rpc_ctx->resp_membuf, strlen(rpc_ctx->resp_membuf));
rpc_ctx->resp_valid = 0;

return(1);
}


/*
 * safe i/o read.  it isn't optimized for speed, but
 * should be impervious to buffer overflows.  
 */
char *saferead(int fd)
{
char *pnt=NULL;
struct safe *head=NULL, *current=NULL, *link=NULL;
int rlen, totallen=0;
char ch;

for(;;)
	{
        rlen=read(fd, &ch, 1);
	if(rlen>0) 
		{
		totallen+=rlen;
		link=(struct safe *)calloc2(1,sizeof(struct safe));
		link->val=ch;
		
		if(!current)
			{
			head=current=link;
			}
			else
			{
			current->next=link;
			current=link;
			}
		}
		else
		{
		break;
		}
	}

if(totallen)
	{
	int i=0;
	pnt=(char *)calloc2(1, totallen+1);
	link=head;
	while(link)
		{
		pnt[i++]=link->val;
		link=link->next;
		}
	}

while((link=head))
	{
	head=link->next;	
	free(link);
	}

return(pnt);
}


/*
 * calloc() that exits on error (for request/resp only!)
 */
void *calloc2(size_t nmemb, size_t size)
{
void *pnt;

if(!(pnt=calloc(nmemb, size)))
	{	
	exit(1);	
	}
	else
	{
	return(pnt);
	}
}
