#include "raft.h"
#include "store.h"

void init_node(RaftNode* node, int id, const char* ip, int port) {
    node->node_id = id;
    node->current_term = 0;
    node->voted_for = -1;
    node->votes = 0;
    node->state = FOLLOWER;
    node->last_heartbeat = time(NULL);
    node->election_timeout = ((double)rand() / RAND_MAX) * 2.0 + 1.0;

    // 소켓 초기화
    node->socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (node->socket_fd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&node->addr, 0, sizeof(node->addr));
    node->addr.sin_family = AF_INET;
    node->addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &node->addr.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        exit(EXIT_FAILURE);
    }

    if (bind(node->socket_fd, (const struct sockaddr*)&node->addr, sizeof(node->addr)) < 0) {
        perror("bind failed");
        close(node->socket_fd);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <node_id> <ip:port> [other ip:port...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int node_id = atoi(argv[1]);
    char* ip = strtok(argv[2], ":");
    int port = atoi(strtok(NULL, ":"));

    thread_args arg;
    arg.node = (RaftNode*)malloc(sizeof(RaftNode));
    init_node(arg.node, node_id, ip, port);

    // 차례대로 노드들의 주소를 저장
    for (int i = 0; i < argc - 2; i++) {
        sscanf(argv[i + 2], "%[^:]:%d", ip, &port);

        memset(&arg.nodes[i], 0, sizeof(struct sockaddr_in));
        arg.nodes[i].sin_family = AF_INET;
        arg.nodes[i].sin_port = htons(port);
        inet_pton(AF_INET, ip, &arg.nodes[i].sin_addr);
    }

    // table 초기화
    memset(table, 0, sizeof(Entry*) * TABLE_SIZE);

    pthread_t node_thread, socket_thread;

    pthread_create(&node_thread, NULL, run_node, (void*)&arg);
    pthread_create(&socket_thread, NULL, run_socket, (void*)&arg);

    pthread_join(node_thread, NULL);
    pthread_join(socket_thread, NULL);

    return 0;
}
