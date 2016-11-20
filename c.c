#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>

#define BUFSIZE 1024
void error_handling(char *message);

int main(int argc, char **argv) {
	int sock;
	char message[BUFSIZE], f_name[BUFSIZE], serv_buf[BUFSIZE];
	int str_len, i, file_size;
	socklen_t addr_size;
	FILE *fp;
	char content[BUFSIZE], len[BUFSIZE];
	int serv_buf_size;
	struct sockaddr_in serv_addr;
	struct sockaddr_in from_addr;

	if (argc != 3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (sock == -1)
		error_handling("UDP 소켓 생성 오류");
	memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));


	fputs("Input file name: ", stdout);
	scanf("%s", f_name);
	fp = fopen(f_name, "rb");
	if (NULL == fp) {
		printf("Error opening file\n");
		return 0;
	}

	sendto(sock, f_name, strlen(f_name), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	memset(message, 0, sizeof(message));

	addr_size = sizeof(from_addr);
	memset(serv_buf, 0, strlen(serv_buf));
	str_len = recvfrom(sock, serv_buf, BUFSIZE, 0, (struct sockaddr*)&from_addr, &addr_size);

	printf("str_len: %d\n", str_len);
	printf("server의 버퍼 사이즈: %s\n", serv_buf);
	serv_buf_size = atoi(serv_buf);

	while (1) {
		file_size = fread(content, 1, serv_buf_size, fp);
		if (file_size == 0)
			break;
		sendto(sock, content, serv_buf_size, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
		memset(content, 0, sizeof(content));
	}
	strcpy(content, "end");
	sendto(sock, content, serv_buf_size, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	fclose(fp);
	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}