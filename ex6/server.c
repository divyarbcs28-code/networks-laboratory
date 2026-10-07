#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUF 1024
#define PORT 5010
#define USERS_FILE "users.txt"

static int sock;

static void die(const char *m) {
    perror(m);
    exit(EXIT_FAILURE);
}

static void sendTo(const struct sockaddr_in *to, socklen_t len,
                   const char *msg) {
    if (sendto(sock, msg, strlen(msg), 0,
               (const struct sockaddr *)to, len) < 0) {
        die("sendto");
    }
}

static ssize_t recvFrom(char *buf, struct sockaddr_in *from, socklen_t *len) {
    ssize_t n = recvfrom(sock, buf, BUF - 1, 0,
                         (struct sockaddr *)from, len);
    if (n >= 0) {
        buf[n] = '\0';
    }
    return n;
}

static int findUser(const char *name, char *passOut, size_t size) {
    FILE *f = fopen(USERS_FILE, "r");
    if (!f) {
        return -1;
    }

    char line[256], u[128], p[128];
    int found = 0;

    while (fgets(line, sizeof(line), f)) {
        if (sscanf(line, "%127s %127s", u, p) == 2 &&
            strcmp(u, name) == 0) {
            snprintf(passOut, size, "%s", p);
            found = 1;
            break;
        }
    }

    fclose(f);
    return found;
}

static const char *askClient(struct sockaddr_in *cli, socklen_t len,
                             const char *prompt, char *buf) {
    char pkt[BUF + 8];
    snprintf(pkt, sizeof(pkt), "ASK|%s", prompt);
    sendTo(cli, len, pkt);

    for (;;) {
        if (recvFrom(buf, cli, &len) < 0) {
            die("recvfrom");
        }

        if (strncmp(buf, "INP|", 4) == 0) {
            return buf + 4;
        }

        printf("[SERVER] Ignored unexpected packet: %.40s...\n", buf);
    }
}

static int authenticate(struct sockaddr_in *cli, socklen_t len,
                        char *username) {
    char buf[BUF], storedPass[128];

    for (int attempt = 1; attempt <= 3; attempt++) {
        const char *name = askClient(cli, len, "Username: ", buf);
        printf("[AUTH] Login attempt for username '%s'\n", name);

        int r = findUser(name, storedPass, sizeof(storedPass));

        if (r == -1) {
            sendTo(cli, len,
                   "SHOW|\n[ERROR] User database file 'users.txt' was not found.\n");
            printf("[ERROR] User database file '%s' is missing.\n", USERS_FILE);
            return 0;
        }

        if (r == 0) {
            sendTo(cli, len,
                   "SHOW|\n[LOGIN FAILED] Username not found. Please try again.\n");
            printf("[AUTH] Unknown username '%s'.\n", name);
            continue;
        }

        sendTo(cli, len, "GETPASS|Password: ");

        for (;;) {
            if (recvFrom(buf, cli, &len) < 0) {
                die("recvfrom");
            }
            if (strncmp(buf, "INP|", 4) == 0) {
                break;
            }
        }

        const char *password = buf + 4;
        printf("[AUTH] Password received for '%s' (hidden).\n", name);

        if (strcmp(storedPass, password) == 0) {
            sendTo(cli, len, "SHOW|\n[LOGIN SUCCESS] Welcome, ");
            sendTo(cli, len, name);
            sendTo(cli, len, "!\n");

            strcpy(username, name);
            printf("[AUTH] User '%s' authenticated successfully.\n", name);
            return 1;
        }

        sendTo(cli, len,
               "SHOW|\n[LOGIN FAILED] Incorrect password. Please try again.\n");
        printf("[AUTH] Incorrect password for '%s' (attempt %d of 3).\n",
               name, attempt);
    }

    sendTo(cli, len,
           "SHOW|\n[LOGIN FAILED] Too many unsuccessful attempts. Disconnecting.\n");
    sendTo(cli, len, "EXIT|");
    printf("[AUTH] Client disconnected after three failed attempts.\n");
    return 0;
}

