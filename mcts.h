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
    Mcts(board::piece_type type) : blackSpace(board::size_x * board::size_y),
                                   whiteSpace(board::size_x * board::size_y),
                                   who(type) {
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
            int result = simulate(node->position, isOpponent);
            expand(node, isOpponent);
            update(node, result);
            return result;
        } else {
            Node* nextNode = select(node);
            std::cout << nextNode->position << std::endl;
            int result = traverse(nextNode, !isOpponent);
            update(node, result);
            return result;
        }
    }

    Node* select(Node* node) {
        float bestScore = 0;
        Node* nextNode = node->childs[0];
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
        std::string test;
        board curPosition = position;
        action::place randomMove = getRandomAction(curPosition, isOpponent);
        while (randomMove.apply(curPosition) == board::legal) {
//            std::cout << curPosition << std::endl;
//            std::cin >> test;
            isOpponent = !isOpponent;
            randomMove = getRandomAction(curPosition, isOpponent);
        }
        return isOpponent;
    }

    void expand(Node* node, bool isOpponent) {
        std::vector<Node*> childs;
        std::vector<action::place>& nextSpace = (isBlackTurn(isOpponent)) ? blackSpace : whiteSpace;
        std::shuffle(nextSpace.begin(), nextSpace.end(), engine);
        for (action::place& move : nextSpace) {
            board curPosition = node->position;
            if (move.apply(curPosition) == board::legal)
                childs.push_back(new Node(curPosition));
        }
        node->childs = childs;
    }

    void update(Node* node, int result) {
        node->visitCount++;
        node->wins += result;
    }

    action::place getRandomAction(const board& position, bool isOpponent) {
        std::vector<action::place> temSpace = (isBlackTurn(isOpponent))? blackSpace : whiteSpace;
        std::shuffle(temSpace.begin(), temSpace.end(), engine);
        for (action::place& move : temSpace) {
            board nextBoard = position;
            if (move.apply(nextBoard) == board::legal) {
                return move;
            }
        }
        return temSpace[0];
    }

    bool isBlackTurn(bool isOpponent) {
        return (!isOpponent && who == board::black) || (isOpponent && who == board::white);
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

    Node* root;
private:
    std::vector<action::place> blackSpace;
    std::vector<action::place> whiteSpace;
    board::piece_type who;
    std::default_random_engine engine;
};


#endif //TCG_NOGO_MCTS_MCTS_H
