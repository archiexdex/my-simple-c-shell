#include <bits/stdc++.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <glob.h>
using namespace std;

#define CMD_SEQ_BUFF_SIZE (1024)

#define MAX_ARG_COUNT (15)
#define MAX_CMD_COUNT (5)

void signal_config();

/* Purpose: Load a command sequence from standard input. */
char *read_cmd_seq();

/* Purpose: Convert a string to argv array array. */
char ***parse_cmd_seq(char *);

/* Purpose: Create several child process and redirect the standard output
 * to the standard input of the later process.
 */
void execute_cmd_seq(char ***argvs);

void creat_proc(char **argv, int fd_in, int fd_out, int pipes_count, int pipes_fd[][2]);

/* Purpose: Strip to whitespace at the front and end of the string. */
char *str_strip(char *str);

/* Purpose: Count the fequency of a character in the string. */
int str_char_count(char const *str, char c);

void write_file(string file_name);

int main(){
	while (1){
		char *cmd_seq = read_cmd_seq();
		if (cmd_seq == NULL) { break; }

		execute_cmd_seq(parse_cmd_seq(cmd_seq));
		fputc('\n', stdout);
	}

	return EXIT_SUCCESS;
}

/* Purpose: Create several child process and redirect the standard output
 * to the standard input of the later process.
 */
void execute_cmd_seq(char ***argvs){
	int C, P;

	int cmd_count = 0;
	while (argvs[cmd_count]) { ++cmd_count; }

	int pipeline_count = cmd_count - 1;

	int pipes_fd[MAX_CMD_COUNT][2];

	/* 準備足夠的 pipe */
	for (P = 0; P < pipeline_count; ++P){
		if (pipe(pipes_fd[P]) == -1){
			fprintf(stderr, "Error: Unable to create pipe. (%d)\n", P);
			exit(EXIT_FAILURE);
		}
	}

	for (C = 0; C < cmd_count; ++C){
		int fd_in = (C == 0) ? (STDIN_FILENO) : (pipes_fd[C - 1][0]);
		int fd_out = (C == cmd_count - 1) ? (STDOUT_FILENO) : (pipes_fd[C][1]);

		/* 呼叫下面的 creat_proc 來建立 Child Process */
		creat_proc(argvs[C], fd_in, fd_out, pipeline_count, pipes_fd);
	}

	/* 在建立所有 Child Process 之後，Parent Process 本身就不必使用 pipe
	了，所以關閉所有的 File descriptor。*/
	for (P = 0; P < pipeline_count; ++P){
		close(pipes_fd[P][0]);
		close(pipes_fd[P][1]);
	}

	/* 等待所有的程式執行完畢 */
	for (C = 0; C < cmd_count; ++C){
		int status = 0;
		wait(&status);
	}  
}

