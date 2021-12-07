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
#include <ctime>

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
    Mcts(board::piece_type type) : who(type),
                                   blackSpace(board::size_x * board::size_y),
                                   whiteSpace(board::size_x * board::size_y){
        resetMcts();
        srand(time(NULL));
        engine.seed(rand() % 100000);
        for (int i = 0; i < (int)blackSpace.size(); i++)
            blackSpace[i] = action::place(i, board::black);
        for (int i = 0; i < (int)whiteSpace.size(); i++)
            whiteSpace[i] = action::place(i, board::white);
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

public:  // After testing, it should be private
    int traverse(Node* node, bool isOpponent=false) {
        if (node->childs.empty()) {  // expand and simulate
            simulate(node->position, isOpponent);
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

    int simulate(const board& position, bool isOpponent) {
        board curPosition = position;
        action::place randomMove = getRandomAction(curPosition, isOpponent);
        while (randomMove.apply(curPosition) == board::legal) {
            isOpponent = !isOpponent;
            randomMove = getRandomAction(curPosition, isOpponent);
        }
        return isOpponent;
    }

    std::vector<Node*> expand(Node* node) {

    }

    action::place getRandomAction(const board& position, bool isOpponent) {
        std::vector<action::place> temSpace;
        if ((!isOpponent && who == board::black) || (isOpponent && who == board::white))
            temSpace = blackSpace;
        if ((!isOpponent && who == board::white) || (isOpponent && who == board::black))
            temSpace = whiteSpace;
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
    std::vector<action::place> blackSpace;
    std::vector<action::place> whiteSpace;
    board::piece_type who;
    Node* root;
    std::default_random_engine engine;
};


#endif //TCG_NOGO_MCTS_MCTS_H
