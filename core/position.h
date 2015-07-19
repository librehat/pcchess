#ifndef POSITION_H
#define POSITION_H

#include <array>
#include <boost/serialization/access.hpp>
#include <boost/mpi/datatype.hpp>

class position
{
public:
    position(int _file = 0, int _rank = 0);
    position(const position &b);
    int file;
    int rank;

    bool not_in_range(int min_file, int max_file, int min_rank, int max_rank) const;

    position& operator= (const position &b);
    position operator+ (const position &b);
    position operator- (const position &b);
    bool operator< (const position &b) const;

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_NVP(file);
        ar & BOOST_SERIALIZATION_NVP(rank);
    }
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
template<class Archive>
void serialize(Archive &ar, pos_move &p, const unsigned int version)
{
    ar & BOOST_SERIALIZATION_NVP(p[0]);
    ar & BOOST_SERIALIZATION_NVP(p[1]);
}

BOOST_IS_MPI_DATATYPE(position)
BOOST_IS_MPI_DATATYPE(pos_move)

#endif //POSITION_H
