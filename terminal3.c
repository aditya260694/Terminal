#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/utsname.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>
#include<signal.h>
#include<sys/wait.h>
#include<malloc.h>
#include<termios.h>
#include<sys/types.h>
char *a;
pid_t super;
pid_t child;
int ex;
typedef struct job
{
	char j[1000];
	pid_t p;
	struct job *next;
}job;



int max;
job *head,*tail;
char *token1;

void CSH(int signum) {
	int status;
	pid_t pid;
	pid = waitpid(-1, &status, WNOHANG);
	if(pid>0 && signum==SIGCHLD)
	{
		job *temp=head;

		while(temp->next!=NULL)
		{
			if(temp->next->p==pid)
				break;
			temp=temp->next;
		}

		if(temp->next!=NULL)
		{
			printf("%s with pid %d : exited successfully\n",temp->next->j,pid);

			if(temp->next==tail)
				tail=temp;
			temp->next=temp->next->next;
		}
	}
}

void sigproc(int signum)
{
	if(signum == SIGINT)
		printf("termination attempted using SIGINT\n");
	if(signum == SIGKILL)
		printf("termination attempted using SIGKILL\n");
	if(signum==SIGQUIT)
		printf("termination attempted using SIGQUIT\n");
	if(signum==SIGTSTP)
	{

		kill(child,SIGSTOP);
		tcsetpgrp(STDIN_FILENO,super);
		printf("\n%d process in background\n",child);
		tail->next=malloc(sizeof(job));
		tail->next->p=child;
		strcpy(tail->next->j,token1);
		tail->next->next=NULL;
		tail=tail->next;

	}

	if(signum==SIGTSTP && ex==0)
	{
		printf("no process currently executing\n");
	}
}

