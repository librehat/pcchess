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
    bool operator< (const position &b);
    bool operator!= (const position &b);
    bool operator== (const position &b);
};

#endif //POSITION_H
