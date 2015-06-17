#ifndef POSITION_H
#define POSITION_H

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
};

inline bool operator< (const position &a, const position &b)
{
    if (a.file < b.file) {
        return true;
    } else if (a.file == b.file) {
        return a.rank < b.rank;
    } else {
        return false;
    }
}

inline bool operator!= (const position &a, const position &b)
{
    return (a.file != b.file) || (a.rank != b.rank);
}

inline bool operator== (const position &a, const position &b)
{
    return !(a != b);
}

#endif //POSITION_H
