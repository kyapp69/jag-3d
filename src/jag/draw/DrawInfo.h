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

#ifndef __JAG_DRAW_DRAW_INFO_H__
#define __JAG_DRAW_DRAW_INFO_H__ 1

#include <jag/draw/Export.h>
#include <jag/draw/PlatformOpenGL.h>
#include <jag/draw/ContextSupport.h>
#include <jag/draw/CommandMap.h>
#include <jag/draw/PerContextData.h>

#include <jag/draw/Uniform.h>
#include <jag/draw/UniformBlock.h>
#include <gmtl/gmtl.h>


namespace jag {
namespace draw {


/** \addtogroup jagDrawDrawGraph Draw Graph Data Structure */
/*@{*/

/** \class DrawInfo DrawInfo.h <jag/draw/DrawInfo.h>
\brief Draw-traversal control information.
\details Contains information used to execute and optimize the draw traversal.
*/
struct JAGDRAW_EXPORT DrawInfo
{
    DrawInfo();

    void startFrame( const bool startFrame=true );


    /** \brief Set to true just before the render traversal. */
    bool _startFrame;

    /** Jag context identifier. */
    jagDrawContextID _id;

    /** The current CommandMap. As each jag::draw::DrawNode is executed during
    the draw traversal, \c _current is updated to reflect the CommandMap in
    effect. \c _current is used to access the current jag::draw::Program to
    bind new Uniform and UniformBlock objects, amoung other things. */
    CommandMap _current;

    /** \brief Support for external OpenGL programs.
    \details To use an application-bound program with JAG rendering, do not
    specify a Program object in the _current CommandMap. Instead, specify
    the OpenGL program id here. */
    GLuint _externalProgramID;

    /** \brief Buffer object ID most recently bound to GL_ELEMENT_ARRAY_BUFFER.
    \details Facilitates element buffer sharing and eliminates redundant
    element buffer binding by tracking the most recently bound element buffer
    object. */
    GLuint _elementBufferID;

    /** \brief Track the most recent jag::draw::DrawNode matrix. */
    gmtl::Matrix44d _transform;


    typedef enum {
        DRAW_BOUND = ( 0x1 << 0 ), //< Draw bound containing all Drawables within a draw node.
    } ControlFlags;
    /** \brief Specify draw traversal control flags.
    \details Bitwise-OR enums from ControlFlags to alter behavior of the
    draw traversal. Currently, the jag::draw::DrawNode::execute() method respects
    one enum, DRAW_BOUND. */
    unsigned int _controlFlags;


    typedef std::map< Program::HashValue, UniformPtr > UniformMap;

    /** \brief List of active uniforms during draw.
    \details When a new Program is used, it iterates over its
    active uniforms and attempts to find a hash key match in this map.
    If a match is found, the corresponding uniform value is specified.
    */
    UniformMap _uniformMap;

    /**\brief Returns an existi9ng, or creates a new uniform in _uniformMap.
    \details TBD
    */
    UniformPtr& getOrCreateUniform( const std::string& name )
    {
        const Program::HashValue hashVal( Program::createHash( name ) );
        UniformMap::iterator it( _uniformMap.find( hashVal ) );
        if( it == _uniformMap.end() )
        {
            UniformPtr uniform( UniformPtr( new Uniform( name ) ) );
            _uniformMap[ hashVal ] = uniform;
            it = _uniformMap.find( hashVal );
        }
        return( it->second );
    }


    typedef std::map< Program::HashValue, ConstUniformBlockPtr > UniformBlockMap;

    /** \brief List of active uniform blocks during draw.
    \details When a new Program is used, it iterates over its
    active uniform blocks and attempts to find a hash key match in this map.
    If a match is found, it binds the corresponding uniform block.
    */
    UniformBlockMap _uniformBlockMap;
};

typedef PerContextData< DrawInfo > PerContextDrawInfo;


/*@}*/


// namespace jag::draw::
}
}


// __JAG_DRAW_DRAW_INFO_H__
#endif
