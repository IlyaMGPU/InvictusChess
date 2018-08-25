/**************************************************/
/*  Invictus 2018						          */
/*  Edsel Apostol                                 */
/*  ed_apostol@yahoo.com                          */
/**************************************************/
#pragma once

#include <functional>
#include <thread>
#include <unordered_map>
#include "typedefs.h"
#include "trans.h"
#include "utils.h"
#include "log.h"
#include "eval.h"
#include "search.h"

struct uci_limits_t {
	void init() {
		wtime = 0;
		btime = 0;
		winc = 0;
		binc = 0;
		movestogo = 0;
		depth = 0;
		movetime = 0;
		mate = 0;
		infinite = false;
		ponder = false;
		nodes = 0;
	};
	int wtime;
	int btime;
	int winc;
	int binc;
	int movestogo;
	int depth;
	int movetime;
	int mate;
	bool infinite;
	bool ponder;
	uint64_t nodes;
};

struct uci_options_t {
	using callbackfunc = std::function<void()>;
	uci_options_t() {}
	uci_options_t(std::string v, callbackfunc f) : type("string"), min(0), max(0), onChange(f) {
		defaultval = currval = v;
	}
	uci_options_t(bool v, callbackfunc f) : type("check"), min(0), max(0), onChange(f) {
		defaultval = currval = (v ? "true" : "false");
	}
	uci_options_t(callbackfunc f) : type("button"), min(0), max(0), onChange(f) {
	}
	uci_options_t(int v, int minv, int maxv, callbackfunc f) : type("spin"), min(minv), max(maxv), onChange(f) {
		defaultval = currval = std::to_string(v);
	}
	int getIntVal() const {
		return (type == "spin" ? atoi(currval.c_str()) : currval == "true");
	}
	std::string getStrVal() {
		return currval;
	}
	uci_options_t& operator=(const std::string& val) {
		if (type != "button") currval = val;
		onChange();
		return *this;
	}
	std::string defaultval, currval, type;
	int min, max;
	callbackfunc onChange;
};

struct uci_options_map : public std::unordered_map<std::string, uci_options_t> {
	void print() const {
		for (auto itr = begin(); itr != end(); ++itr) {
			const uci_options_t& opt = itr->second;
			LogAndPrintOutput log;
			log << "option name " << itr->first << " type " << opt.type;
			if (opt.type != "button") log << " default " << opt.defaultval;
			if (opt.type == "spin") log << " min " << opt.min << " max " << opt.max;
		}
	}
};

struct engine_t : public std::vector<search_t*> {
	static const int CS_SIZE = 32768;
	static const int CS_WAYS = 4;
	static const int DEFER_DEPTH = 3;
	static const int CUTOFF_CHECK_DEPTH = 3;

	engine_t();
	~engine_t();
	void initSearch();
	void newgame();
	void stopthreads();
	void initUCIoptions();
	void printUCIoptions();
	void waitForThreads();
	void ponderhit();

	void onHashChange();
	void onThreadsChange();
	void onDummyChange();

	uint64_t nodesearched();
	void stopIterations();

	bool defer_move(uint64_t move_hash, int depth);
	void starting_search(uint64_t move_hash, int depth);
	void finished_search(uint64_t move_hash, int depth);

	pvhash_table_t pvt;
	trans_table_t tt;
	uci_options_map options;
	uci_limits_t limits;
	position_t origpos;
	move_t rootbestmove;
	move_t rootponder;
	int rootbestdepth;
	spinlock_t updatelock;

	std::atomic<int> currently_searching[CS_SIZE][CS_WAYS];
	std::atomic<bool> use_time;
	std::atomic<bool> stop;
	bool doSMP;

	int64_t start_time;
	int64_t time_limit_max;
	int64_t time_limit_abs;
};