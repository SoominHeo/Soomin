#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>

#define BUFSIZE 512
void error_handling(char *message);

typedef struct clnt_buffer {
	char fileName[BUFSIZE];
	struct sockaddr_in address;
	struct clnt_buffer *next;
}clnt_buffer;

typedef struct dummy {
	clnt_buffer *next;
	clnt_buffer *end;
}dummy;


//create the wait client
clnt_buffer* create(char* fileName, struct sockaddr_in address);

//make the client wait
void stop(dummy *buffer, clnt_buffer *new_clnt);

//return the next client
clnt_buffer* start(dummy *buffer);

int main(int argc, char **argv) {
	int serv_sock;
	char message[BUFSIZE];
	char bufsize[BUFSIZE];

	int str_len;
	int num = 0;
	FILE *fp;
	int a;

	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	int clnt_addr_size;

	struct sockaddr_in *curr;
	struct sockaddr_in *next;
	dummy* wait_list = NULL;
	clnt_buffer* clnt_info;

	if (argc != 2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	serv_sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (serv_sock == -1) {
		error_handling("UDP 소켓 생성 오류");
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(atoi(argv[1]));

	curr = NULL;
	next = NULL;
	wait_list = (struct dummy*)malloc(sizeof(struct dummy*));
	wait_list->next == NULL;
	wait_list->end == NULL;

	if (bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1) {
		error_handling("bind() error");
	}


	//이제부터 파일 받기 시작!
	while (1) {
		clnt_addr_size = sizeof(clnt_addr);
		str_len = recvfrom(serv_sock, message, BUFSIZE, 0, (struct sockaddr*)&clnt_addr, &clnt_addr_size);

		message[str_len] = 0;
		curr = next;
		next = &clnt_addr;

		// 처음
		if (curr == NULL) {
			sprintf(bufsize, "%d", BUFSIZE);
			bufsize[strlen(bufsize)] = 0;
			a = sendto(serv_sock, bufsize, strlen(bufsize), 0, (struct sockaddr*)&clnt_addr, sizeof(clnt_addr));

			if (a == -1) {
				perror("sendto");
				exit(1);
			}

			//파일 열기
			printf("%s open\n", message);
			fp = fopen(message, "wb");

			if (NULL == fp) {
				printf("Error opening file\n");
				return 0;
			}
			continue;
		}

		// 파일 다 받았으면
		if (strcmp(message, "end") == 0) {
			fclose(fp);
			if (wait_list->next != NULL) {
				clnt_info = start(wait_list);
				clnt_info = start(wait_list);
				fopen(clnt_info->fileName, "wb");
				*next = clnt_info->address;
			}
			else {
				curr = NULL;
				next = NULL;
			}
			continue;
		}

		//다른 client가 보냄
		if (curr != next) {
			clnt_info = create(message, clnt_addr);
			printf("add %s to buffer\n", message);
			*next = *curr;
			continue;
		}

		fwrite(message, 1, strlen(message), fp);

	}

	fclose(fp);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
clnt_buffer* create(char* fileName, struct sockaddr_in address) {
	clnt_buffer* new_node = (clnt_buffer*)malloc(sizeof(clnt_buffer*));
	strcpy(new_node->fileName, fileName);
	new_node->address = address;
	new_node->next = NULL;
	return new_node;
}

void stop(dummy *buffer, clnt_buffer *new_clnt) {
	clnt_buffer *tmp;
	if (buffer->next == NULL) {
		buffer->next = new_clnt;
		buffer->end = new_clnt;
	}
	else {
		buffer->end->next = new_clnt;
	}
}

clnt_buffer* start(dummy *buffer) {
	clnt_buffer *tmp = NULL;
	if (buffer->next == NULL) {
		return;
	}

	else {
		tmp = buffer->next;
		buffer->next = buffer->next->next;
		tmp->next = NULL;
		return tmp;
	}
}