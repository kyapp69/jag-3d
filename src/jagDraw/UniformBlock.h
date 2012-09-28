/*************** <auto-copyright.pl BEGIN do not edit this line> **************
*
* jag3d is (C) Copyright 2011-2012 by Kenneth Mark Bryden and Paul Martz
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License version 2.1 as published by the Free Software Foundation.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the
* Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
*
*************** <auto-copyright.pl END do not edit this line> ***************/

#ifndef __JAGDRAW_UNIFORM_BLOCK_H__
#define __JAGDRAW_UNIFORM_BLOCK_H__ 1


#include <jagDraw/Export.h>
#include <jagDraw/Program.h>
#include <jagDraw/Uniform.h>
#include <jagDraw/BufferObject.h>
#include <jagDraw/DrawablePrep.h>
#include <jagBase/LogBase.h>
#include <jagDraw/ObjectID.h>
#include <jagDraw/Shader.h>
#include <jagDraw/PerContextData.h>
#include <jagBase/ptr.h>

#include <map>
#include <vector>


namespace jagDraw {


struct DrawInfo;


/** \class UniformBlock UniformBlock.h <jagDraw/UniformBlock.h>
\brief Support for GLSL interface blocks / named uniform blocks.
\details

Future enhancements:

Requires efficiency enhancements. If the attached Uniform values
have not changed, there is no need to copy their values into the
buffer. But when the values have changes, UniformBlock should
map the buffer and modify the values.

\gl{section 2.9} \gl{section 2.11.4}.
*/
class JAGDRAW_EXPORT UniformBlock : public DrawablePrep,
            public ObjectIDOwner, public SHARED_FROM_THIS(UniformBlock),
            protected jagBase::LogBase
{
public:
    UniformBlock( const std::string& name );
    UniformBlock( const UniformBlock& rhs );
    virtual ~UniformBlock();

    void addUniform( UniformPtr uniform );

    /** \brief TBD
    \details TBD
    Does not add this uniform block to drawInfo._uniformBlockMap. */
    void operator()( DrawInfo& drawInfo, const Program::BlockInfo& blockInfo ) const;
    /** \brief TBD
    \details Override methods from DrawablePrep.
    Adds this uniform to drawInfo._uniformBlockMap. */
    virtual void operator()( DrawInfo& drawInfo );

    /** Override methods from ObjectIDOwner */
    virtual void setMaxContexts( const unsigned int numContexts );
    virtual void deleteID( const jagDraw::jagDrawContextID contextID );

protected:
    std::string _name;
    Program::HashValue _nameHash;

    UniformVec _uniforms;

    UniformBufferPtr _bufferObject;
    jagBase::BufferPtr _buffer;
};

typedef jagBase::ptr< jagDraw::UniformBlock >::shared_ptr UniformBlockPtr;
typedef jagBase::ptr< const jagDraw::UniformBlock >::shared_ptr ConstUniformBlockPtr;
typedef std::vector< UniformBlockPtr > UniformBlockVec;


// jagDraw
}


// __JAGDRAW_UNIFORM_BLOCK_H__
#endif