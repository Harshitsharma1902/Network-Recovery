#pragma once

struct Connection {
    int u = 0;
    int v = 0;
    int weight = 1;
    bool failed = false;

    bool operator==(const Connection& other) const {
        return ((u == other.u && v == other.v) || (u == other.v && v == other.u)) &&
               weight == other.weight && failed == other.failed;
    }
};
