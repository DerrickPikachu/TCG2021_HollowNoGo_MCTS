//
// Created by pikachin on 2021/12/6.
//

#ifndef TCG_NOGO_MCTS_MCTS_H
#define TCG_NOGO_MCTS_MCTS_H

#include <unordered_map>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <cmath>
#include <random>

class Mcts {
private:
    struct Node {
        int visitCount;
        int wins;
        board position;
        std::vector<Node*> childs;
        Node(board b) : visitCount(0), wins(0), position(b) {}
    };

public:
    Mcts(std::vector<action::place> s) : space(s) {
        resetMcts();
        engine.seed(12345);
    }

    void setupRoot(board& b) {
        root = new Node(b);
    }

    void resetMcts() {
        root = nullptr;
    }

    void search(int timesOfMcts) {
        for (int i = 0; i < timesOfMcts; i++) {

        }
    }

private:
    int traverse(Node* node, bool isOpponent=false) {
        if (node->childs.empty()) {  // expand and simulate
            simulate(node, isOpponent);
        } else {
            Node* nextNode = select(node);
            int result = traverse(nextNode, !isOpponent);
            // update
            return result;
        }
    }

    Node* select(Node* node) {
        float bestScore = 0;
        Node* nextNode;
        for (Node* child : node->childs) {
            float score = uct(*child, node->visitCount);
            if (bestScore < score) {
                bestScore = score;
                nextNode = child;
            }
        }
        return nextNode;
    }

    // Need test
    int simulate(Node* node, bool isOpponent) {
        board curPosition = node->position;
        action::place randomMove = getRandomAction(curPosition);
        while (randomMove.apply(curPosition) == board::legal) {
            isOpponent = !isOpponent;
            randomMove = getRandomAction(curPosition);
        }
        return isOpponent;
    }

    std::vector<Node*> expand(Node* node) {

    }

    action::place getRandomAction(const board& position) {
        std::vector<action::place> temSpace = space;
        std::shuffle(temSpace.begin(), temSpace.end(), engine);
        for (action::place& move : temSpace) {
            board nextBoard = position;
            if (move.apply(nextBoard) == board::legal) {
                return move;
            }
        }
        return temSpace[0];
    }

    std::vector<board> getPossibleBoard(const board& b) {

    }

    float uct(Node& node, int parentVisitCount) {
        float c = 0.3;
        float exploitation = (float)node.wins / (float)(node.visitCount + 1);
        float exploration = sqrt(log(parentVisitCount) / (float)(node.visitCount + 1));
        return exploitation + c * exploration;
    }

    std::string appendPath(std::string path, const action::place& move) {
        std::string moveCode = std::to_string(move.position().x) + std::to_string(move.position().y);
        return path + "_" + moveCode;
    }

private:
    std::vector<action::place> space;
    Node* root;
    std::default_random_engine engine;
};


#endif //TCG_NOGO_MCTS_MCTS_H
