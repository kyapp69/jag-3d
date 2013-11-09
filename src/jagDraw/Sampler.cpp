/*************** <auto-copyright.rb BEGIN do not edit this line> **************
 
 Copyright 2012-2014 by Ames Laboratory and Skew Matrix Software, LLC
 
 The MIT License (MIT)
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 
 *************** <auto-copyright.rb END do not edit this line> ***************/

#include <jagDraw/Sampler.h>
#include <jagDraw/PlatformOpenGL.h>
#include <jagDraw/DrawInfo.h>
#include <jagDraw/Error.h>
#include <jagBase/LogMacros.h>
#include <jagDraw/Version.h>

#include <sstream>
#include <string>


namespace jagDraw {


Sampler::Sampler( const std::string& logName )
  : DrawablePrep( DrawablePrep::Sampler_t ),
    ObjectID(),
    jagBase::LogBase( logName.empty() ? "jag.draw.sampler" : logName ),
    _samplerState( SamplerStatePtr( new SamplerState() ) )
{
}
Sampler::Sampler( const Sampler& rhs )
  : DrawablePrep( rhs ),
    ObjectID( rhs ),
    jagBase::LogBase( rhs ),
    _samplerState( rhs._samplerState )
{
}
Sampler::~Sampler()
{
}


void Sampler::activate( DrawInfo& drawInfo, const unsigned int unit )
{
#ifdef GL_VERSION_3_3
    JAG3D_TRACE( "activate" );

    GLuint localUnit( ( unit >= GL_TEXTURE0 ) ? unit - GL_TEXTURE0 : unit );
    const GLuint id( getID( drawInfo._id ) );
    glBindSampler( localUnit, id );

    JAG3D_ERROR_CHECK( "Sampler::activate()" );
#endif
}

void Sampler::execute( DrawInfo& drawInfo )
{
#ifdef GL_VERSION_3_3
    const GLuint id( getID( drawInfo._id ) );

    SamplerState& state( *_samplerState );
    if( state.getDirty() )
    {
        glSamplerParameteri( id, GL_TEXTURE_WRAP_S, state._wrapS );
        glSamplerParameteri( id, GL_TEXTURE_WRAP_T, state._wrapT );
        glSamplerParameteri( id, GL_TEXTURE_WRAP_R, state._wrapR );

        glSamplerParameteri( id, GL_TEXTURE_MIN_FILTER, state._minFilter );
        glSamplerParameteri( id, GL_TEXTURE_MAG_FILTER, state._magFilter );

        glSamplerParameterfv( id, GL_TEXTURE_BORDER_COLOR, state._borderColor );

        glSamplerParameterf( id, GL_TEXTURE_MIN_LOD, state._minLOD );
        glSamplerParameterf( id, GL_TEXTURE_MAX_LOD, state._maxLOD );
        glSamplerParameteri( id, GL_TEXTURE_LOD_BIAS, state._lodBias );

        glSamplerParameteri( id, GL_TEXTURE_COMPARE_MODE, state._compareMode );
        glSamplerParameteri( id, GL_TEXTURE_COMPARE_FUNC, state._compareFunc );

        state.setDirty( false );
    }

    JAG3D_ERROR_CHECK( "Sampler::execute()" );
#endif
}


void Sampler::executeSampler( const jagDraw::jagDrawContextID contextID, const GLuint unit )
{
    JAG3D_TRACE( "executeSampler" );

#ifdef GL_VERSION_3_3

    const GLuint id( getID( contextID ) );
    JAG3D_ERROR_CHECK( "Sampler::executeSampler() pre" );
    glBindSampler( unit, id );
    JAG3D_ERROR_CHECK( "Sampler::executeSampler() post" );

    SamplerState& state( *_samplerState );
    if( state.getDirty() )
    {
        glSamplerParameteri( id, GL_TEXTURE_WRAP_S, state._wrapS );
        glSamplerParameteri( id, GL_TEXTURE_WRAP_T, state._wrapT );
        glSamplerParameteri( id, GL_TEXTURE_WRAP_R, state._wrapR );

        glSamplerParameteri( id, GL_TEXTURE_MIN_FILTER, state._minFilter );
        glSamplerParameteri( id, GL_TEXTURE_MAG_FILTER, state._magFilter );

        glSamplerParameterfv( id, GL_TEXTURE_BORDER_COLOR, state._borderColor );

        glSamplerParameterf( id, GL_TEXTURE_MIN_LOD, state._minLOD );
        glSamplerParameterf( id, GL_TEXTURE_MAX_LOD, state._maxLOD );
        glSamplerParameteri( id, GL_TEXTURE_LOD_BIAS, state._lodBias );

        glSamplerParameteri( id, GL_TEXTURE_COMPARE_MODE, state._compareMode );
        glSamplerParameteri( id, GL_TEXTURE_COMPARE_FUNC, state._compareFunc );

        state.setDirty( false );
    }

    JAG3D_ERROR_CHECK( "Sampler::executeSampler()" );

#else
    const std::string version( getOpenGLVersionString() );
    JAG3D_ERROR( "OpenGL version " + version +
        " does not support Sampler objects.\nMust be at least v3.3" );
#endif
}
void Sampler::executeTexture( const GLenum target )
{
    JAG3D_TRACE( "executeTexture" );

    SamplerState& state( *_samplerState );
    if( state.getDirty() )
    {
        glTexParameteri( target, GL_TEXTURE_WRAP_S, state._wrapS );
        glTexParameteri( target, GL_TEXTURE_WRAP_T, state._wrapT );
        glTexParameteri( target, GL_TEXTURE_WRAP_R, state._wrapR );

        glTexParameteri( target, GL_TEXTURE_MIN_FILTER, state._minFilter );
        glTexParameteri( target, GL_TEXTURE_MAG_FILTER, state._magFilter );

        glTexParameterfv( target, GL_TEXTURE_BORDER_COLOR, state._borderColor );

        glTexParameterf( target, GL_TEXTURE_MIN_LOD, state._minLOD );
        glTexParameterf( target, GL_TEXTURE_MAX_LOD, state._maxLOD );
        glTexParameteri( target, GL_TEXTURE_LOD_BIAS, state._lodBias );

        glTexParameteri( target, GL_TEXTURE_COMPARE_MODE, state._compareMode );
        glTexParameteri( target, GL_TEXTURE_COMPARE_FUNC, state._compareFunc );

        state.setDirty( false );
    }

    JAG3D_ERROR_CHECK( "Sampler::executeTexture()" );
}


SamplerStatePtr Sampler::getSamplerState() const
{
    return( _samplerState );
}


GLuint Sampler::getID( const jagDraw::jagDrawContextID contextID )
{
    if( _ids[ contextID ] == 0 )
    {
#ifdef GL_VERSION_3_3
        GLuint id;
        glGenSamplers( 1, &id );
        _ids[ contextID ] = id;

        std::ostringstream ostr;
        ostr << _ids[ contextID ];
        JAG3D_TRACE( "getID() generated sampler ID " + ostr.str() + "." );

        JAG3D_ERROR_CHECK( "Sampler::getID()" );
#endif
    }

    return( _ids[ contextID ] );
}
    
void Sampler::deleteID( const jagDraw::jagDrawContextID contextID )
{
}




SamplerState::SamplerState()
  : _wrapS( GL_REPEAT ),
    _wrapT( GL_REPEAT ),
    _wrapR( GL_REPEAT ),
    _minFilter( GL_NEAREST_MIPMAP_LINEAR ),
    _magFilter( GL_LINEAR ),
    _minLOD( -1000.f ),
    _maxLOD( 1000.f ),
    _lodBias( 0 ),
    _compareMode( GL_NONE ),
    _compareFunc( GL_LEQUAL ),
    _dirty( true )
{
    _borderColor[ 0 ] = _borderColor[ 1 ] = 
        _borderColor[ 2 ] = _borderColor[ 3 ] = 0.f;
}
SamplerState::SamplerState( const SamplerState& rhs )
  : _wrapS( rhs._wrapS ),
    _wrapT( rhs._wrapT ),
    _wrapR( rhs._wrapR ),
    _minFilter( rhs._minFilter ),
    _magFilter( rhs._magFilter ),
    _minLOD( rhs._minLOD ),
    _maxLOD( rhs._maxLOD ),
    _lodBias( rhs._lodBias ),
    _compareMode( rhs._compareMode ),
    _compareFunc( rhs._compareFunc ),
    _dirty( rhs._dirty )
{
    std::memcpy( _borderColor, rhs._borderColor, sizeof( _borderColor ) );
}
SamplerState::~SamplerState()
{
}


// jagDraw
}
