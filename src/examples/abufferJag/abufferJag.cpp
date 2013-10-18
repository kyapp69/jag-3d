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

#include <demoSupport/DemoInterface.h>

#include <jagDraw/Common.h>
#include <jagSG/Common.h>
#include <jagUtil/ABuffer.h>
#include <jagUtil/Blur.h>
#include <jagDisk/ReadWrite.h>
#include <jagBase/Profile.h>
#include <jagUtil/DrawGraphCountVisitor.h>
#include <jagUtil/Shapes.h>
#include <jagBase/Version.h>
#include <jagBase/Log.h>
#include <jagBase/LogMacros.h>

#include <jagUtil/BufferAggregationVisitor.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <gmtl/gmtl.h>

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#include <string>
#include <sstream>


using namespace std;
namespace bpo = boost::program_options;


class ABufferJag : public DemoInterface
{
public:
    ABufferJag()
      : DemoInterface( "jag.ex.abuf" ),
        _fileName( "cow.osg" ),
        _moveRate( 1. ),
        _width( 800 ),
        _height( 600 )
    {}
    virtual ~ABufferJag() {}

    virtual bool parseOptions( boost::program_options::variables_map& vm );

    virtual bool startup( const unsigned int numContexts );
    virtual bool init();
    virtual bool frame( const gmtl::Matrix44d& view, const gmtl::Matrix44d& proj );
    virtual void reshape( const int w, const int h );
    virtual bool keyCommand( const int command );

    // Return a value to bontrol base gamepad move rate in the scene.
    virtual double getMoveRate() const
    {
        return( _moveRate );
    }

protected:
    std::string _fileName;

    jagSG::NodePtr _root;

    double _moveRate;

    jagUtil::ABufferPtr _aBuffer;

    jagDraw::FramebufferPtr _opaqueFBO;
    jagDraw::TexturePtr _opaqueBuffer, _secondaryBuffer, _glowBuffer, _depthBuffer;
    jagUtil::BlurPtr _blur;

    int _width, _height;
};


DemoInterface* DemoInterface::create( bpo::options_description& desc )
{
    desc.add_options()
        ( "file,f", bpo::value< std::string >(), "Model to load. Default: cow.osg" );

    return( new ABufferJag );
}

bool ABufferJag::parseOptions( bpo::variables_map& vm )
{
    if( vm.count( "file" ) > 0 )
        _fileName = vm[ "file" ].as< std::string >();
    return( true );
}


