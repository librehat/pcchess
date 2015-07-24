#ifndef SERIALIZATION_EXPORT_H
#define SERIALIZATION_EXPORT_H

#include "../core/advisor.h"
#include "../core/cannon.h"
#include "../core/elephant.h"
#include "../core/king.h"
#include "../core/horse.h"
#include "../core/chariot.h"
#include "../core/pawn.h"
#include "threaded_node.h"
#include "random_player.h"
#include "uct_player.h"
#include "threaded_uct_player.h"

#include <boost/serialization/export.hpp>

BOOST_CLASS_EXPORT_GUID(advisor, "advisor")
BOOST_CLASS_EXPORT_GUID(cannon, "cannon")
BOOST_CLASS_EXPORT_GUID(elephant, "elephant")
BOOST_CLASS_EXPORT_GUID(king, "king")
BOOST_CLASS_EXPORT_GUID(horse, "horse")
BOOST_CLASS_EXPORT_GUID(chariot, "chariot")
BOOST_CLASS_EXPORT_GUID(pawn, "pawn")
BOOST_CLASS_EXPORT_GUID(threaded_node, "threaded_node")
BOOST_CLASS_EXPORT_GUID(random_player, "random_player")
BOOST_CLASS_EXPORT_GUID(uct_player, "uct_player")
BOOST_CLASS_EXPORT_GUID(threaded_uct_player, "threaded_uct_player")

#endif //SERIALIZATION_EXPORT_H