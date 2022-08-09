#include <Stepper.h>

enum class RotorState { UP, STOP, DOWN, OPEN, CLOSE };

class Curtain {
public:
    Curtain();
    const RotorState getRotorState() const { return this->rotorState; };
    void setRotorState(const RotorState state) { this->rotorState = state; };
private:
    RotorState rotorState;

    int YPosClosed;
    int currentYPos;
    bool configMode;

    Stepper* stepper;
};