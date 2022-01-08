/**
 * Framework for NoGo and similar games (C++ 11)
 * agent.h: Define the behavior of variants of the player
 *
 * Author: Theory of Computer Games (TCG 2021)
 *         Computer Games and Intelligence (CGI) Lab, NYCU, Taiwan
 *         https://cgilab.nctu.edu.tw/
 */

#pragma once
#include <string>
#include <random>
#include <sstream>
#include <map>
#include <type_traits>
#include <algorithm>
#include "board.h"
#include "action.h"
#include "mcts.h"
#include <fstream>
#include <thread>
#include <future>

class agent {
public:
	agent(const std::string& args = "") {
		std::stringstream ss("name=unknown role=unknown " + args);
		for (std::string pair; ss >> pair; ) {
			std::string key = pair.substr(0, pair.find('='));
			std::string value = pair.substr(pair.find('=') + 1);
			meta[key] = { value };
		}
	}
	virtual ~agent() {}
	virtual void open_episode(const std::string& flag = "") {}
	virtual void close_episode(const std::string& flag = "") {}
	virtual action take_action(const board& b) { return action(); }
	virtual bool check_for_win(const board& b) { return false; }

public:
	virtual std::string property(const std::string& key) const { return meta.at(key); }
	virtual void notify(const std::string& msg) { meta[msg.substr(0, msg.find('='))] = { msg.substr(msg.find('=') + 1) }; }
	virtual std::string name() const { return property("name"); }
	virtual std::string role() const { return property("role"); }

protected:
	typedef std::string key;
	struct value {
		std::string value;
		operator std::string() const { return value; }
		template<typename numeric, typename = typename std::enable_if<std::is_arithmetic<numeric>::value, numeric>::type>
		operator numeric() const { return numeric(std::stod(value)); }
	};
	std::map<key, value> meta;
};

/**
 * base agent for agents with randomness
 */
class random_agent : public agent {
public:
	random_agent(const std::string& args = "") : agent(args) {
		if (meta.find("seed") != meta.end())
			engine.seed(int(meta["seed"]));
	}
	virtual ~random_agent() {}

protected:
	std::default_random_engine engine;
};

/**
 * random player for both side
 * put a legal piece randomly
 */
class player : public random_agent {
public:
	player(const std::string& args = "") : random_agent("name=random role=unknown " + args),
		space(board::size_x * board::size_y), who(board::empty) {
		if (name().find_first_of("[]():; ") != std::string::npos)
			throw std::invalid_argument("invalid name: " + name());
		if (role() == "black") who = board::black;
		if (role() == "white") who = board::white;
		if (who == board::empty)
			throw std::invalid_argument("invalid role: " + role());
		for (size_t i = 0; i < space.size(); i++)
			space[i] = action::place(i, who);
//		mcts.setWho(who);
//		mcts.setUctType(meta["uct"]);
	}

	virtual action take_action(const board& state) {
		if (std::string(meta["name"]) == "mcts") {
		    return mctsAction(state);
		} else {
		    return randomAction(state);
		}
	}

	action randomAction(const board& state) {
        std::shuffle(space.begin(), space.end(), engine);
        for (const action::place& move : space) {
            board after = state;
            if (move.apply(after) == board::legal)
                return move;
        }
        return action();
	}

	action mctsAction(const board& state) {
        int parallel = int(meta["parallel"]);
        int actionSize = board::size_x * board::size_y;
        std::vector<std::vector<int>> simulationCount(parallel, std::vector<int>(actionSize));
        std::vector<std::thread> threads;
        for (int i = 0; i < parallel; i++) {
            threads.push_back(std::thread(&player::runMcts, this, state, &simulationCount[i]));
        }
        for (int i = 0; i < parallel; i++) {
            threads[i].join();
        }
        int bestCount = 0;
        int bestMoveIndex = 0;
//        int verify = 0;
        for (int i = 0; i < actionSize; i++) {
            int total = 0;
            for (int j = 0; j < parallel; j++)
                total += simulationCount[j][i];
//            std::cerr << total << std::endl;
//            verify += total;
            if (bestCount < total) {
                bestCount = total;
                bestMoveIndex = i;
            }
        }
        threads.clear();
        simulationCount.clear();
//        std::cout << "verify: " << verify << std::endl;
        return action::place(board::point(bestMoveIndex), who);
//	    mcts.setupRoot(state);
//	    mcts.search(int(meta["simulation"]), float(meta["explore"]));
//        board::point move = mcts.chooseAction();
//	    mcts.resetMcts();
//	    return action::place(move, who);
	}

    void runMcts(board state, std::vector<int>* counter) {
        Mcts mcts;
        mcts.setWho(who);
        mcts.setUctType(meta["uct"]);
        mcts.setupRoot(state);
        mcts.search(int(meta["simulation"]), float(meta["explore"]));
        for (int i = 0; i < (int)counter->size(); i++)
            (*counter)[i] = mcts.getSimulationCount(i);
        mcts.resetMcts();
    }

private:
	std::vector<action::place> space;
	board::piece_type who;
};
