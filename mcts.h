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
    Mcts() : blackSpace(board::size_x * board::size_y),
                                   whiteSpace(board::size_x * board::size_y) {
        srand(time(NULL));
        engine.seed(rand() % 100000);
        for (int i = 0; i < (int)blackSpace.size(); i++)
            blackSpace[i] = action::place(i, board::black);
        for (int i = 0; i < (int)whiteSpace.size(); i++)
            whiteSpace[i] = action::place(i, board::white);
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

    action::place chooseAction() {
        if (root->childs.empty())
            return action::place(0, who);
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
        std::vector<action::place> curSpace = (isBlackTurn(isOpponent))? blackSpace : whiteSpace;
        std::vector<action::place> oppSpace = (isBlackTurn(isOpponent))? whiteSpace : blackSpace;
        std::shuffle(curSpace.begin(), curSpace.end(), engine);
        std::shuffle(oppSpace.begin(), oppSpace.end(), engine);
        bool isCurTurn = true;
        for (;; isCurTurn = !isCurTurn, isOpponent = !isOpponent) {
            std::vector<action::place>& temSpace = (isCurTurn)? curSpace : oppSpace;
            int i = 0;
            for (; i < (int)temSpace.size(); i++) {
                board temPosition = curPosition;
                if (temSpace[i].apply(temPosition) == board::legal)
                    break;
            }
            if (i == (int)temSpace.size()) {
                break;
            } else {
                temSpace[i].apply(curPosition);
            }
        }
        return isOpponent;
    }

    void expand(Node* node, bool isOpponent) {
        std::vector<Node*> childs;
        std::vector<action::place>& nextSpace = (isBlackTurn(isOpponent)) ? blackSpace : whiteSpace;
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

    float uct(Node& node, int parentVisitCount, bool isOpponent) {
        float c = 1.5;
        float winRate = (float)node.wins / (float)(node.visitCount + 1);
        float exploitation = (isOpponent && uctType == "anti")? 1 - winRate : winRate;
        float exploration = sqrt(log(parentVisitCount) / (float)(node.visitCount + 1));
        return exploitation + c * exploration;
    }

    action::place findActionByNextBoard(const board& nextBoard) {
        std::vector<action::place>& temSpace = (who == board::black)? blackSpace : whiteSpace;
        for (action::place& move : temSpace) {
            board position = root->position;
            if (move.apply(position) == board::legal && position == nextBoard)
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
    std::vector<action::place> blackSpace;
    std::vector<action::place> whiteSpace;
    board::piece_type who;
    std::default_random_engine engine;
    std::string uctType;
};


#endif //TCG_NOGO_MCTS_MCTS_H
