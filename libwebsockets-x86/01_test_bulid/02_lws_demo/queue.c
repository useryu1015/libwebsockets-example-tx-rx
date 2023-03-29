// queue.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MSGSZ 1024

typedef struct msgbuf {
    long mtype;         // 消息类型
    char mtext[MSGSZ];  // 消息内容
} message_buf;

int main() {
    int msqid;
    key_t key;
    message_buf sbuf;
    size_t buflen;

    // 创建消息队列
    if ((key = ftok(".", 'a')) == -1) {
        perror("ftok error");
        exit(1);
    }
    if ((msqid = msgget(key, 0666 | IPC_CREAT)) == -1) {
        perror("msgget error");
        exit(1);
    }

    // 发送消息到队列
    sbuf.mtype = 1;
    strcpy(sbuf.mtext, "Hello, message queue!");
    buflen = strlen(sbuf.mtext) + 1;
    if (msgsnd(msqid, &sbuf, buflen, IPC_NOWAIT) == -1) {
        perror("msgsnd error");
        exit(1);
    }

    // 从队列中读取消息
    if (msgrcv(msqid, &sbuf, MSGSZ, 1, 0) == -1) {
        perror("msgrcv error");
        exit(1);
    }
    printf("Received message: %!s(MISSING)\n", sbuf.mtext);

    // 删除消息队列
    if (msgctl(msqid, IPC_RMID, NULL) == -1) {
        perror("msgctl error");
        exit(1);
    }

    return 0;
}