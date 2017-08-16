#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>

#define NUM_TOKENS 64
#define BUFFER_LENGTH 64
#define DEFAULT_PROMPT "CS347(M)$ "
#define ROLL_NO "140010011"
#define NAME "Rahul Govind"
typedef int bool;
bool child_running = 0;

int num_args(char** args)
{
	int result = 0;
	while(*args != NULL)
	{
		result++;
		args++;
	}

	return result;
}

void change_directory(const char** args)
{
	int n_args = num_args(args);

	if(n_args == 1)
	{
		chdir(getenv("HOME"));
	}
	else if(n_args >= 2)
	{
		if(chdir(args[1]) == -1)
			fprintf(stderr, "No such directory");
	}
}

void handle_command(const char** args)
{
	if(strcmp(args[0], "exit") == 0)
		exit(0);
	else if(strcmp(args[0], "cd") == 0)
		change_directory(args);
	else if(strcmp(args[0], "roll") == 0)
		printf("%s\n", ROLL_NO);
	else if(strcmp(args[0], "name") == 0)
		printf("%s\n", NAME);
	else
		execute_command(args);
}

int execute_command(const char** args)
{
	pid_t pid;
	int status;
	pid = fork();

	if(pid == 0)
	{
		// Child
		execvp(args[0], args);
		fprintf(stderr, "Error!\n");
		exit(0);
	}
	else
	{
		child_running = 1;
		
		// Parent
		do {
			waitpid(pid, &status, WUNTRACED);
		} while(!(WIFEXITED(status) || WIFSIGNALED(status)));

		child_running = 0;
	}
	return 1;
}

char* read_input()
{
	char* line = NULL;
	size_t len = 0;

	if(getline(&line, &len, stdin) == -1)
	{
		fprintf(stderr, "Unable to read input\n");
		exit(-1);
	}
	
	return line;
}

char** tokenize(char* s, int* n_tokens)
{
	char* sep = " \t\r\n";
	char* original_s = s;
	int max_token_count = NUM_TOKENS;
	char** tokens = (char**)calloc(max_token_count, sizeof(char*));
	int num_tokens = 0;
	char* token;
	int len;

	// Tokenize string
	for(token=strsep(&s, sep);
	    token != NULL;
	    token=strsep(&s, sep))
	{
		// Only read token if it's non-empty
		if((len = strlen(token)) != 0)
		{
			num_tokens++;

			// If more tokens than can be stored, then allocate more memory
			if (num_tokens >= max_token_count)
			{
				max_token_count += NUM_TOKENS;
				tokens = (char**)realloc(tokens, max_token_count * sizeof(char*));
				memset(tokens + (max_token_count - NUM_TOKENS), 0, NUM_TOKENS * sizeof(char*));
			}
			
			// Copy token to our list of tokens
			tokens[num_tokens-1] = strdup(token);
		}
	}

	*n_tokens = num_tokens;
	return tokens;
}

void free_tokens(char** tokens)
{
	char** it = tokens;
	while(*it != NULL)
		free(*(it++));
	free(tokens);
}

int sigint()
{
	if(!child_running)
	{
		printf("\n");
		exit(0);
	}
}

int main()
{
	signal(SIGINT, sigint);
	while(1) {
		printf(DEFAULT_PROMPT);
		int n_tokens;
		char *s = read_input();
		char ** tokens = tokenize(s, &n_tokens);
		
		// Exit if exit command given
		if(n_tokens == 0)
			continue;

		handle_command(tokens);
		free(s);
		free_tokens(tokens);
	}

}