static void arithmeticSession(struct sockaddr_in *cli, socklen_t len,
                              const char *user) {
    char buf[BUF];

    printf("[SESSION] %s entered arithmetic mode.\n", user);
    sendTo(cli, len,
           "SHOW|\n"
           "================================\n"
           "         ARITHMETIC MODE\n"
           "================================\n"
           "Enter one value at a time. Type 'back' to return to the menu.\n");

    for (;;) {
        const char *input = askClient(cli, len, "First number: ", buf);

        if (strcmp(input, "back") == 0) {
            printf("[SESSION] %s left arithmetic mode.\n", user);
            sendTo(cli, len, "SHOW|\n[SESSION] Returning to the main menu.\n");
            return;
        }

        char *end;
        double a = strtod(input, &end);

        if (end == input || *end != '\0') {
            sendTo(cli, len, "SHOW|\n[ERROR] Please enter a valid number.\n");
            continue;
        }

        input = askClient(cli, len, "Operator (+, -, *, /): ", buf);

        if (strcmp(input, "back") == 0) {
            printf("[SESSION] %s left arithmetic mode.\n", user);
            sendTo(cli, len, "SHOW|\n[SESSION] Returning to the main menu.\n");
            return;
        }

        if (input[0] == '\0' || input[1] != '\0' ||
            (input[0] != '+' && input[0] != '-' &&
             input[0] != '*' && input[0] != '/')) {
            sendTo(cli, len,
                   "SHOW|\n[ERROR] Choose one of these operators: +, -, *, /.\n");
            continue;
        }

        char op = input[0];

        input = askClient(cli, len, "Second number: ", buf);

        if (strcmp(input, "back") == 0) {
            printf("[SESSION] %s left arithmetic mode.\n", user);
            sendTo(cli, len, "SHOW|\n[SESSION] Returning to the main menu.\n");
            return;
        }

        double b = strtod(input, &end);

        if (end == input || *end != '\0') {
            sendTo(cli, len, "SHOW|\n[ERROR] Please enter a valid number.\n");
            continue;
        }

        double result;

        switch (op) {
            case '+':
                result = a + b;
                break;
            case '-':
                result = a - b;
                break;
            case '*':
                result = a * b;
                break;
            case '/':
                if (b == 0) {
                    sendTo(cli, len, "SHOW|\n[ERROR] Division by zero is not allowed.\n");
                    continue;
                }
                result = a / b;
                break;
            default:
                continue;
        }

        char reply[BUF];
        snprintf(reply, sizeof(reply), "SHOW|\n[RESULT] %g %c %g = %g\n",
                 a, op, b, result);
        sendTo(cli, len, reply);

        printf("[CALC] %s: %g %c %g = %g\n", user, a, op, b, result);
    }
}

static void chatSession(struct sockaddr_in *cli, socklen_t len,
                        const char *user) {
    char buf[BUF], line[BUF], pkt[BUF + 8];

    printf("[SESSION] %s entered chat mode.\n", user);
    sendTo(cli, len,
           "SHOW|\n"
           "================================\n"
           "             CHAT\n"
           "================================\n"
           "You speak first. Type 'over' to hand over your turn or 'bye' to end chat.\n");
    sendTo(cli, len, "CHATGO|");

    int end = 0;

    while (!end) {
        while (1) {
            if (recvFrom(buf, cli, &len) < 0) {
                die("recvfrom");
            }

            if (strncmp(buf, "CHAT|", 5) != 0) {
                continue;
            }

            const char *msg = buf + 5;
            printf("[CHAT] %s: %s\n", user, msg);

            if (strcasecmp(msg, "bye") == 0) {
                end = 1;
                break;
            }

            if (strcasecmp(msg, "over") == 0) {
                break;
            }
        }

        if (end) {
            break;
        }

        printf("\n[CHAT] Your turn. Type 'over' to hand over or 'bye' to end chat.\n");

        while (1) {
            printf("[YOU] ");

            if (!fgets(line, sizeof(line), stdin)) {
                end = 1;
                break;
            }

            line[strcspn(line, "\n")] = '\0';
            snprintf(pkt, sizeof(pkt), "CHAT|%s", line);
            sendTo(cli, len, pkt);

            printf("[CHAT] Server: %s\n", line);

            if (strcasecmp(line, "bye") == 0) {
                end = 1;
                break;
            }

            if (strcasecmp(line, "over") == 0) {
                break;
            }
        }
    }

    sendTo(cli, len, "SHOW|\n[CHAT] Chat ended.\n");
    printf("[SESSION] %s left chat mode.\n", user);
}

int main(void) {
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        die("socket");
    }

    struct sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        die("bind");
    }

    printf("[SERVER] UDP server is running on port %d.\n", PORT);

    for (;;) {
        struct sockaddr_in cli;
        socklen_t cliLen = sizeof(cli);
        char buf[BUF], username[128];

        printf("\n[SERVER] Waiting for a client...\n");

        if (recvFrom(buf, &cli, &cliLen) < 0) {
            die("recvfrom");
        }

        printf("[SERVER] Client connected from %s:%d.\n",
               inet_ntoa(cli.sin_addr), ntohs(cli.sin_port));

        if (authenticate(&cli, cliLen, username) != 1) {
            continue;
        }

        int session = 1;

        while (session) {
            sendTo(&cli, cliLen,
                   "SHOW|\n"
                   "================================\n"
                   "           MAIN MENU\n"
                   "================================\n"
                   "  1) Arithmetic\n"
                   "  2) Chat\n"
                   "  3) Logout\n");

            const char *choice =
                askClient(&cli, cliLen, "Select an option: ", buf);

            printf("[MENU] %s selected option %s.\n", username, choice);

            switch (atoi(choice)) {
                case 1:
                    arithmeticSession(&cli, cliLen, username);
                    break;

                case 2:
                    chatSession(&cli, cliLen, username);
                    break;

                case 3:
                    sendTo(&cli, cliLen,
                           "SHOW|\n[SESSION] You have been logged out. Goodbye!\n");
                    sendTo(&cli, cliLen, "EXIT|");
                    printf("[SESSION] %s logged out.\n", username);
                    session = 0;
                    break;

                default:
                    sendTo(&cli, cliLen,
                           "SHOW|\n[ERROR] Invalid selection. Choose 1, 2, or 3.\n");
                    printf("[MENU] %s entered invalid option '%s'.\n",
                           username, choice);
                    break;
            }
        }
    }

    close(sock);
    return 0;
}