int main()
{
	super=getpid();
	head=malloc(sizeof(job));
	head->next=NULL;
	tail=head;

	struct utsname data;
	uname(&data);
	char b[250];
	char cwd[200];
	getcwd(cwd,200);
	int m=strlen(cwd);
	signal(SIGCHLD,CSH);
	if (signal(SIGUSR1, sigproc) == SIG_ERR)
		printf("\ncan't catch SIGUSR1\n");
	if (signal(SIGINT, sigproc) == SIG_ERR)
		printf("\ncan't catch SIGSTOP\n");
	if (signal(SIGQUIT, sigproc) == SIG_ERR)
		printf("\ncan't catch SIGQUIT\n");
	signal(SIGTSTP,SIG_IGN);
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);


	while(1)
	{
		a=getlogin();
		char c[200];
		getcwd(b,200);
		int i=0;
		if(strstr(b,cwd) != NULL)
		{
			for(i=0;i<=m;i++)
				b[i]=b[i+m];
			printf("<%s@%s:~%s>",a,data.nodename,b);
		}

		else
			printf("<%s@%s:%s>",a,data.nodename,b);

		char f='\0';
		i=0;
		gets(c);
		char qw[200];
		int cmd_len=strlen(c);

		if(strlen(c)==0)
			continue;
		int it,qt=0;

		for(it=0;it<cmd_len;it++)
		{
			while(c[it]==' ' || c[it]=='\t')
				it++;
			if(c[it]=='\0')
			{
				qw[qt++]='\0';
				break;
			}

			while(c[it]!=' ' && c[it]!='\t' && c[it]!='\0')
			{
				qw[qt++]=c[it];
				it++;
			}

			if(c[it]!='\0')
				qw[qt++]=' ';
			else
			{
				qw[qt++]='\0';
				break;
			}
		}

		strcpy(c,qw);
		
		int num_pipe=0;
		char *execute[100];
		char *token2=strtok(qw,"|");
		while(token2!=NULL)
		{
			if(token2[0]==' ')
				execute[num_pipe++]=token2+1;
			else
				execute[num_pipe++]=token2;

			token2=strtok(NULL,"|");
		}
		
		
		if(strlen(c)==0)
			continue;

		//if quit is command
		if(strcmp(c,"quit")==0)
			break;

		char *token;
		token=strtok(c," ");
		token1=token;


		//for cd
		int j=0;
		int k;
		if(strcmp(token,"cd")==0)
		{
			token=strtok(NULL," ");
			if(token==NULL)
				chdir(cwd);
			else if(chdir(token) != 0)
			{
				printf("error :%s\n",strerror(errno));
			}
			continue;
		}


		//user defined commands
		//jobs
		if(strcmp(token,"jobs")==0)
		{
			job *temp=head;
			if(temp->next==NULL)
				printf("no jobs\n");
			else
			{
				int j1=1;
				while(temp->next!=NULL)
				{
					printf("[%d]%s[%d]\n",j1,temp->next->j,temp->next->p);
					j1++;
					temp=temp->next;
				}
			}
			continue;
		}



		//pinfo
		if(strcmp(token,"pinfo")==0)
		{
			token=strtok(NULL," ");
			FILE *fp;
			ssize_t len2;
			char buff2[1000];
			if(token==NULL)
			{
				fp=fopen("/proc/self/stat", "r");
				len2=readlink("/proc/self/exe",buff2,1000);
				if(strstr(buff2,cwd) != NULL)
				{
					for(i=0;i<=m;i++)
						buff2[i]=buff2[i+m];
				}	
			}
			else
			{
				char proc[1000];
				proc[0]='\0';
				strcat(proc,"/proc/");
				strcat(proc,token);
				strcat(proc,"/stat");
				fp=fopen(proc, "r");

				proc[0]='\0';
				strcat(proc,"/proc/");
				strcat(proc,token);
				strcat(proc,"/exe");
				len2=readlink(proc,buff2,1000);
			}

			if(fp!=NULL)
			{
				char buff[1002];	
				fseek(fp,0,SEEK_SET);
				fgets (buff, 1000, fp);
				buff[strlen(buff)-1] = '\0';
				char *t=strtok(buff," ");
				printf("Pid :%s\n",t);
				t=strtok(NULL," ");
				int len=strlen(t);
				int h;
				t=strtok(NULL," ");
				printf("status :");
				printf("%s\n",t);
				for(h=4;h<=23;h++)	
					t=strtok(NULL," ");
				printf("memory :");
				printf("%s bytes\n",t);
				buff2[len2]='\0';
				printf("Executable path :%s\n",buff2);
			}

			else
				printf("error :%s\n",strerror(errno));
			continue;
		}

		//kjob
		if(strcmp(token,"kjob")==0)
		{
			token=strtok(NULL," ");

			if(token==NULL)
			{
				printf("invalid argument\n");
				continue;
			}
			int y=atoi(token);
			if(y<1)
			{
				printf("Invalid process number\n");
				continue;
			}
			token=strtok(NULL," ");
			if(token==NULL)
			{
				printf("invalid argument\n");
				continue;
			}

			int z=atoi(token);
			job *temp=head;
			int o=0;
			int chk=0;
			job *pr=temp;
			for(o=1;o<=y;o++)
			{
				if(temp->next==NULL)
				{
					chk=1;
					printf("No process available\n");

					break;
				}

				if(chk==1)
					break;
				pr=temp;
				temp=temp->next;
			}

			if(chk==1)
				continue;


			kill(temp->p,SIGCONT);
			kill(temp->p,z);
			if(z==9 || z==2 || z==3 || z==15)
			{
				if(tail==pr->next)
					tail=pr;
				pr->next=pr->next->next;
			}
			continue;
		}

		//fg
		if(strcmp(token,"fg")==0)
		{
			token=strtok(NULL," ");

			if(token==NULL)
			{
				printf("Invalid Argument");
				continue;
			}

			int o=atoi(token);

			int z=0;
			int status;
			job *temp=head;
			int chk=0;
			job *pr;
			if(o<1)
			{
				printf("Invalid process Number\n");
				continue;
			}

			if(temp->next==NULL)
			{
				printf("No process Available\n");
				continue;
			}
			for(z=1;z<=o;z++)
			{
				if(temp->next==NULL)
				{
					chk=1;
					printf("No process available\n");
					break;
				}
				pr=temp;
				temp=temp->next;	
			}
			if(chk==1)
				continue;

			if(chk==0)
			{
				kill(temp->p,SIGCONT);
				printf("%d %d %s\n",o,temp->p,temp->j);
				tcsetpgrp(STDIN_FILENO,temp->p);
				waitpid(temp->p,&status,WUNTRACED);
				if(WIFSTOPPED(status))
				{
					kill(temp->p,SIGCONT);
					tcsetpgrp(STDIN_FILENO,temp->p);
					waitpid(temp->p,&status,WUNTRACED);
				}
				tcsetpgrp(STDIN_FILENO,getpid());



				pid_t rem=temp->p;
				if(WIFEXITED(status)!=0)
				{
					temp=head;
					while(temp->next->p!=rem)
						temp=temp->next;
					if(tail==temp->next)
						tail=temp;
					temp->next=temp->next->next;
				}
			}
			continue;
		}

		//overkill
		if(strcmp(token,"overkill")==0)
		{
			job *temp=head;

			while(temp->next!=NULL)
			{
				kill(temp->next->p,9);
				job *pr=temp->next;
				temp->next=temp->next->next;
				free(pr);
			}
			tail=head;
			continue;
		}

		//for other commands





		int exec_pipe=0;
		if(num_pipe==1)
		{
			char *com=token;
			char *args[20];
			int y=0;
			args[y]=com;
			int in=0,out=0,infile=0,outfile=0;
			while(token != NULL)
			{	
				token=strtok(NULL," ");

				if(token != NULL)
				{
					if(strcmp(token,"<")==0)
					{
						in=1;
						token=strtok(NULL," ");
						infile=open(token,O_RDONLY);
					}

					else if(strcmp(token,">")==0)
					{
						out=1;
						token=strtok(NULL," ");
						outfile=open(token,O_WRONLY|O_CREAT,0777);
					}

					else
						args[++y]=token;
				}
			}

			args[++y]=token;

			int chk=1,status;
			child=fork();
			tail->next=malloc(sizeof(job));
			strcpy(tail->next->j,c);
			tail->next->p=child;
			tail->next->next=NULL;
			tail=tail->next;
			if(strcmp(args[y-1],"&")==0)
			{
				chk=0;
				args[y-1]=NULL;	
			}

			if(child==0)
			{

				if(in==1)
					dup2(infile,0);
				if(out==1)
					dup2(outfile,1);

				setpgid(getpid(),getpid());
				signal(SIGTSTP,SIG_DFL);
				signal(SIGTTOU,SIG_DFL);
				signal(SIGTTIN,SIG_DFL);
				signal(SIGCHLD,SIG_DFL);
				if(chk==1)
					tcsetpgrp(STDIN_FILENO,getpid());

				if(execvp(com,args)!=-1)
					;
				else
					printf("error :%s\n",strerror(errno));
				exit(0);	
			}

			ex=chk;

			//background
			if(chk==1)
			{
				tcsetpgrp(STDIN_FILENO,child);
				waitpid(child,&status,WUNTRACED);
				if(WIFEXITED(status)!=0)
				{
					job *temp=head;

					while(temp->next->p!=child)
						temp=temp->next;

					if(tail=temp->next)
						tail=temp;
					temp->next=temp->next->next;
				}
				tcsetpgrp(STDIN_FILENO,super);
				ex=0;
			}

			else
			{
				tcsetpgrp(STDIN_FILENO,super);
			}
			in=0;out=0;
		}



		else
		{

			child=fork();
			int chk=1,status;
			tail->next=malloc(sizeof(job));
			strcpy(tail->next->j,c);
			tail->next->p=child;
			tail->next->next=NULL;
			tail=tail->next;
			int bkg=0;
			if(execute[num_pipe-1][strlen(execute[num_pipe-1])-1]=='&')
			{
				bkg=1;
				execute[num_pipe-1][strlen(execute[num_pipe-1])-1]='\0';
			}

			if(child==0)
			{
				setpgid(getpid(),getpid());
				signal(SIGTSTP,SIG_DFL);
				signal(SIGTTOU,SIG_DFL);
				signal(SIGTTIN,SIG_DFL);
				signal(SIGCHLD,SIG_DFL);
				tcsetpgrp(STDIN_FILENO,getpid());

				while(exec_pipe < num_pipe)
				{
					char *token4;
					token4=strtok(execute[exec_pipe]," ");
					char *com=token4;
					char *args[20];
					int y=0;
					int fd[2];
					pipe(fd);
					args[y]=com;
					int in=0,out=0,infile=0,outfile=0;
					while(token4 != NULL)
					{	

						token4=strtok(NULL," ");

						if(token4 != NULL)
						{
							if(strcmp(token4,"<")==0)
							{
								in=1;
								token4=strtok(NULL," ");
								infile=open(token4,O_RDONLY);
							}

							else if(strcmp(token4,">")==0)
							{
								out=1;
								token4=strtok(NULL," ");
								outfile=open(token4,O_WRONLY|O_CREAT,0777);
							}

							else
								args[++y]=token4;
						}
					}

					args[++y]=token4;
					pid_t child1;

					child1=fork();


					if(child1==0)
					{
				signal(SIGTSTP,SIG_DFL);
				signal(SIGTTOU,SIG_DFL);
				signal(SIGTTIN,SIG_DFL);
				signal(SIGCHLD,SIG_DFL);
						exec_pipe++;
						dup2(fd[0],0);
						close(fd[1]);
						close(fd[0]);
						if(exec_pipe==num_pipe)
						{

							return 0;
						}
						//waitpid(getppid(),&status,0);
					}

					else
					{
						pid_t naruto;

						naruto=fork();

						if(naruto==0)
						{
						if(exec_pipe!=num_pipe-1)
							dup2(fd[1],1);
						close(fd[0]);
						close(fd[1]);
						if(in==1)
							dup2(infile,0);
						if(out==1)
							dup2(outfile,1);
						if(execvp(com,args)!=-1)
							;
						else
							printf("error :%s\n",strerror(errno));
						return 0;
						}

						waitpid(naruto,&status,0);
						exec_pipe=num_pipe;

						return 0;
					}
				in=0;out=0;
				}

			return 0;
			}

			int closing,status1;

			ex=chk;
				
				tcsetpgrp(STDIN_FILENO,child);
				waitpid(child,&status,0);
				if(WIFEXITED(status)!=0)
				{
					job *temp=head;
						while(temp->next->p!=child)
						temp=temp->next;
						if(tail=temp->next)
						tail=temp;
					temp->next=temp->next->next;
				}
				if(bkg!=1)
				sleep(1);
				tcsetpgrp(STDIN_FILENO,super);
				ex=0;
		}	
			
	}



job *temp=head,*pr;
while(temp->next!=NULL)
{
	kill(temp->next->p,9);
	pr=temp->next;
	temp->next=temp->next->next;
	free(pr);
}

return 0;
}
