//
// Created by pikachin on 2021/12/7.
//

#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include "board.h"
#include "action.h"
#include "agent.h"
#include "episode.h"
#include "statistic.h"
#include "mcts.h"

int main() {
    board testBoard;
    Mcts mcts(board::black);
    mcts.setupRoot(testBoard);
    mcts.traverse(mcts.root);
    std::cout << "first traverse complete" << std::endl;
    std::string tem;
    while (true) {
        getline(std::cin, tem);
        std::cout << "start traverse" << std::endl;
        mcts.traverse(mcts.root);
        for (int i = 0; i < mcts.root->childs.size(); i++)
            std::cout << mcts.root->childs[i]->visitCount << " ";
        std::cout << std::endl;
    }
}

