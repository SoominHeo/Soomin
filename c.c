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

int main(int argc, char **argv){
        int sock;
        char message[BUFSIZE], f_name[BUFSIZE];
        int str_len, addr_size, i, file_size;
        FILE *fp;
        char content[BUFSIZE], len[BUFSIZE];
        int serv_buf_size;
        struct sockaddr_in serv_addr;
        struct sockaddr_in from_addr;

        if(argc!=3){
                printf("Usage : %s <IP> <port>\n", argv[0]);
                exit(1);
        }
        sock=socket(PF_INET, SOCK_DGRAM, 0);
        if(sock == -1)
                error_handling("UDP 소켓 생성 오류");
        memset(&serv_addr, 0, sizeof(serv_addr));

        serv_addr.sin_family=AF_INET;
        serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
        serv_addr.sin_port=htons(atoi(argv[2]));

// file 이름 입력하고, server에게 file 이름을 보냄
        fputs("Input file name: ",stdout);
        scanf("%s", f_name);

        fp = fopen(f_name, "rb");
        if(NULL == fp){
                printf("Error opening file\n");
                return 0;
        }
        // 보내기1
        sendto(sock, f_name, strlen(f_name), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        // 받기 2
        memset(message, 0, sizeof(message));
        str_len = recvfrom(sock, message, BUFSIZE, 0, (struct sockaddr*)&from_addr, &addr_size);
        printf("sender가 인식한 파일 이름: %s \n", message);

// server의 버퍼 사이즈 받기
        // 받기3
        memset(message, 0, sizeof(message));
        str_len = recvfrom(sock, message, BUFSIZE, 0, (struct sockaddr*)&from_addr, &addr_size);
        printf("server의 버퍼 사이즈: %s\n", message);
        serv_buf_size = atoi(message);
        printf("숫자로 나타낸 server의 버퍼 사이즈: %d\n", serv_buf_size);

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