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
#include <stack>
#include <utility>

class Mcts {
private:
    struct Node {
        int visitCount;
        int wins;
        board position;
        std::vector<board::point> legal;
        std::vector<Node*> childs;
        Node(board b, std::vector<board::point> actions, std::default_random_engine& engine) :
            visitCount(0), wins(0), position(b) {
            legal.reserve(actions.size());
            std::shuffle(actions.begin(), actions.end(), engine);
            for (int i = 0; i < (int)actions.size(); i++) {
                board tem = b;
                if (tem.place(actions[i]) == board::legal)
                    legal.push_back(actions[i]);
            }
        }
    };

public:
    Mcts() {
        srand(time(NULL));
        engine.seed(rand() % 100000);
        int actionSize = board::size_x * board::size_y;
        actions.reserve(actionSize);
        for (int i = 0; i < actionSize; i++)
            actions.push_back(board::point(i));
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
        root = new Node(b, actions, engine);
        int result = simulate(root->position, false);
        update(root, result);
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
//            std::cout << root->childs[i]->visitCount << " ";
            if (bestCount < root->childs[i]->visitCount) {
                bestCount = root->childs[i]->visitCount;
                bestNode = root->childs[i];
            }
        }
//        std::cout << std::endl;
        return findActionByNextBoard(bestNode->position);
    }

private:  // After testing, it should be private
    int traverse(Node* node, bool isOpponent=false) {
        if (!node->legal.empty()) {
            Node* leaf = expand(node, isOpponent);
            int result = simulate(leaf->position, !isOpponent);
            update(leaf, result);
            update(node, result);
            return result;
        } else {
            int result;
            if (node->childs.empty()) {  //Terminal node
                result = simulate(node->position, isOpponent);
            } else {
                Node* nextNode = select(node, isOpponent);
                result = traverse(nextNode, !isOpponent);
            }
//            std::cout << nextNode->position << std::endl;
            update(node, result);
            return result;
        }
    }

    Node* select(Node* node, bool isOpponent) {
        float bestScore = 0;
        Node* nextNode = NULL;
        for (Node* child : node->childs) {
            float score = uct(*child, node->visitCount, isOpponent);
            if (bestScore < score) {
                bestScore = score;
                nextNode = child;
            }
        }
        if (nextNode == NULL) {
            std::cerr << "select error" << std::endl;
            exit(0);
        }
        return nextNode;
    }

    int simulate(const board& position, bool isOpponent) {
        board curPosition = position;
        std::vector<board::point> copyActions = actions;
        std::shuffle(actions.begin(), actions.end(), engine);
        for (size_t front = 0, back = copyActions.size(); back != 0;) {
            if (curPosition.place(copyActions[back]) == board::legal) {
                back--;
                front = 0;
                isOpponent = !isOpponent;
            } else if (front < back) {
                std::swap(copyActions[front++], copyActions[back]);
            } else {
                back = 0;
            }
        }
        return isOpponent;
    }

    Node* expand(Node* node, bool isOpponent) {
        board::point move = node->legal.back();
        node->legal.pop_back();
        board tem = node->position;
        tem.place(move);
        Node* leaf = new Node(tem, actions, engine);
        node->childs.push_back(leaf);
        return leaf;
    }

    void update(Node* node, int result) {
        node->visitCount++;
        node->wins += result;
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
        std::vector<board::point> copyActions = actions;
        for (board::point& move : copyActions) {
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
