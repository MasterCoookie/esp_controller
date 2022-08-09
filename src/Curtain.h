enum class RotorState { UP, STOP, DOWN, OPEN, CLOSE };

class Curtain {

    RotorState rotorState;

    int YPosClosed;
    int currentYPos;
    bool configMode;
};