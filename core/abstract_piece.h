/*
 * Abstract piece class
 *
 * This is the base class for all pieces
 * 
 * Copyright 2015 Symeon Huang <hzwhuang@gmail.com>
 */

#ifndef ABSTRACT_PIECE_H
#define ABSTRACT_PIECE_H

class abstract_piece
{
public:
    abstract_piece();
    abstract_piece(int _file, int _rank);
    virtual ~abstract_piece();

    void move_to_pos(int newfile, int newrank);
    virtual char abbr_name() const;
    virtual int value() const;//the "value" of this piece indicates how important it is
    
protected:
    //position data
    int file;
    int rank;
};

#endif //ABSTRACT_PIECE_H
