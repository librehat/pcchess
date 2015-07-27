#ifndef POSITION_H
#define POSITION_H

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/mpi/datatype.hpp>
#include <cstdint>

class position
{
public:
    explicit position(std::int8_t _file = 0, std::int8_t _rank = 0);
    position(const position &b);
    std::int8_t file;
    std::int8_t rank;

    bool not_in_range(std::int8_t min_file, std::int8_t max_file, std::int8_t min_rank, std::int8_t max_rank) const;

    position& operator= (const position &b);
    position operator+ (const position &b);
    position operator- (const position &b);
    bool operator< (const position &b) const;

    friend std::ostream& operator<<(std::ostream &os, const position &p)
    {
        return os << static_cast<char>(p.file + 'a') << static_cast<char>(p.rank + '0');
    }

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
