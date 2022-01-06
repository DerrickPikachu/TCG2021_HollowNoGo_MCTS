//
// Created by pikachin on 2021/12/7.
//

#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <iterator>
#include <string>
#include "board.h"
#include "action.h"
#include "agent.h"
#include "episode.h"
#include "statistic.h"
#include "mcts.h"

//void testTraverse() {
//    board testBoard;
//    Mcts mcts(board::black);
//    mcts.setupRoot(testBoard);
//    mcts.traverse(mcts.root);
//    std::cout << "first traverse complete" << std::endl;
//    std::string tem;
//    while (true) {
//        getline(std::cin, tem);
//        std::cout << "start traverse" << std::endl;
//        mcts.traverse(mcts.root);
//        for (int i = 0; i < (int)mcts.root->childs.size(); i++)
//            std::cout << mcts.root->childs[i]->visitCount << " ";
//        std::cout << std::endl;
//    }
//}
//
//void testMcts() {
//    board testBoard;
//    Mcts mcts(board::black);
//    std::string tem;
//    std::cout << "start routine" << std::endl;
//    while (true) {
//        mcts.setupRoot(testBoard);
//        mcts.search(3200);
//
//        for (int i = 0; i < (int)mcts.root->childs.size(); i++)
//            std::cout << mcts.root->childs[i]->visitCount << " ";
//        std::cout << std::endl;
//
//        action::place move = mcts.chooseAction();
//        board b = testBoard;
//        move.apply(b);
//        std::cout << b << std::endl;
//        mcts.resetMcts();
//        getline(std::cin, tem);
//    }
//}

void testRandom() {
    std::uniform_int_distribution<int> uniform(0, 10);
//    std::random_device dev;
    std::default_random_engine engine(time(0));
    for (int i = 0; i < 20; i++) {
        std::cout << uniform(engine) << " ";
    }
}

int main() {
    Mcts mcts(board::black);
    for (int i = 0; i < 2; i++) {
        board testBoard;
        bool result = mcts.simulate(testBoard, false);
        if (result)
            std::cout << "black win" << std::endl;
        else
            std::cout << "black lose" << std::endl;
    }
}

