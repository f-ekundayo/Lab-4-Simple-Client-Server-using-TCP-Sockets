#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <limits.h>

#include "list.h"

/* ======== CONFIG ======== */
#define PORT 8088               /* choose a free port */
#define ACK  "ACK: "

/* ======== GLOBALS for graceful shutdown ======== */
static int g_serv_fd = -1;
static int g_client_fd = -1;
static list_t *g_list = NULL;

static void cleanup_and_exit(int code) {
  if (g_client_fd != -1) close(g_client_fd);
  if (g_serv_fd != -1) close(g_serv_fd);
  if (g_list) { list_free(g_list); g_list = NULL; }
  exit(code);
}

static void handle_sigint(int sig) {
  (void)sig;
  fprintf(stderr, "\nSIGINT received. Cleaning up and exiting...\n");
  cleanup_and_exit(0);
}

static void send_string(int fd, const char *s) {
  /* send only the string length (not full fixed buffer) */
  size_t n = strlen(s);
  if (n == 0) { const char *e=""; send(fd, e, 0, 0); return; }
  send(fd, s, n, 0);
}

int main(void) {
  signal(SIGINT, handle_sigint);

  /* Create server socket */
  g_serv_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (g_serv_fd == -1) { perror("socket"); return 1; }

  /* Reuse addr */
  int opt = 1;
  setsockopt(g_serv_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  struct sockaddr_in servAddr;
  memset(&servAddr, 0, sizeof(servAddr));
  servAddr.sin_family = AF_INET;
  servAddr.sin_port = htons(PORT);
  servAddr.sin_addr.s_addr = INADDR_ANY;

  if (bind(g_serv_fd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
    perror("bind");
    cleanup_and_exit(1);
  }

  if (listen(g_serv_fd, 1) < 0) {
    perror("listen");
    cleanup_and_exit(1);
  }

  printf("Server listening on port %d ...\n", PORT);

  g_client_fd = accept(g_serv_fd, NULL, NULL);
  if (g_client_fd < 0) {
    perror("accept");
    cleanup_and_exit(1);
  }
  printf("Client connected.\n");

  g_list = list_alloc();
  if (!g_list) {
    fprintf(stderr, "Failed to alloc list\n");
    cleanup_and_exit(1);
  }

  char rbuf[1024];
  char sbuf[1024];
  while (1) {
    ssize_t n = recv(g_client_fd, rbuf, sizeof(rbuf)-1, 0);
    if (n <= 0) {
      if (n < 0) perror("recv");
      else fprintf(stderr, "Client disconnected.\n");
      cleanup_and_exit(0);
    }
    rbuf[n] = '\0';

    /* Tokenize command */
    char *token = strtok(rbuf, " \t\r\n");
    if (!token) { strcpy(sbuf, "ERR: empty command"); send_string(g_client_fd, sbuf); continue; }

    if (strcmp(token, "exit") == 0) {
      strcpy(sbuf, "Goodbye.");
      send_string(g_client_fd, sbuf);
      cleanup_and_exit(0);
    }
    else if (strcmp(token, "get_length") == 0) {
      snprintf(sbuf, sizeof(sbuf), "Length = %zu", list_length(g_list));
    }
    else if (strcmp(token, "print") == 0) {
      snprintf(sbuf, sizeof(sbuf), "%s", listToString(g_list));
    }
    else if (strcmp(token, "add_back") == 0) {
      char *v = strtok(NULL, " \t\r\n");
      if (!v) { snprintf(sbuf, sizeof(sbuf), "ERR: missing <value>"); }
      else {
        int val = atoi(v);
        list_add_to_back(g_list, val);
        snprintf(sbuf, sizeof(sbuf), ACK "%d", val);
      }
    }
    else if (strcmp(token, "add_front") == 0) {
      char *v = strtok(NULL, " \t\r\n");
      if (!v) { snprintf(sbuf, sizeof(sbuf), "ERR: missing <value>"); }
      else {
        int val = atoi(v);
        list_add_to_front(g_list, val);
        snprintf(sbuf, sizeof(sbuf), ACK "%d", val);
      }
    }
    else if (strcmp(token, "add_position") == 0) {
      char *i = strtok(NULL, " \t\r\n");
      char *v = strtok(NULL, " \t\r\n");
      if (!i || !v) {
        snprintf(sbuf, sizeof(sbuf), "ERR: usage add_position <index> <value>");
      } else {
        int idx = atoi(i), val = atoi(v);
        int rc = list_add_at_index(g_list, idx, val);
        if (rc == 0) snprintf(sbuf, sizeof(sbuf), ACK "%d", val);
        else snprintf(sbuf, sizeof(sbuf), "ERR: bad index");
      }
    }
    else if (strcmp(token, "remove_back") == 0) {
      int removed = list_remove_from_back(g_list);
      if (removed == INT_MIN) snprintf(sbuf, sizeof(sbuf), "ERR: empty");
      else snprintf(sbuf, sizeof(sbuf), "%d", removed);
    }
    else if (strcmp(token, "remove_front") == 0) {
      int removed = list_remove_from_front(g_list);
      if (removed == INT_MIN) snprintf(sbuf, sizeof(sbuf), "ERR: empty");
      else snprintf(sbuf, sizeof(sbuf), "%d", removed);
    }
    else if (strcmp(token, "remove_position") == 0) {
      char *i = strtok(NULL, " \t\r\n");
      if (!i) snprintf(sbuf, sizeof(sbuf), "ERR: usage remove_position <index>");
      else {
        int idx = atoi(i);
        int removed = list_remove_at_index(g_list, idx);
        if (removed == INT_MIN) snprintf(sbuf, sizeof(sbuf), "ERR: bad index");
        else snprintf(sbuf, sizeof(sbuf), ACK "%d", removed);
      }
    }
    else if (strcmp(token, "get") == 0) {
      char *i = strtok(NULL, " \t\r\n");
      if (!i) snprintf(sbuf, sizeof(sbuf), "ERR: usage get <index>");
      else {
        int idx = atoi(i);
        int val = list_get_elem_at(g_list, idx);
        if (val == INT_MIN) snprintf(sbuf, sizeof(sbuf), "ERR: bad index");
        else snprintf(sbuf, sizeof(sbuf), "%d", val);
      }
    }
    else {
      snprintf(sbuf, sizeof(sbuf), "ERR: unknown command");
    }

    send_string(g_client_fd, sbuf);
  }
  /* not reached */
  return 0;
}
