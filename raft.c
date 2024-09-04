#include "raft.h"

void* run_node(void* arg) {
    thread_args* args = (thread_args*) arg;
    RaftNode* node = args->node;
    struct sockaddr_in* nodes = args->nodes;

    for (int i = 0; i < 3; i++) {
        printf("Node %d: %s:%d\n", i, inet_ntoa(nodes[i].sin_addr), ntohs(nodes[i].sin_port));
    }

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
        usleep(2000000); // 2초 간격으로 실행
    }
    return NULL;
}

void follower_behavior(RaftNode* node) {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    while (1) {
        /* code */
        if (difftime(time(NULL), node->last_heartbeat) > node->election_timeout) {
            node->state = CANDIDATE;
            printf("Node %d timed out, becoming candidate\n", node->node_id);
            break;
        }
    }

}

void candidate_behavior(RaftNode* node, struct sockaddr_in* nodes) {
    node->current_term++;
    node->voted_for = node->node_id;
    int votes = 1; // 자신에게 투표

    printf("Node %d is starting an election for term %d\n", node->node_id, node->current_term);

    // 투표 요청 전송
    request_vote(node, nodes, node->current_term, node->node_id);

}

int request_vote(RaftNode* node, struct sockaddr_in* nodes, int term, int candidate_id) {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    char buffer[1024];

    sprintf(buffer, "REQUEST_VOTE %d %d", term, candidate_id);

    for (int i = 1; i < NUM_NODES; i++) {
        printf("Node %d is requesting vote to node %d\n", node->node_id, ntohs(nodes[i].sin_port));
        sendto(node->socket_fd, buffer, strlen(buffer), 0, (const struct sockaddr*)&nodes[i], sizeof(nodes[i]));
    }

    return 0;
}

void leader_behavior(RaftNode* node, struct sockaddr_in* nodes) {
    printf("Node %d is sending heartbeats to followers\n", node->node_id);
    send_heartbeat(node, nodes);  // 이 부분을 수정해서 전체 노드에게 보냅니다.
    sleep(1); // heartbeat 간격
}

void send_heartbeat(RaftNode* node, struct sockaddr_in* nodes) {
    char buffer[1024];
    sprintf(buffer, "HEARTBEAT %d %d", node->current_term, node->node_id);

    for (int i = 0; i < NUM_NODES; i++) {
        if (i != node->node_id) {
            sendto(node->socket_fd, buffer, strlen(buffer), 0, (const struct sockaddr*)&nodes[i], sizeof(nodes[i]));
        }
    }
}

void receive_heartbeat(RaftNode* node, int term, int leader_id) {
    if (term >= node->current_term) {
        node->current_term = term;
        node->leader_id = leader_id;
        node->state = FOLLOWER;
        node->last_heartbeat = time(NULL);
    }
}

// void set_nonblocking(int sockfd) {
//     int flags = fcntl(sockfd, F_GETFL, 0);
//     if (flags < 0) {
//         perror("fcntl(F_GETFL) failed");
//         exit(EXIT_FAILURE);
//     }
//     if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
//         perror("fcntl(F_SETFL) failed");
//         exit(EXIT_FAILURE);
//     }
// }

// void set_blocking(int sockfd) {
//     int flags = fcntl(sockfd, F_GETFL, 0);
//     if (flags < 0) {
//         perror("fcntl(F_GETFL) failed");
//         exit(EXIT_FAILURE);
//     }
//     if (fcntl(sockfd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
//         perror("fcntl(F_SETFL) failed");
//         exit(EXIT_FAILURE);
//     }
// }
