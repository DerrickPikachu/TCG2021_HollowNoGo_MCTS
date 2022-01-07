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
    Mcts() {
        srand(time(NULL));
        engine.seed(rand() % 100000);
        int actionSize = board::size_x * board::size_y;
        actions.reserve(actionSize);
        for (int i = 0; i < actionSize; i++)
            actions.push_back(board::point(i));
        // TODO: Need test
    }
    Mcts(board::piece_type type) : Mcts() {
        setWho(type);
    }

    void setWho(board::piece_type type) {
        who = type;
    }

    void setUctType(std::string type) {
        uctType = type;
    }

    void setupRoot(const board& b) {
        root = new Node(b);
    }

    void resetMcts(Node* node=nullptr) {
        if (node == nullptr)
            node = root;
        for (int i = 0; i < (int)node->childs.size(); i++)
            resetMcts(node->childs[i]);
        delete node;
    }

    void search(int timesOfMcts) {
        for (int i = 0; i < timesOfMcts; i++)
            traverse(root);
    }

    board::point chooseAction() {
        if (root->childs.empty())
            return board::point(0);
        int bestCount = 0;
        Node* bestNode = root->childs[0];
        for (int i = 0; i < (int)root->childs.size(); i++) {
            if (bestCount < root->childs[i]->visitCount) {
                bestCount = root->childs[i]->visitCount;
                bestNode = root->childs[i];
            }
        }
        return findActionByNextBoard(bestNode->position);
    }

private:  // After testing, it should be private
    int traverse(Node* node, bool isOpponent=false) {
        if (node->childs.empty()) {  // expand and simulate
            int result = simulate(node->position, isOpponent);
            expand(node, isOpponent);
            update(node, result);
            return result;
        } else {
            Node* nextNode = select(node, isOpponent);
//            std::cout << nextNode->position << std::endl;
            int result = traverse(nextNode, !isOpponent);
            update(node, result);
            return result;
        }
    }

    Node* select(Node* node, bool isOpponent) {
        float bestScore = 0;
        std::vector<Node*> nextNodes;
        for (Node* child : node->childs) {
            float score = uct(*child, node->visitCount, isOpponent);
            if (bestScore < score) {
                bestScore = score;
                nextNodes.clear();
                nextNodes.push_back(child);
            } else if (bestScore == score) {
                nextNodes.push_back(child);
            }
        }
        std::shuffle(nextNodes.begin(), nextNodes.end(), engine);
        if (nextNodes.empty()) {
            std::cerr << "select error" << std::endl;
            exit(0);
        }
        return nextNodes[0];
    }

    int simulate(const board& position, bool isOpponent) {
        std::string test;
        board curPosition = position;
        board::point randomMove = getRandomAction(curPosition, isOpponent);
        while (curPosition.place(randomMove) == board::legal) {
            isOpponent = !isOpponent;
            randomMove = getRandomAction(curPosition, isOpponent);
        }
        return isOpponent;
    }

    void expand(Node* node, bool isOpponent) {
        std::vector<Node*> childs;
        std::vector<board::point> copyActions = actions;
        for (board::point& move : copyActions) {
            board curPosition = node->position;
            if (curPosition.place(move) == board::legal)
                childs.push_back(new Node(curPosition));
        }
        node->childs = childs;
    }

    void update(Node* node, int result) {
        node->visitCount++;
        node->wins += result;
    }

    board::point getRandomAction(const board& position, bool isOpponent) {
        std::vector<board::point> temSpace = actions;
        std::shuffle(temSpace.begin(), temSpace.end(), engine);
        for (board::point& move : temSpace) {
            board nextBoard = position;
            if (nextBoard.place(move) == board::legal) {
                return move;
            }
        }
        return temSpace[0];
    }

    bool isBlackTurn(bool isOpponent) {
        return (!isOpponent && who == board::black) || (isOpponent && who == board::white);
    }

    float uct(Node& node, int parentVisitCount, bool isOpponent) {
        if (node.visitCount == 0) return 10000.0;
        float c = 1.5;
        float winRate = (float)node.wins / (float)(node.visitCount + 1);
        float exploitation = (isOpponent && uctType == "anti")? 1 - winRate : winRate;
        float exploration = sqrt(log(parentVisitCount) / (float)(node.visitCount + 1));
        return exploitation + c * exploration;
    }

    board::point findActionByNextBoard(const board& nextBoard) {
        std::vector<board::point> temSpace = actions;
        for (board::point& move : temSpace) {
            board position = root->position;
            if (position.place(move) == board::legal && position == nextBoard)
                return move;
        }
        std::cerr << "find action error" << std::endl;
        exit(0);  // Error with call
    }

    std::string appendPath(std::string path, const action::place& move) {
        std::string moveCode = std::to_string(move.position().x) + std::to_string(move.position().y);
        return path + "_" + moveCode;
    }

private:
    Node* root;
    std::vector<board::point> actions;
    board::piece_type who;
    std::default_random_engine engine;
    std::string uctType;
};


#endif //TCG_NOGO_MCTS_MCTS_H
