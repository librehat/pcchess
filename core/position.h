#ifndef POSITION_H
#define POSITION_H

#include <array>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/mpi/datatype.hpp>

class position
{
public:
    explicit position(int _file = 0, int _rank = 0);
    explicit position(char _file, int _rank = 0);
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
    void serialize(Archive &ar, const unsigned int)
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

BOOST_IS_MPI_DATATYPE(position)

#endif //POSITION_H
