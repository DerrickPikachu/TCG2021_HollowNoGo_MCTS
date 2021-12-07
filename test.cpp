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
    std::cout << mcts.simulate(testBoard, false) << std::endl;
}