bool ABufferJag::startup( const unsigned int numContexts )
{
    DemoInterface::startup( numContexts );

    JAG3D_INFO_STATIC( _logName, _fileName );

    if( _fileName.empty() )
    {
        JAG3D_FATAL_STATIC( _logName, "Specify '--file <fileName>' on command line." );
        return( false );
    }


    // Create the texture used by _opaqueFBO to store the opaque
    // color buffer. After our opaque NodeContainer #0 renders into it, 
    // the ABuffer class's third NodeContainer uses it during the
    // abuffer resolve.
    jagDraw::ImagePtr image( new jagDraw::Image() );
    image->set( 0, GL_RGBA, _width, _height, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
    _opaqueBuffer.reset( new jagDraw::Texture( GL_TEXTURE_2D, image,
        jagDraw::SamplerPtr( new jagDraw::Sampler() ) ) );
    _opaqueBuffer->getSampler()->getSamplerState()->_minFilter = GL_NEAREST;
    _opaqueBuffer->getSampler()->getSamplerState()->_magFilter = GL_NEAREST;
    _opaqueBuffer->setMaxContexts( numContexts );

    // Create second color buffer for glow effect.
    _secondaryBuffer.reset( new jagDraw::Texture( GL_TEXTURE_2D, image,
        jagDraw::SamplerPtr( new jagDraw::Sampler() ) ) );
    _secondaryBuffer->getSampler()->getSamplerState()->_minFilter = GL_NEAREST;
    _secondaryBuffer->getSampler()->getSamplerState()->_magFilter = GL_NEAREST;
    _secondaryBuffer->setMaxContexts( numContexts );

    // Create glow effect output buffer
    _glowBuffer.reset( new jagDraw::Texture( GL_TEXTURE_2D, image,
        jagDraw::SamplerPtr( new jagDraw::Sampler() ) ) );
    _glowBuffer->getSampler()->getSamplerState()->_minFilter = GL_NEAREST;
    _glowBuffer->getSampler()->getSamplerState()->_magFilter = GL_NEAREST;
    _glowBuffer->setMaxContexts( numContexts );

    // Depth texture used as depth buffer for opaque pass.
    // Also used during abuffer render to discard occluded fragments.
    image->set( 0, GL_DEPTH_COMPONENT, _width, _height, 1, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL );
    _depthBuffer.reset( new jagDraw::Texture( GL_TEXTURE_2D, image,
        jagDraw::SamplerPtr( new jagDraw::Sampler() ) ) );
    _depthBuffer->getSampler()->getSamplerState()->_minFilter = GL_NEAREST;
    _depthBuffer->getSampler()->getSamplerState()->_magFilter = GL_NEAREST;
    _depthBuffer->setMaxContexts( numContexts );

    // Create the ABuffer management object.
    _aBuffer.reset( new jagUtil::ABuffer( _depthBuffer, _opaqueBuffer, _glowBuffer ) );
    _aBuffer->setMaxContexts( numContexts );

    // Obtain the draw graph from the ABuffer object.
    // Default behavior is that the ABuffer owns NodeContainers 1-3, and we put
    // all opaque geometry in NodeContainer 0.
    jagDraw::DrawGraphPtr drawGraph( _aBuffer->createDrawGraphTemplate( 2 ) );

    // Create blur effect NodeContainer
    _blur.reset( new jagUtil::Blur( _secondaryBuffer, _glowBuffer ) );
    _blur->setMaxContexts( numContexts );
    (*drawGraph)[ 1 ] = _blur->getNodeContainer();

    getCollectionVisitor().setDrawGraphTemplate( drawGraph );

    // Allow ABuffer object to specify which matrices it needs.
    jagDraw::TransformCallback* xformCB( getCollectionVisitor().getTransformCallback() );
    xformCB->setRequiredMatrixUniforms(
        xformCB->getRequiredMatrixUniforms() |
        _aBuffer->getRequiredMatrixUniforms() );


    // Prepare the scene graph.
    jagSG::NodePtr model( DemoInterface::readSceneGraphNodeUtil( _fileName ) );
    if( model == NULL )
        return( false );

    // Add model instance as opaque
    _root.reset( new jagSG::Node );
    _root->addChild( model );

    // Add translated model instance for ABuffer transparency
    jagSG::NodePtr xformNode( jagSG::NodePtr( new jagSG::Node() ) );
    gmtl::setTrans( xformNode->getTransform(), gmtl::Vec3d( 0., model->getBound()->getRadius() * -1.5, 0. ) );
    _root->addChild( xformNode );
    xformNode->addChild( model );
    {
        // Specify control objects for abuffer rendering.
        jagDraw::CommandMapPtr commands;
        jagSG::SelectContainerCallbackPtr sccb;
        _aBuffer->getABufferControls( commands, sccb );

        xformNode->setCommandMap( commands );
        xformNode->getCollectionCallbacks().push_back( sccb );
    }


    // For gamepad speed control
    _moveRate = _root->getBound()->getRadius();


    // Prepare for culling.
    jagSG::FrustumCullDistributionVisitor fcdv;
    _root->accept( fcdv );
    jagSG::SmallFeatureDistributionVisitor sfdv;
    _root->accept( sfdv );

    // Optimize VAO and element buffers.
    jagUtil::BufferAggregationVisitor bav( _root );


    // The scene graph will have two CommandMaps, one for opaque
    // rendering and the other for ABuffer transparency. The opaque
    // CommandMap will sit at the root of the scene graph, and the
    // application will add or remove the ABuffer CommandMap as
    // needed to toggle transparency on and off.

    // Create opaque CommandMap.
    jagDraw::ShaderPtr vs( DemoInterface::readShaderUtil( "jagmodel.vert" ) );
    jagDraw::ShaderPtr fs( DemoInterface::readShaderUtil( "abufferJagGlow.frag" ) );

    jagDraw::ProgramPtr prog;
    prog = jagDraw::ProgramPtr( new jagDraw::Program );
    prog->attachShader( vs );
    prog->attachShader( fs );

    jagDraw::CommandMapPtr commands( _root->getOrCreateCommandMap() );
    commands->insert( prog );

    // Create a framebuffer object. The color texture will store opaque
    // rendering. The depth texture is shared with ABuffer to avoid
    // rendering behind opaque geometry.
    _opaqueFBO.reset( new jagDraw::Framebuffer( GL_DRAW_FRAMEBUFFER ) );
    _opaqueFBO->setViewport( 0, 0, _width, _height );
    _opaqueFBO->setClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    _opaqueFBO->setClearColor( 0.f, 0.f, 0.f, 0.f );
    _opaqueFBO->addAttachment( GL_DEPTH_ATTACHMENT, _depthBuffer );
    _opaqueFBO->addAttachment( GL_COLOR_ATTACHMENT0, _opaqueBuffer );
    _opaqueFBO->addAttachment( GL_COLOR_ATTACHMENT1, _secondaryBuffer );
    commands->insert( _opaqueFBO );

    // Set up lighting uniforms
    jagDraw::UniformBlockPtr lights( jagDraw::UniformBlockPtr(
        new jagDraw::UniformBlock( "LightingLight" ) ) );
    gmtl::Vec3f dir( 0.f, 0.f, 1.f );
    gmtl::normalize( dir );
    gmtl::Point4f lightVec( dir[0], dir[1], dir[2], 0. );
    lights->addUniform( jagDraw::UniformPtr(
        new jagDraw::Uniform( "position", lightVec ) ) );
    lights->addUniform( jagDraw::UniformPtr(
        new jagDraw::Uniform( "ambient", gmtl::Point4f( 0.f, 0.f, 0.f, 1.f ) ) ) );
    lights->addUniform( jagDraw::UniformPtr(
        new jagDraw::Uniform( "diffuse", gmtl::Point4f( 1.f, 1.f, 1.f, 1.f ) ) ) );
    lights->addUniform( jagDraw::UniformPtr(
        new jagDraw::Uniform( "specular", gmtl::Point4f( 1.f, 1.f, 1.f, 1.f ) ) ) );

    jagDraw::UniformBlockPtr backMaterials( jagDraw::UniformBlockPtr(
        new jagDraw::UniformBlock( "LightingMaterialBack" ) ) );
    backMaterials->addUniform( jagDraw::UniformPtr(
        new jagDraw::Uniform( "ambient", gmtl::Point4f( .1f, .1f, .1f, 1.f ) ) ) );
    backMaterials->addUniform( jagDraw::UniformPtr(
        new jagDraw::Uniform( "diffuse", gmtl::Point4f( .7f, .7f, .7f, 1.f ) ) ) );
    backMaterials->addUniform( jagDraw::UniformPtr(
        new jagDraw::Uniform( "specular", gmtl::Point4f( .5f, .5f, .5f, 1.f ) ) ) );
    backMaterials->addUniform( jagDraw::UniformPtr(
        new jagDraw::Uniform( "shininess", 16.f ) ) );

    jagDraw::UniformBlockPtr frontMaterials( jagDraw::UniformBlockPtr(
        new jagDraw::UniformBlock( "LightingMaterialFront" ) ) );
    frontMaterials->addUniform( jagDraw::UniformPtr(
        new jagDraw::Uniform( "ambient", gmtl::Point4f( .1f, .1f, .1f, 1.f ) ) ) );
    frontMaterials->addUniform( jagDraw::UniformPtr(
        new jagDraw::Uniform( "diffuse", gmtl::Point4f( .7f, .7f, .7f, 1.f ) ) ) );
    frontMaterials->addUniform( jagDraw::UniformPtr(
        new jagDraw::Uniform( "specular", gmtl::Point4f( .5f, .5f, .5f, 1.f ) ) ) );
    frontMaterials->addUniform( jagDraw::UniformPtr(
        new jagDraw::Uniform( "shininess", 16.f ) ) );

    jagDraw::UniformBlockSetPtr ubsp( jagDraw::UniformBlockSetPtr(
        new jagDraw::UniformBlockSet() ) );
    ubsp->insert( lights );
    ubsp->insert( backMaterials );
    ubsp->insert( frontMaterials );
    commands->insert( ubsp );


    // We have potentially different views per window, so we keep an MxCore
    // per context. Initialize the MxCore objects and create default views.
    const jagDraw::BoundPtr bound( _root->getBound() );
    const gmtl::Point3d pos( bound->getCenter() + gmtl::Vec3d( 0., -1., 0. ) );
    for( unsigned int idx( 0 ); idx<numContexts; ++idx )
    {
        jagMx::MxCorePtr mxCore( new jagMx::MxCore() );
        mxCore->setAspect( 1. );
        mxCore->setFovy( 30. );
        mxCore->setPosition( pos );
        mxCore->setOrbitCenterPoint( bound->getCenter() );
        mxCore->lookAtAndFit( bound->asSphere() );
        _mxCore._data.push_back( mxCore );
    }


    // Tell scene graph how many contexts to expect.
    _root->setMaxContexts( numContexts );


    return( true );
}

bool ABufferJag::init()
{
    glEnable( GL_DEPTH_TEST );

    // Auto-log the version string.
    jagBase::getVersionString();

    // Auto-log the OpenGL version string.
    jagDraw::getOpenGLVersionString();

    return( true );
}


#define ENABLE_SORT

bool ABufferJag::frame( const gmtl::Matrix44d& view, const gmtl::Matrix44d& proj )
{
    if( !getStartupCalled() )
        return( true );

    JAG3D_PROFILE( "frame" );

#ifdef ENABLE_SORT
    jagDraw::DrawablePrep::CommandTypeVec plist;
    plist.push_back( jagDraw::DrawablePrep::UniformBlockSet_t );
#endif

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    const jagDraw::jagDrawContextID contextID( jagDraw::ContextSupport::instance()->getActiveContext() );
    jagDraw::DrawInfo& drawInfo( getDrawInfo( contextID ) );

    jagMx::MxCorePtr mxCore( _mxCore._data[ contextID ] );

    jagSG::CollectionVisitor& collect( getCollectionVisitor() );
    collect.reset();

    gmtl::Matrix44d viewMatrix;
    {
        JAG3D_PROFILE( "Collection" );

        // Set view and projection to define the collection frustum.
        viewMatrix = mxCore->getInverseMatrix();
        collect.setViewProj( viewMatrix, mxCore->computeProjection( .1, 25000. ) );

        {
            JAG3D_PROFILE( "Collection traverse" );
            // Collect a draw graph.
            _root->accept( collect );
        }
#ifdef ENABLE_SORT
        {
            JAG3D_PROFILE( "Collection sort" );
            jagDraw::DrawGraphPtr drawGraph( collect.getDrawGraph() );
            BOOST_FOREACH( jagDraw::NodeContainer& nc, *drawGraph )
            {
                std::sort( nc.begin(), nc.end(), jagDraw::DrawNodeCommandSorter( plist ) );
            }
        }
#endif
    }

    {
        JAG3D_PROFILE( "Render" );

        // Execute the draw graph.
        jagDraw::DrawGraphPtr drawGraph( collect.getDrawGraph() );

        // Set view and projection to use for drawing. Create projection using
        // the computed near and far planes.
        double minNear, maxFar;
        collect.getNearFar( minNear, maxFar );
        drawGraph->setViewProj( viewMatrix, mxCore->computeProjection( minNear, maxFar ) );

        // The ABuffer object handles rendering.
        // This line replaces drawGraph->execute( drawInfo );
        _aBuffer->renderFrame( collect, drawInfo );
    }
#ifdef JAG3D_ENABLE_PROFILING
    {
        // If profiling, dump out draw graph info.
        jagUtil::DrawGraphCountVisitor dgcv;
        dgcv.traverse( *( collect.getDrawGraph() ) );
        dgcv.dump( std::cout );
    }
#endif

    glFlush();

    JAG3D_ERROR_CHECK( "jagmodel display()" );

    return( true );
}

void ABufferJag::reshape( const int w, const int h )
{
    if( !getStartupCalled() )
        return;

    _width = w;
    _height = h;

    _aBuffer->reshape( w, h );
    _blur->reshape( w, h );

    // TBD reshape _opaqueBuffer and _depthBuffer.
    jagDraw::ImagePtr image( new jagDraw::Image() );
    image->set( 0, GL_RGBA, w, h, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
    _opaqueBuffer->setImage( image );
    _opaqueBuffer->markAllDirty();
    _secondaryBuffer->setImage( image );
    _secondaryBuffer->markAllDirty();
    _glowBuffer->setImage( image );
    _glowBuffer->markAllDirty();

    image.reset( new jagDraw::Image() );
    image->set( 0, GL_DEPTH_COMPONENT, w, h, 1, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL );
    _depthBuffer->setImage( image );
    _depthBuffer->markAllDirty();


    // Set aspect for all matrix control objects.
    const jagDraw::jagDrawContextID contextID( jagDraw::ContextSupport::instance()->getActiveContext() );
    _mxCore._data[ contextID ]->setAspect( ( double ) w / ( double ) h );
}

bool ABufferJag::keyCommand( const int command )
{
    switch( command )
    {
    default:
        return( false ); // Unhandled. Do not redraw.
        break;
    case (int)'g':
        _aBuffer->setResolveMethod( jagUtil::ABuffer::RESOLVE_GELLY );
        JAG3D_CRITICAL_STATIC( _logName, "Using " + jagUtil::ABuffer::resolveMethodToString( _aBuffer->getResolveMethod() ) );
        break;
    case (int)'a':
        _aBuffer->setResolveMethod( jagUtil::ABuffer::RESOLVE_ALPHA_BLEND );
        JAG3D_CRITICAL_STATIC( _logName, "Using " + jagUtil::ABuffer::resolveMethodToString( _aBuffer->getResolveMethod() ) );
        break;
    case (int)'c':
        _aBuffer->setResolveMethod( jagUtil::ABuffer::RESOLVE_ALPHA_BLEND_CAD );
        JAG3D_CRITICAL_STATIC( _logName, "Using " + jagUtil::ABuffer::resolveMethodToString( _aBuffer->getResolveMethod() ) );
        break;

    case (int)'t':
        _aBuffer->setFragmentAlpha( _aBuffer->getFragmentAlpha() - 0.033f );
        break;
    case (int)'T':
        _aBuffer->setFragmentAlpha( _aBuffer->getFragmentAlpha() + 0.033f );
        break;
    }

    return( true ); // cause redraw.
}