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

#include <jagDraw/PlatformOpenGL.h>
#include <jagDraw/Drawable.h>
#include <jagDraw/Program.h>
#include <jagDraw/VertexArrayObject.h>
#include <jagDraw/BufferObject.h>
#include <jagDraw/DrawCommand.h>
#include <jagDraw/VertexAttribContainer.h>
#include <jagDraw/DrawInfo.h>
#include <jagDraw/Error.h>
#include <jagBase/LogMacros.h>
#include <jagBase/Profile.h>

#include <boost/foreach.hpp>

#include <sstream>


namespace jagDraw {


Drawable::Drawable( const std::string& logName )
  : jagBase::LogBase( logName.empty() ? "jag.draw.drawable" : logName ),
    ObjectIDOwner(),
    BoundOwner(),
    jagBase::Notifier()
{
}
Drawable::Drawable( const Drawable& rhs )
  : jagBase::LogBase( rhs ),
    ObjectIDOwner( rhs ),
    BoundOwner( rhs ),
    jagBase::Notifier( rhs ),
    _drawCommands( rhs._drawCommands )
{
}
Drawable::~Drawable()
{
}


void Drawable::execute( DrawInfo& drawInfo )
{
    JAG3D_TRACE( "Drawable::execute" );

    const size_t sz( _drawCommands.size() );
    if( sz == 0 )
        return;

    for( size_t idx = 0; idx < sz; ++idx )
        _drawCommands[ idx ]->execute( drawInfo );

    JAG3D_ERROR_CHECK( "Drawable::execute()" );
}


BoundPtr Drawable::newBound()
{
    return( BoundPtr( new BoundAABox() ) );
}

void Drawable::computeBound( BoundPtr& bound, const jagDraw::CommandMap& commands, BoundOwner* owner )
{
    JAG3D_PROFILE( "Drawable::computeBound" );

    const jagDraw::DrawablePrepPtr& drawablePrep( commands[ jagDraw::DrawablePrep::VertexArrayObject_t ] );
    const jagDraw::VertexArrayObject* vao( ( boost::static_pointer_cast< jagDraw::VertexArrayObject >( drawablePrep ) ).get() );
    if( vao == NULL )
    {
        JAG3D_WARNING( "computeBound(): NULL vertex array object." );
        return;
    }

    BufferObjectPtr bop( boost::dynamic_pointer_cast< BufferObject >(
        vao->getVertexArrayCommand( VertexArrayCommand::BufferObject_t, VertexArrayObject::Vertex ) ) );
    VertexAttribPtr verts( boost::dynamic_pointer_cast< VertexAttrib >(
        vao->getVertexArrayCommand( VertexArrayCommand::VertexAttrib_t, VertexArrayObject::Vertex ) ) );
    if( ( bop == NULL ) || ( verts == NULL ) )
    {
        JAG3D_WARNING( "computeBound(): NULL buffer object or vertex attrib (VAO has no vertex data)." );
        return;
    }

    GLint size;
    GLenum type;
    verts->getSizeType( size, type );
    typedef std::pair< GLint, GLenum > IntEnum;
    const IntEnum vertType( size, type );

    BOOST_FOREACH( DrawCommandPtr dcp, _drawCommands )
    {
        if( vertType == IntEnum( 3, GL_FLOAT ) )
        {
            VertexAttribContainer< gmtl::Point3f > vac( bop, verts, dcp );
            VertexAttribContainer< gmtl::Point3f >::iterator pointIter;
            for( pointIter = vac.begin(); pointIter != vac.end(); ++pointIter )
                bound->expand( *pointIter );
        }
        else if( vertType == IntEnum( 3, GL_DOUBLE ) )
        {
            VertexAttribContainer< gmtl::Point3d > vac( bop, verts, dcp );
            VertexAttribContainer< gmtl::Point3d >::iterator pointIter;
            for( pointIter = vac.begin(); pointIter != vac.end(); ++pointIter )
                bound->expand( *pointIter );
        }
        else
        {
            std::ostringstream ostr;
            ostr << "computeBound(): Unsupported VertexAttrib type/size combination.\n";
            ostr << "\tType: " << std::hex << type << ", size: " << std::dec << size;
            JAG3D_ERROR( ostr.str() );
        }
    }
}

void Drawable::setBoundDirty( const bool dirty )
{
    BoundOwner::setBoundDirty( dirty );
    if( dirty )
    {
        BoundDirtyNotifyInfo bdni;
        notify( bdni );
    }
}
void Drawable::setAllBoundsDirty( const bool dirty )
{
    BoundOwner::setAllBoundsDirty( dirty );
    if( dirty )
    {
        BoundDirtyNotifyInfo bdni;
        notify( bdni );
    }
}


void Drawable::addDrawCommand( DrawCommandPtr dcp )
{
    _drawCommands.push_back( dcp );
    setAllBoundsDirty();
}

DrawCommandVec& Drawable::getDrawCommandVec()
{
    return( _drawCommands );
}
const DrawCommandVec& Drawable::getDrawCommandVec() const
{
    return( _drawCommands );
}


void Drawable::setMaxContexts( const unsigned int numContexts )
{
    BOOST_FOREACH( DrawCommandPtr dcp, _drawCommands )
    {
        dcp->setMaxContexts( numContexts );
    }
}
void Drawable::deleteID( const jagDraw::jagDrawContextID contextID )
{
    BOOST_FOREACH( DrawCommandPtr dcp, _drawCommands )
    {
        dcp->deleteID( contextID );
    }
}


// jagDraw
}
