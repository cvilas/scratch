// Experiments with FSM using modern C++.
// Based on https://www.cppstories.com/2023/finite-state-machines-variant-cpp/
// Compiler Explorer: https://godbolt.org/z/KEsxW99TP
//
// Some new ideas explored here:
// - States and Events are just structs enclosed in namespaces state and event, respectively. So what marks
//   them as states and events are the namespace they are in. Simple, generic, extendable.
// - First version of FSM using modern C++ techniques where the implementation is close to modification
//   but open to extension. Users can add events (see `ExtraEvent`, `UndefinedEvent`) and transitions
// - Transitions are implemented as free functions, without reference to the FSM object
// - Undefined events and transitions are handled properly
// - The FSM itself is extremely simple class. It just wraps an initialisation and a generic event handling method

#include <cstdlib>
#include <variant>
#include <iostream>
#include <format>

namespace state {
    struct PlayerAlive {
        unsigned int health_{ 0 };
        unsigned int remainingLives_{ 0 };
    };
    struct PlayerDead {
        unsigned int remainingLives_{ 0 };
    };
    struct GameOver { };
}

using HealthState = std::variant<state::PlayerAlive, state::PlayerDead, state::GameOver>;

namespace event {
    struct HitByMonster { unsigned int forcePoints_{ 0 }; };
    struct Heal { unsigned int points_{ 0 }; };
    struct Restart { unsigned int startHealth_{ 0 }; };
}

using PossibleEvent = std::variant<event::HitByMonster, event::Heal, event::Restart>;

HealthState onEvent(const state::PlayerAlive& alive, const event::HitByMonster& monster) {
    std::cout << std::format("PlayerAlive -> HitByMonster force {}\n", monster.forcePoints_);
    if (alive.health_ > monster.forcePoints_) {
        return state::PlayerAlive{ alive.health_ - monster.forcePoints_, alive.remainingLives_};
    }

    if (alive.remainingLives_ > 0)
        return state::PlayerDead{ alive.remainingLives_ - 1 };

    return state::GameOver{};
}

HealthState onEvent(state::PlayerAlive alive, const event::Heal& healingBonus) {
    std::cout << std::format("PlayerAlive -> Heal points {}\n", healingBonus.points_);
        alive.health_ += healingBonus.points_;
        return alive;
    }

HealthState onEvent(const state::PlayerDead& dead, const event::Restart& restart) {
    std::cout << std::format("PlayerDead -> restart\n");
    return state::PlayerAlive{ restart.startHealth_, dead.remainingLives_ };
}

HealthState onEvent(const state::GameOver& over, const event::Restart& restart) {
    std::cout << std::format("GameOver -> restart\n");
    std::cout << "Game Over, please restart the whole game!\n";
    return over;
}

HealthState onEvent(const auto&, const auto&) {
    throw std::logic_error{ "Unsupported state transition" };
}

class GameStateMachine {
public:
    void startGame(unsigned int health, unsigned int lives) {
        state_ = state::PlayerAlive{ health, lives };
    }

    void processEvent(const PossibleEvent& event) {
        state_ = std::visit(
            [](const auto& state, const auto& evt) {
                return onEvent(state, evt);
            },
        state_, event);
    }

protected:
    HealthState state_;
};

// Extending the system with extra user-defined events
namespace event {
    struct ExtraEvent { unsigned int forcePoints_{ 0 }; };
    struct UndefinedEvent {};
}

using ExtraPossibleEvent = std::variant<event::ExtraEvent, event::UndefinedEvent>;

HealthState onEvent(const auto& state, const event::ExtraEvent&) {
    std::cout << "Extra event. No change in state\n";
    return state;
}

// Extend the FSM class to handle extra user defined events
class MyGameFSM : public GameStateMachine {
public:
    void processEvent(const PossibleEvent& event) { return GameStateMachine::processEvent(event); }
    void processEvent(const ExtraPossibleEvent& event) {
        state_ = std::visit(
            [](const auto& state, const auto& evt) {
                return onEvent(state, evt);
            },
        state_, event);
    }

};

int main() {

    MyGameFSM game;
    game.startGame(100, 1);

try {
    game.processEvent(event::ExtraEvent {30});
    game.processEvent(event::HitByMonster {30});
    game.processEvent(event::HitByMonster {30});
    game.processEvent(event::HitByMonster {30});
    game.processEvent(event::HitByMonster {30});
    game.processEvent(event::Restart {100});
    game.processEvent(event::HitByMonster {60});
    game.processEvent(event::HitByMonster {50});
    game.processEvent(event::Restart {100});
    game.processEvent(event::UndefinedEvent{});
}
catch (std::exception& ex) {
    std::cout << "Exception! " << ex.what() << '\n';
}

    return EXIT_SUCCESS;
}
