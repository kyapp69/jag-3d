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

#ifndef __JAGDRAW_COMMAND_NODE_H__
#define __JAGDRAW_COMMAND_NODE_H__ 1

#include <jagDraw/Export.h>
#include <jagDraw/CommandNodePtr.h>
#include <jagDraw/CommandMap.h>
#include <jagBase/LogBase.h>
#include <jagBase/UserDataOwner.h>

#include <map>


namespace jagDraw {


/** \addtogroup jagDrawCommandGraph Command Graph Data Structure */
/*@{*/

/** \class CommandNode CommandNode.h <jagDraw/CommandNode.h>
\brief The command graph node element.
\details The command graph is used to cache accumulations of CommandMaps.

The jagSG::CollectionVisitor manages a command graph to avoid redundant
accumulation of static CommandMap objects. Accumulations are only performed
when a new CommandMap is encountered by the CollectionVisitor, or when
a given CommandMap is marked as dirty for a given command graph node.

\logname jag.draw.cg.node */
class JAGDRAW_EXPORT CommandNode : protected jagBase::LogBase, public jagBase::UserDataOwner
{
public:
    CommandNode( const std::string& logName );
    CommandNode( CommandNode* parent, const std::string& logName=std::string( "" ) );
    CommandNode( const CommandNode& rhs );
    ~CommandNode();

    CommandNode* findOrCreateChild( CommandMapPtr commands );

    void accumulate( const CommandMapPtr commands );

protected:
    CommandNode* _parent;

    typedef std::map< CommandMap*, CommandNodePtr > ChildMap;
    ChildMap _children;

    CommandMap _accumulation;
};


/*@}*/


// jagDraw
}


// __JAGDRAW_COMMAND_NODE_H__
#endif
