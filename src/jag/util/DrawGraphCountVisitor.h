/*************** <auto-copyright.rb BEGIN do not edit this line> **************
 
 Copyright 2012-2014 by Ames Laboratory and Skew Matrix Software, LLC
 
 Permission is granted to anyone to use this software for any purpose,
 including commercial applications, and to alter it and redistribute it
 freely, subject to the following restrictions:
 
    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.
 
    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.
 
    3. This notice may not be removed or altered from any source
    distribution.
 
 *************** <auto-copyright.rb END do not edit this line> ***************/

#ifndef __JAG_UTIL_DRAW_GRAPH_COUNT_VISITOR_H__
#define __JAG_UTIL_DRAW_GRAPH_COUNT_VISITOR_H__ 1

#include <jag/util/Export.h>
#include <jag/draw/Visitor.h>
#include <jag/draw/DrawNodeContainer.h>
#include <jag/draw/DrawNode.h>
#include <jag/draw/CommandMap.h>

#include <iostream>


namespace jag {
namespace util {


class DrawGraphCountVisitor : public jag::draw::Visitor
{
public:
    DrawGraphCountVisitor();
    ~DrawGraphCountVisitor();

    void reset();
    void dump( std::ostream& ostr );

    virtual bool visit( jag::draw::DrawNodeContainer& nc );
    virtual bool visit( jag::draw::DrawNode& node, jag::draw::DrawNodeContainer& nc );

protected:
    unsigned int _containers;
    unsigned int _nodes;
    unsigned int _drawables;

    unsigned int _nonEmptyCommandMapDeltas;
    jag::draw::CommandMap _commands;
};

inline DrawGraphCountVisitor::DrawGraphCountVisitor()
    : jag::draw::Visitor( "count" )
{
    reset();
}
inline DrawGraphCountVisitor::~DrawGraphCountVisitor()
{
}

inline void DrawGraphCountVisitor::reset()
{
    _containers = 0;
    _nodes = 0;
    _drawables = 0;
    _nonEmptyCommandMapDeltas = 0;
}
inline void DrawGraphCountVisitor::dump( std::ostream& ostr )
{
    ostr << "            \tTotal" << std::endl;
    ostr << "            \t-----" << std::endl;
    ostr << " Containers:\t" << _containers << std::endl;
    ostr << "      Nodes:\t" << _nodes << std::endl;
    ostr << "  Drawables:\t" << _drawables << std::endl;
    ostr << "  CM deltas:\t" << _nonEmptyCommandMapDeltas << std::endl;
}

inline bool DrawGraphCountVisitor::visit( jag::draw::DrawNodeContainer& nc )
{
    ++_containers;
    return( true );
}
inline bool DrawGraphCountVisitor::visit( jag::draw::DrawNode& node, jag::draw::DrawNodeContainer& nc )
{
    ++_nodes;
    _drawables += node.getNumDrawables();

    if( node.getCommandMap() != NULL )
    {
        jag::draw::CommandMap delta( _commands << *( node.getCommandMap() ) );
        if( !( delta.empty() ) )
            ++_nonEmptyCommandMapDeltas;
    }

    return( true );
}


// namespace jag::util::
}
}


// __JAG_UTIL_DRAW_GRAPH_COUNT_VISITOR_H__
#endif
