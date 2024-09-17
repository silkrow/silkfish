#pragma once
#include "chess.hpp"
#include <thread>
#include <future>
#include <queue>
#include <algorithm>

class LennyPOOL {
private:
    std::vector<std::thread> threads;                      // Vector to hold threads
    std::queue<std::function<void()>> tasks;               // Queue to store tasks
    std::mutex queue_mutex;                                // Mutex for synchronizing task queue
    std::condition_variable condition;                     // Condition variable to signal threads
    std::atomic<bool> stop;                                // Flag to stop threads
    std::atomic<int> active_tasks{0};                      // Tracks the number of active tasks
    int max_threads;                                       // Maximum number of threads

public:
    // Constructor
    LennyPOOL(int max_threads);

    // Template function to enqueue tasks
    template<class F>
    void run(F&& f);

    // Waits for all tasks to finish
    void wait_all();

    // Destructor
    ~LennyPOOL();
};
std::pair<int, std::string> quiescence_search(int q_depth, int alpha, int beta, chess::Color color, chess::Board board);
std::pair<int, std::string> minimax(int mm_depth, int alpha, int beta, chess::Color color, chess::Board board);
chess::Move findBestMove(chess::Board& board, int depth, int max_threads);