void creat_proc(char **argv, int fd_in, int fd_out, int pipes_count, int pipes_fd[][2]){
	pid_t proc = fork();

	if (proc < 0){
		fprintf(stderr, "Error: Unable to fork.\n");
		exit(EXIT_FAILURE);
	}
	else if (proc == 0){
		/* 把 fd_in 與 fd_out 分別當成 stdin 與 stdout。 */
		int inf, outf, id = 0, pin = 0, pout = 0, setid = 0, unsetid = 0;
		bool isIn = false, isOut = false;
		bool isSetenv = false, isUnset = false;
		string infilename = "", outfilename = "";
		
		
		
		for( int i = 0 ; argv[i] ; ++i ){
			string ptr = argv[i];
			if( ptr == "export" ){
				setid = i;
				if( argv[i+1] ){
					argv[i] = "set";
					isSetenv = true;
				}
				else {
					argv[i] = "env";
				}
			}
			if( ptr == "unset" )
				isUnset = true, unsetid = i;
		}
		
		for( int i = 0 ; argv[i] ; ++i ){
			string ptr = argv[i];
			if( ptr == "<" ){
				isIn = true;
				infilename = argv[i+1];
				int j;
				for( j = i; argv[j] ; ++j ){
					argv[j] = argv[j+2];
				}
				argv[j] = NULL;
			}
		}
		for( int i = 0 ; argv[i] ; ++i ){
			string ptr = argv[i];
			if( ptr == ">" ){
				isOut = true;
				outfilename = argv[i+1];
				int j;
				for( j = i; argv[j] ; ++j ){
					argv[j] = argv[j+2];
				}
				argv[j] = NULL;
			}
		}
		//printf("isIn %d isOut %d pipecount %d\n",isIn, isOut, pipes_count);
		
		
		if (isIn){
			inf = open(infilename.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
			dup2(inf,0);
			close(inf);
		}
		else if (fd_in != STDIN_FILENO) { dup2(fd_in, STDIN_FILENO); }
		if ( isOut ){
			outf = open(outfilename.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
			dup2(outf,1);
			close(outf);
		}
		else if (fd_out != STDOUT_FILENO) { dup2(fd_out, STDOUT_FILENO); }
		
		
		/* 除了 stdin, stdout 之外，所有的 File descriptor (pipe) 都要關閉。*/
		for ( int P = 0; P < pipes_count; ++P){
			close(pipes_fd[P][0]);
			close(pipes_fd[P][1]);
		}
		
		
		
		
		if( isSetenv ){
			for( int i = setid+1 ; argv[i] ; ++i ){
				char *ptr = strtok(argv[i],"=");
				char *a = ptr;
				ptr = strtok(NULL, "=");
				char *b = ptr;
				//printf("a : %s b : %s \n",a,b);
				if( !b )
					setenv(a,"",1);
				else
					setenv(a,b,1);
			}
			return;
		}
		if( isUnset ){
			for( int i = unsetid+1 ; argv[i] ; ++i ){
				char *ptr = argv[i];
				puts(ptr);
				unsetenv(ptr);
			}
			return ;
		}
		
		vector<string> v;
		for( int i = 0 ; argv[i] ; ++i ){
			string tmp = argv[i];
			if( tmp.find("*") != string::npos || tmp.find("?") != string ::npos ){
				glob_t gl;
				glob(tmp.c_str(), GLOB_TILDE, NULL, &gl);
				for(int i = 0 ; i < gl.gl_pathc ; ++i){
					v.push_back(gl.gl_pathv[i]);
				}
				globfree(&gl);
			}
			else {
				v.push_back(tmp);
			}
		}

		if( !v.empty() )
			for( int i = 0 ; i < v.size() ; ++i )
				argv[i] = (char*)v[i].c_str();
		
		if ( execvp(argv[0], argv) == -1){
			
			fprintf(stderr,"Error: Unable to load the executable %s.\n", argv[0]);
			exit(EXIT_FAILURE);
		}

		/* NEVER REACH */
		exit(EXIT_FAILURE);
	}
}

/* Purpose: Load a command sequence from standard input. */
char *read_cmd_seq(){
	static char cmd_seq_buffer[CMD_SEQ_BUFF_SIZE];

	fputs("(」・ω・)」うー！(／・ω・)／にゃー！ ", stdout);
	fflush(stdout);

	memset(cmd_seq_buffer, '\0', sizeof(cmd_seq_buffer));
	fgets(cmd_seq_buffer, sizeof(cmd_seq_buffer), stdin);

	if (feof(stdin)) { return NULL; }

	char *cmd_seq = str_strip(cmd_seq_buffer);
	if (strlen(cmd_seq) == 0) { return NULL; }

	return cmd_seq; 
}

/* Purpose: Convert a string to argv array array. */
char ***parse_cmd_seq(char *str){
	int i, j;

	static char *cmds[MAX_CMD_COUNT + 1];
	memset(cmds, '\0', sizeof(cmds));

	cmds[0] = str_strip(strtok(str, "|"));
	for (i = 1; i <= MAX_CMD_COUNT; ++i)
	{
		cmds[i] = str_strip(strtok(NULL, "|"));
		if (cmds[i] == NULL) { break; }
	}

	static char *argvs_array[MAX_CMD_COUNT + 1][MAX_ARG_COUNT + 1];
	static char **argvs[MAX_CMD_COUNT + 1];

	memset(argvs_array, '\0', sizeof(argvs_array));
	memset(argvs, '\0', sizeof(argvs));

	for (i = 0; cmds[i]; ++i)
	{
		argvs[i] = argvs_array[i];

		argvs[i][0] = strtok(cmds[i], " \t\n\r");
		for (j = 1; j <= MAX_ARG_COUNT; ++j)
		{
			argvs[i][j] = strtok(NULL, " \t\n\r");
			if (argvs[i][j] == NULL) { break; }
		}
	}

	return argvs;
}



/* Purpose: Strip to whitespace at the front and end of the string. */
char *str_strip(char *str){
	if (!str) { return str; }

	while (isspace(*str)) { ++str; }

	char *last = str;
	while (*last != '\0') { ++last; }
	last--;

	while (isspace(*last)) { *last-- = '\0'; }

	return str;
}

/* Purpose: Count the fequency of a character in the string. */
int str_char_count(char const *str, char c){
	int count = 0;

	if (str){
		while (*str != '\0'){
			if (*str++ == c) { count++; }
		}
	}

	return count;
}
