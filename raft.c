#include "raft.h"

void* run_node(void* arg) {
    thread_args* args = (thread_args*) arg;
    RaftNode* node = args->node;
    struct sockaddr_in* nodes = args->nodes;

    while (1) {
        switch (node->state) {
            case FOLLOWER:
                follower_behavior(node);
                break;
            case CANDIDATE:
                candidate_behavior(node, nodes);
                break;
            case LEADER:
                leader_behavior(node, nodes);
                break;
        }
    }
    return NULL;
}

void follower_behavior(RaftNode* node) {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    struct timespec timeout;
    char buffer[1024];
    double diff;
    memset(buffer, 0, sizeof(buffer));

    do {
        clock_gettime(CLOCK_REALTIME, &timeout);
        long sec = timeout.tv_sec - node->last_heartbeat.tv_sec;
        long nsec = timeout.tv_nsec - node->last_heartbeat.tv_nsec;
        // 나노초 차이가 음수인 경우 보정
        if (nsec < 0) {
            sec--;
            nsec += 1000000000;
        }
        diff = sec * 1000.0 + nsec / 1000000.0;
    } while (diff <= node->election_timeout);

    node->state = CANDIDATE;
    printf("Node %d is now candidate\n", node->node_id);
}

void candidate_behavior(RaftNode* node, struct sockaddr_in* nodes) {
    node->current_term++;
    node->voted_for = node->node_id;
    node->votes = 1; // 자신에게 투표
    double start_time = time(NULL);

    // 투표 요청 전송
    while (node->state == CANDIDATE) {
        request_vote(node, nodes, node->current_term, node->node_id);

        if (difftime(time(NULL), start_time) > MAX_ELECTION_TIMEOUT) {
            node->voted_for = -1;
            node->state = FOLLOWER;
            node->election_timeout = (double)(rand() % 150) + 151.0;
            return;
        }
    }
}

int request_vote(RaftNode* node, struct sockaddr_in* nodes, int term, int candidate_id) {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    char buffer[1024];

    sprintf(buffer, "REQUEST_VOTE %d %d", term, candidate_id);

    for (int i = 1; i < node->num_nodes; i++) {
        sendto(node->socket_fd, buffer, strlen(buffer), 0, (const struct sockaddr*)&nodes[i], sizeof(nodes[i]));
    }

    return 0;
}

void leader_behavior(RaftNode* node, struct sockaddr_in* nodes) {
    send_heartbeat(node, nodes);  // 전체 노드에게 보냅니다.
}

void send_heartbeat(RaftNode* node, struct sockaddr_in* nodes) {
    char buffer[1024];
    sprintf(buffer, "HEARTBEAT %d IP %s PORT %d", node->current_term, node->leader_ip, node->leader_port);

    for (int i = 1; i < node->num_nodes; i++) {
        sendto(node->socket_fd, buffer, strlen(buffer), 0, (const struct sockaddr*)&nodes[i], sizeof(nodes[i]));
    }
}
