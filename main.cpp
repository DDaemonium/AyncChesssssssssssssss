// Chesssssssssss.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <atomic>
#include <ctime>
#include <vector>
#include <thread>
#include <chrono>
#include <algorithm> // for copy
#include <iterator> // for ostream_iterator

#define PLAYERS_CNT 6
#define SLEEP_TIME 300
#define WAITING_SLEEP_TIME 5000
#define MAX_STEPS 50

struct Field;
class Rook;

class Rook final
{
private:
    struct Coords final
    {
        int X;
        int Y;
        friend std::ostream& operator << (std::ostream & out, const Coords & coords)
        {
            return out << "(" << coords.X << " : " << coords.Y << ")";
        }
    };
    enum Direction
    {
        Top = 0,
        Bottom = 1,
        Left = 2,
        Right = 3
    };
    std::vector<Coords> history;

    Coords GetNextPosition() const;
public:
    void PrintHistory() const;
    void Run(Field& field);
    int CurrentX{};
    int CurrentY{};
};

// Why is it structure?
// IDK
struct Field final
{
    std::atomic<Rook*> Field[8][8];
};

using Players = Rook[PLAYERS_CNT];

Players players{};
Field field{};

int GetXY() { return rand() % 8; }

void FillInitialPOsition(Field& field)
{
    for (auto& player : players)
    {
        int x = GetXY();
        int y = GetXY();
        while (field.Field[x][y])
        {
            x = GetXY();
            y = GetXY();
        }
        field.Field[x][y] = &player;
        player.CurrentX = x;
        player.CurrentY = y;
    }
}

void FuckingHellMagicWillDestroyThisWorld(Field& field)
{
    std::vector<std::thread> threadPool;
    // Optimisations!!!!!!!!!!!!!
    threadPool.reserve(PLAYERS_CNT);
    for (auto& player : players)
    {
        threadPool.push_back(std::thread([&] { player.Run(field); }));
    }
    // Let's wait this guys, they will help us later, maybe....
    for (auto& thread : threadPool)
    {
        thread.join();
    }
}

void IfWorldStillAliveLetsPrintHistory()
{
    for (const auto& player : players)
    {
        player.PrintHistory();
        std::cout << "\n";
    }
}

int main()
{
    srand(time(0));
    Field field;
    FillInitialPOsition(field);
    // Hmm, this function looks safe enough
    // Let's call it
    FuckingHellMagicWillDestroyThisWorld(field);

    // Why not? Are you afraid?
    IfWorldStillAliveLetsPrintHistory();
    /*
        Eisbrecher - Bombe 
        https://music.yandex.ru/album/262208/track/2569515
    */
}

inline Rook::Coords Rook::GetNextPosition() const
{
    while (true) {
        Direction direction = Direction(rand() % 4);
        if (direction == Direction::Top && CurrentY + 1 < 8)
        {
            return { CurrentX, CurrentY + 1 };
        }
        else if (direction == Direction::Bottom && CurrentY - 1 >= 0)
        {
            return { CurrentX, CurrentY - 1 };
        }
        if (direction == Direction::Right && CurrentX + 1 < 8)
        {
            return { CurrentX + 1, CurrentY };
        }
        else if (direction == Direction::Left && CurrentX - 1 >= 0)
        {
            return { CurrentX - 1, CurrentY };
        }
    }
}

inline void Rook::PrintHistory() const
{
    // Too lazy ass to write loops
    std::copy(history.begin(), history.end(), std::ostream_iterator<Coords>(std::cout, " "));
}

void Rook::Run(Field& field)
{
    for (size_t steps = 0; steps < MAX_STEPS; ++steps)
    {
        // Let's try to get new direction
        auto nextPosition = GetNextPosition();
        // Timer? Why not.
        auto start = std::chrono::high_resolution_clock::now();
        // Wow!! Looks like spinlock, let me think
        // Hm...
        // Yep, this is fucking spinlock
        Rook* unlatched = nullptr;
        while (!field.Field[nextPosition.X][nextPosition.Y].compare_exchange_weak(unlatched, this, std::memory_order_acquire)) {
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed = end - start;
            // Looks like this thread is looser
            if (elapsed.count() >= WAITING_SLEEP_TIME)
            {
                // Fuck off from this way!!!!
                nextPosition = GetNextPosition();
            }
            unlatched = nullptr;
        }

        // Hmmm, now I have two fields, cool!
        // ') But I no need so much, will free last position
        unlatched = this;
        // What the hell, again spinlock? For what?
        // Fuck you CPU Core cache!!!!
        while (!field.Field[CurrentX][CurrentY].compare_exchange_weak(unlatched, nullptr, std::memory_order_acquire)) {
            unlatched = this;
        }

        // Wow, MOM!! I'll be in history
        // Maybe they'll show me in TV
        history.push_back({CurrentX, CurrentY});

        CurrentX = nextPosition.X;
        CurrentY = nextPosition.Y;

        // I'm tired. Let's say "by by" to our thread.
        // It will sleep for 300ms
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));
    }
}
