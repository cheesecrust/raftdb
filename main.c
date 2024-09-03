#include "raft.h"

void init_node(RaftNode* node, int id, const char* ip, int port) {
    node->node_id = id;
    node->current_term = 0;
    node->voted_for = -1;
    node->state = FOLLOWER;
    node->leader_id = -1;
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
    printf("%d, %s, %s, %s",argc, argv[0], argv[1], argv[2]);
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <node_id> <ip:port> [other ip:port...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int node_id = atoi(argv[1]);
    char* ip = strtok(argv[2], ":");
    int port = atoi(strtok(NULL, ":"));

    RaftNode node;
    init_node(&node, node_id, ip, port);

    RaftNode nodes[NUM_NODES];
    nodes[node_id] = node;

    // 다른 노드들의 IP와 포트 정보 저장
    for (int i = 1; i < argc - 2; i++) {
        ip = strtok(argv[i + 2], ":");
        port = atoi(strtok(NULL, ":"));
        init_node(&nodes[i], i, ip, port);
    }

    pthread_t node_thread;
    pthread_create(&node_thread, NULL, run_node, (void*)&node);
    pthread_join(node_thread, NULL);

    return 0;
}
