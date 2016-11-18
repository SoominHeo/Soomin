#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFSIZE 512
void error_handling(char *message);

int main(int argc, char **argv){
        int serv_sock;
        char message[BUFSIZE], bufsize[BUFSIZE];
        int str_len;
        int num=0;
        FILE *fp;

        struct sockaddr_in serv_addr;
        struct sockaddr_in clnt_addr;
        int clnt_addr_size;

        if(argc!=2){
                printf("Usage : %s <port>\n", argv[0]);
                exit(1);
        }

        serv_sock=socket(PF_INET, SOCK_DGRAM, 0);
        if(serv_sock == -1){
                error_handling("UDP 소켓 생성 오류");
        }

        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family=AF_INET;
        serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
        serv_addr.sin_port=htons(atoi(argv[1]));

        if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1){
                error_handling("bind() error");
        }
// client가 보낸 파일 이름 출력
        //받기1
        str_len = recvfrom(serv_sock, message, BUFSIZE, 0, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
        message[str_len] = 0;
        printf("client가 보낸 파일: %s \n", message);
        //보내기2
        sendto(serv_sock, message, strlen(message), 0, (struct sockaddr*)&clnt_addr, sizeof(clnt_addr));

// client한테 buffer size 보내고 싶어!
        memset(bufsize, 0, sizeof(bufsize));
        strcpy(bufsize, "5");
        printf("server의 buffer size: %s\n", bufsize);
        //보내기3
        sendto(serv_sock, bufsize, strlen(bufsize), 0, (struct sockaddr*)&clnt_addr, sizeof(clnt_addr));
        printf("버퍼사이즈 전송 완료\n");

        fp = fopen(message, "wb");
        if(NULL == fp){
                printf("Error opening file\n");
                return 0;
        }
        sendto(serv_sock, message, str_len, 0, (struct sockaddr*)&clnt_addr, sizeof(clnt_addr));

// 파일 내용 받기
        while(1) {
                memset(message, 0, sizeof(message));
                str_len = recvfrom(serv_sock, message, BUFSIZE, 0, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
                if(strcmp(message, "end")==0) break;
                message[str_len] = 0;

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
