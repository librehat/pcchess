#ifndef POS_MOVE_H
#define POS_MOVE_H

#include "position.h"
#include <boost/functional/hash.hpp>

class pos_move
{
public:
    explicit pos_move(const position &f = position(), const position &t = position());

    /*
     * You can also use the ICCS coordinate string to construct a pos_move
     * check http://www.xqbase.com/protocol/cchess_move.htm (Chinese)
     * there is no check for the string, please ensure it's a valid 4-char array
     */
    explicit pos_move(const std::string &iccs);

    std::string get_iccs() const;

    position from;
    position to;

    inline bool operator != (const pos_move &b) const {
        return from != b.from || to != b.to;
    }

    friend bool operator == (const pos_move &a, const pos_move &b) {
        return !(a != b);
    }

    friend std::ostream& operator<<(std::ostream &os, const pos_move &m)
    {
        return os << m.get_iccs();
    }

    friend std::size_t hash_value(const pos_move &m) {
        std::size_t seed = 0;
        boost::hash_combine(seed, m.from.file);
        boost::hash_combine(seed, m.from.rank);
        boost::hash_combine(seed, m.to.file);
        boost::hash_combine(seed, m.to.rank);
        return seed;
    }

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int)
    {
        ar & boost::serialization::make_nvp("from", from);
        ar & boost::serialization::make_nvp("to", to);
    }
};

namespace std
{
    template <>
    struct hash<pos_move>
    {
        std::size_t operator()(const pos_move& m) const
        {
            return hash_value(m);
        }
    };
}

BOOST_IS_MPI_DATATYPE(pos_move)

#endif //POS_MOVE_H
