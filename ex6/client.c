#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <termios.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUF 1024
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 5010

static int sock;
static struct sockaddr_in serverAddr;

static void die(const char *m) { perror(m); exit(EXIT_FAILURE); }

static void sendMsg(const char *msg) {
    if (sendto(sock, msg, strlen(msg), 0,
               (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
        die("sendto");
}

static ssize_t recvMsg(char *buf) {
    struct sockaddr_in from;
    socklen_t len = sizeof(from);
    ssize_t n = recvfrom(sock, buf, BUF - 1, 0, (struct sockaddr *)&from, &len);
    if (n >= 0) buf[n] = '\0';
    return n;
}

static void readLine(char *buf, int size) {
    if (!fgets(buf, size, stdin)) { buf[0] = '\0'; return; }
    buf[strcspn(buf, "\n")] = 0;
}

/* read a password without echoing it — print '*' per keystroke */
static void readMasked(char *buf, int size) {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO);           /* hide real characters */
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    int i = 0, c;
    while ((c = getchar()) != '\n' && c != EOF && i < size - 1) {
        if (c == 127 || c == '\b') {   /* backspace */
            if (i > 0) { i--; fputs("\b \b", stdout); }
        } else {
            buf[i++] = (char)c;
            fputc('*', stdout); fflush(stdout);
        }
    }
    buf[i] = '\0';
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    putchar('\n');
}

/* Half-duplex chat, client speaks first (mirrors the server) */
static void chatLoop(void) {
    char buf[BUF], line[BUF], pkt[BUF + 8];
    int end = 0;
    while (!end) {
        printf("\nsay 'over' to toggle the turn . 'bye' to end chat.\n");
        while (1) {                                   /* SPEAK */
            printf("Client: ");
            readLine(line, BUF);
            snprintf(pkt, sizeof(pkt), "CHAT|%s", line);
            sendMsg(pkt);
            if (strcasecmp(line, "bye") == 0)  { end = 1; break; }
            if (strcasecmp(line, "over") == 0) break;
        }
        if (end) break;
        printf("\nLISTENING to server...\n");
        while (1) {                                   /* LISTEN */
            recvMsg(buf);
            if (strncmp(buf, "CHAT|", 5) != 0) continue;
            printf("Server: %s\n", buf + 5);
            if (strcasecmp(buf + 5, "bye") == 0)  { end = 1; break; }
            if (strcasecmp(buf + 5, "over") == 0) break;
        }
    }
}

int main(void) {
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) die("socket");

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr) <= 0) die("inet_pton");

    printf("Connecting to server %s:%d ...\n", SERVER_IP, SERVER_PORT);
    sendMsg("HELLO|");                 /* so the server learns our address */

    char buf[BUF];
    for (;;) {                         /* pure driver loop — no menu here */
        if (recvMsg(buf) < 0) die("recvfrom");

        if (strncmp(buf, "SHOW|", 5) == 0) {
            printf("%s", buf + 5); fflush(stdout);
        }
        else if (strncmp(buf, "ASK|", 4) == 0) {
            char line[BUF], out[BUF + 8];
            printf("%s", buf + 4); fflush(stdout);
            readLine(line, BUF);
            snprintf(out, sizeof(out), "INP|%s", line);
            sendMsg(out);
        }
        else if (strncmp(buf, "GETPASS|", 8) == 0) {
            char pass[BUF], out[BUF + 8];
            printf("%s", buf + 8); fflush(stdout);
            readMasked(pass, sizeof(pass));
            snprintf(out, sizeof(out), "INP|%s", pass);
            sendMsg(out);
        }
        else if (strncmp(buf, "CHATGO|", 7) == 0) {
            chatLoop();
        }
        else if (strncmp(buf, "EXIT|", 5) == 0) {
            printf("Connection closed by server.\n");
            break;
        }
    }
    close(sock);
    return 0;
}
