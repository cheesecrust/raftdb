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

        // printf("Node %d is listening for messages\n", node->node_id);
        recvfrom(node->socket_fd, (char*)buffer, 1024, MSG_WAITALL, (struct sockaddr*)&addr, &len);
        printf("Node %d received: %s\n", node->node_id, buffer);

        if (strncmp(buffer, "REQUEST_VOTE", 12) == 0) {
            int term, candidate_id;
            sscanf(buffer, "REQUEST_VOTE %d %d", &term, &candidate_id);
            if (term > node->current_term) {
                node->current_term = term;
                node->state = FOLLOWER;
                node->voted_for = -1;
            }
            if (node->voted_for == -1) {
                node->voted_for = candidate_id;
                sendto(node->socket_fd, "VOTE_GRANTED", 12, 0, (const struct sockaddr*)&addr, len);
            }
        } else if (strncmp(buffer, "HEARTBEAT", 9) == 0) {
            // TODO: implement
        }
    }
}