//
// Created by pikachin on 2021/12/6.
//

#ifndef TCG_NOGO_MCTS_MCTS_H
#define TCG_NOGO_MCTS_MCTS_H

#include <unordered_map>
#include <stdlib.h>
#include <iostream>
#include <ctime>
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
    Mcts() : root(nullptr), exploreC(0),
             blackSpace(board::size_x * board::size_y),
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

    void setupRoot(const board& b) {
        if (root == nullptr) {
            root = new Node(b);
        } else {
            Node* target = nullptr;
            for (int i = 0; i < (int)root->childs.size(); i++) {
                if (root->childs[i]->position == b)
                    target = root->childs[i];
            }
            if (target == nullptr)
                std::cerr << "setupRoot error" << std::endl;
            removeNonsenseNode(target);
        }
    }

    void resetMcts(Node* node=nullptr) {
        if (root == nullptr) return;
        if (node == nullptr) {
            node = root;
            root = nullptr;
        }
        for (int i = 0; i < (int)node->childs.size(); i++)
            resetMcts(node->childs[i]);
        delete node;
    }

    void search(int timesOfMcts, float constant) {
        exploreC = constant;
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
        action::place move = findActionByNextBoard(bestNode->position);
        removeNonsenseNode(bestNode);
        return move;
    }

private:  // After testing, it should be private
    int traverse(Node* node, bool isOpponent=false) {
        if (node->childs.empty()) {  // expand and simulate
            int result = simulate(node->position, isOpponent);
            expand(node, isOpponent);
            update(node, result);
            return result;
        } else {
            Node* nextNode = select(node);
//            std::cout << nextNode->position << std::endl;
            int result = traverse(nextNode, !isOpponent);
            update(node, result);
            return result;
        }
    }

    Node* select(Node* node) {
        float bestScore = 0;
        std::vector<Node*> nextNodes;
        for (Node* child : node->childs) {
            float score = uct(*child, node->visitCount);
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
        if (node.visitCount == 0) return 10000.0;
        float exploitation = (float)node.wins / (float)(node.visitCount + 1);
        float exploration = sqrt(log(parentVisitCount) / (float)(node.visitCount + 1));
        return exploitation + exploreC * exploration;
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

    void removeNonsenseNode(Node* except) {
        for (int i = 0; i < (int)root->childs.size(); i++) {
            if (root->childs[i] != except)
                resetMcts(root->childs[i]);
        }
        delete root;
        root = except;
    }

    std::string appendPath(std::string path, const action::place& move) {
        std::string moveCode = std::to_string(move.position().x) + std::to_string(move.position().y);
        return path + "_" + moveCode;
    }

private:
    Node* root;
    float exploreC;
    std::vector<action::place> blackSpace;
    std::vector<action::place> whiteSpace;
    board::piece_type who;
    std::default_random_engine engine;
};

#endif //TCG_NOGO_MCTS_MCTS_H
