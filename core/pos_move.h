#include "position.h"

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

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int)
    {
        ar & boost::serialization::make_nvp("from", from);
        ar & boost::serialization::make_nvp("to", to);
    }
};

inline bool operator == (const pos_move &a, const pos_move &b) {
    return !(a != b);
}

BOOST_IS_MPI_DATATYPE(pos_move)
