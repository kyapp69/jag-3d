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

#ifndef __JAG_DRAW_SAMPLER_H__
#define __JAG_DRAW_SAMPLER_H__ 1


#include <jag/draw/Export.h>
#include <jag/draw/Command.h>
#include <jag/draw/PlatformOpenGL.h>
#include <jag/draw/ObjectID.h>
#include <jag/base/LogBase.h>
#include <jag/base/ptr.h>

#include <boost/foreach.hpp>
#include <vector>


namespace jag {
namespace draw {


/** \addtogroup jagDrawGLCommands OpenGL Commands */
/*@{*/

// forward
struct SamplerState;

typedef jag::base::ptr< jag::draw::SamplerState >::shared_ptr SamplerStatePtr;


/** \class Sampler Sampler.h <jag/draw/Sampler.h>
\brief Support for Sampler objects.
\details
\gl{3.8.2}

Sampler objects were introduced in OpenGL v3.3 to provide object storage and binding
for parameters that control the behavior of GLSL texture samplers. Currently, the
Sampler class provides backwards compatible behavior for OpenGL v3.2 in order to support
Mac OSX. When OSX moves to a more recent version of OpenGL, much of the Sampler
implementation could be cleaned up at that time.
*/
class JAGDRAW_EXPORT Sampler : public Command,
        public ObjectID, protected jag::base::LogBase
{
public:
    Sampler( const std::string& logName=std::string( "" ) );
    Sampler( const Sampler& rhs );
    ~Sampler();


    /** \brief TBD
    \details TBD */
    virtual CommandPtr clone() const { return( CommandPtr( new Sampler( *this ) ) ); }

    /** \brief TBD
    \details TBD */
    virtual void activate( DrawInfo&, const unsigned int unit );

    /** \brief TBD
    \details TBD */
    virtual void execute( DrawInfo& );

    /** \brief Add the SamplerState to the Sampler object.
    \derails Also binds the Sampler object (and leaves it bound).
    */
    void executeSampler( const jag::draw::jagDrawContextID contextID, const GLuint unit );

    /** \brief Set the SamplerState as texture parameters.
    \details TBD.
    */
    void executeTexture( const GLenum target );


    /** \brief TBD
    \details TBD */
    SamplerStatePtr getSamplerState() const;


    /** \brief TBD
    Override from ObjectID. */
    virtual GLuint getID( const jag::draw::jagDrawContextID contextID );
    
    /** \brief TBD
    Override from ObjectID. */
    virtual void deleteID( const jag::draw::jagDrawContextID contextID );

protected:
    SamplerStatePtr _samplerState;
};

typedef jag::base::ptr< jag::draw::Sampler >::shared_ptr SamplerPtr;



/** \class SamplerState Sampler.h <jag/draw/Sampler.h>
\brief Comtainer for Sampler object parameters.
\details

All parameter defaults are per \glshort{table 6.18}, except:
\li _minFilter defaults to GL_NEAREST_MIPMAP_LINEAR.
\li _wrapS, _wrapT, and _wrapR default to GL_REPEAT.
Note that client code should change these values when using a SamplerState
with rectangle textures.
*/
struct JAGDRAW_EXPORT SamplerState
{
    SamplerState();
    SamplerState( const SamplerState& rhs );
    virtual ~SamplerState();

    void setDirty( const bool dirty=true ) { _dirty = dirty; }
    bool getDirty() const { return( _dirty ); }

    GLenum _wrapS, _wrapT, _wrapR;
    GLenum _minFilter, _magFilter;
    GLfloat _borderColor[4];
    GLfloat _minLOD, _maxLOD;
    GLint _lodBias;
    GLenum _compareMode, _compareFunc;

    bool _dirty;
};



class SamplerSet;
typedef jag::base::ptr< jag::draw::SamplerSet >::shared_ptr SamplerSetPtr;

/** \class SamplerSet Sampler.h <jag/draw/Sampler.h>
\brief TBD
\details TBD */
class SamplerSet : public ObjectIDOwner,
        public CommandSet< unsigned int, SamplerPtr, SamplerSet, SamplerSetPtr >
{
protected:
    typedef CommandSet< unsigned int, SamplerPtr, SamplerSet, SamplerSetPtr > SET_TYPE;

public:
    SamplerSet()
      : ObjectIDOwner(),
        SET_TYPE( SamplerSet_t )
    {}
    SamplerSet( const SamplerSet& rhs )
      : ObjectIDOwner( rhs ),
        SET_TYPE( rhs )
    {}
    ~SamplerSet()
    {}


    /** \brief TBD
    \details TBD */
    virtual CommandPtr clone() const
    {
        return( SamplerSetPtr( new SamplerSet( *this ) ) );
    }

    /** \brief TBD
    \details Override method from Command. */
    virtual void execute( DrawInfo& drawInfo )
    {
        BOOST_FOREACH( const MAP_TYPE::value_type& dataPair, *this )
        {
            const SamplerPtr& sampler( dataPair.second );
            sampler->activate( drawInfo, dataPair.first );
            sampler->execute( drawInfo );
        }
    }

    /** \brief TBD
    \details Override method from ObjectIDOwner */
    virtual void setMaxContexts( const unsigned int numContexts )
    {
        BOOST_FOREACH( const MAP_TYPE::value_type& dataPair, *this )
        {
            dataPair.second->setMaxContexts( numContexts );
        }
    }
    /** \brief TBD
    \details Override method from ObjectIDOwner */
    virtual void deleteID( const jag::draw::jagDrawContextID contextID )
    {
        BOOST_FOREACH( const MAP_TYPE::value_type& dataPair, *this )
        {
            dataPair.second->deleteID( contextID );
        }
    }
};


/*@}*/


// namespace jag::draw::
}
}


// __JAG_DRAW_SAMPLER_H__
#endif
