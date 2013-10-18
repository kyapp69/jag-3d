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

#include <jagUtil/Blur.h>

#include <jagDraw/Common.h>
#include <jagUtil/QuadNode.h>
#include <jagBase/Log.h>
#include <jagBase/LogMacros.h>

#include <gmtl/gmtl.h>

#include <boost/foreach.hpp>

#include <string>
#include <sstream>
#include <iomanip>


namespace jagUtil
{


Blur::Blur( const std::string& logName )
    : jagBase::LogBase( logName.empty() ? "jag.util.blur" : logName ),
      _numContexts( 0 ),
      _width( 0 ),
      _height( 0 )
{
}
Blur::Blur( jagDraw::TexturePtr& inputBuffer, jagDraw::TexturePtr& outputBuffer,
        const std::string& logName )
    : jagBase::LogBase( logName.empty() ? "jag.util.blur" : logName ),
      _inputBuffer( inputBuffer ),
      _outputBuffer( outputBuffer ),
      _numContexts( 0 ),
      _width( 0 ),
      _height( 0 )
{
}
Blur::Blur( const Blur& rhs )
    : jagBase::LogBase( rhs ),
      _inputBuffer( rhs._inputBuffer ),
      _outputBuffer( rhs._outputBuffer ),
      _numContexts( rhs._numContexts ),
      _width( rhs._width ),
      _height( rhs._height )
{
}
Blur::~Blur()
{
}

void Blur::setMaxContexts( const unsigned int numContexts )
{
    _numContexts = numContexts;

    if( _vQuad == NULL )
        internalInit();

    _container.setMaxContexts( numContexts );
}

jagDraw::NodeContainer& Blur::getNodeContainer()
{
    if( _vQuad == NULL )
        internalInit();

    setMaxContexts( _numContexts );

    return( _container );
}

void Blur::reshape( const int w, const int h )
{
    _width = w;
    _height = h;

    if( _vQuad == NULL )
        internalInit();

    _hQuad->reshape( w, h );
    _vQuad->reshape( w, h );

    _intermediateBuffer->getImage()->set( 0, GL_RGBA, _width, _height, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
    _intermediateBuffer->markAllDirty();
}

void Blur::internalInit()
{
    // Create the intermediate color buffer.
    jagDraw::ImagePtr image( new jagDraw::Image() );
    image->set( 0, GL_RGBA, _width, _height, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
    _intermediateBuffer.reset( new jagDraw::Texture( GL_TEXTURE_2D, image,
        jagDraw::SamplerPtr( new jagDraw::Sampler() ) ) );
    _intermediateBuffer->getSampler()->getSamplerState()->_minFilter = GL_NEAREST;
    _intermediateBuffer->getSampler()->getSamplerState()->_magFilter = GL_NEAREST;

    // Create the horizontal blur.
    _hQuad.reset( new QuadNode( _inputBuffer, _intermediateBuffer ) );
    //_hQuad->setShaders( "blurHorizontal.frag" );

    // Create the vertical blur.
    _vQuad.reset( new QuadNode( _intermediateBuffer, _outputBuffer ) );
    //_vQuad->setShaders( "blurVertical.frag" );

    // Configure the NodeContainer.
    _container.clear();
    _container.setResetEnable( false );
    // TBD add depth control.
    _container.push_back( _hQuad );
    _container.push_back( _vQuad );
}


// jagUtil
}