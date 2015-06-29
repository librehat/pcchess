#ifndef POSITION_H
#define POSITION_H

#include <array>

class position
{
public:
    position(int _file = 0, int _rank = 0);
    int file;
    int rank;

    bool not_in_range(int min_file, int max_file, int min_rank, int max_rank) const;

    position& operator= (const position &b);
    position operator+ (const position &b);
    position operator- (const position &b);
    bool operator< (const position &b);
};

inline bool operator !=(const position &a, const position &b)
{
    return (a.file != b.file) || (a.rank != b.rank);
}

inline bool operator ==(const position &a, const position &b)
{
    return !(a != b);
}

typedef std::array<position, 2> pos_move;//moving a piece from move[0] to move[1]

#endif //POSITION_H
