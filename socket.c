#include "raft.h"

void* run_socket(void* arg) {
    thread_args* args = (thread_args*) arg;
    RaftNode* node = args->node;
    struct sockaddr_in* nodes = args->nodes;

    while (1) {
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);

        recvfrom(node->socket_fd, (char*)buffer, 1024, MSG_WAITALL, (struct sockaddr*)&addr, &len);
        printf("Node %d received: %s\n", node->node_id, buffer);

        if (strncmp(buffer, "REQUEST_VOTE", 12) == 0) {
            int term, candidate_id;
            sscanf(buffer, "REQUEST_VOTE %d %d", &term, &candidate_id);

            // TODO: 투표 요청 처리 나 보다 임기가 작은 아이의 요청

            if (node->voted_for == -1) {
                node->voted_for = candidate_id;
                sendto(node->socket_fd, "VOTE_GRANTED", 12, 0, (const struct sockaddr*)&addr, len);
            }
        } else if (strncmp(buffer, "VOTE_GRANTED", 12) == 0) {
            node->votes++;

            if (node->votes > NUM_NODES / 2) {
                node->state = LEADER;
                printf("Node %d is now the leader\n", node->node_id);
                for (int i = 1; i < NUM_NODES; i++) {
                    sendto(node->socket_fd, "LEADER_ELECTED ", 14, 0, (const struct sockaddr*)&nodes[i], len);
                }
            }
        } else if (strncmp(buffer, "HEARTBEAT", 9) == 0) {
            int term, leader_id;
            sscanf(buffer, "HEARTBEAT term: %d, leader_id: %d", &term, &leader_id);

            node->current_term = term;
            node->leader_id = leader_id;
            node->last_heartbeat = time(NULL);
            node->voted_for = -1;
            node->votes = 0;
            node->state = FOLLOWER;
        }
    }
}
