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

#include <demoSupport/DemoInterface.h>

#include <jag/draw/Common.h>
#include <jag/draw/PerContextData.h>
#include <jag/sg/Common.h>
#include <jag/disk/ReadWrite.h>
#include <jag/base/Profile.h>
#include <jag/util/DrawGraphCountVisitor.h>
#include <jag/base/Version.h>
#include <jag/base/Log.h>
#include <jag/base/LogMacros.h>

#include <jag/util/BufferAggregationVisitor.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <gmtl/gmtl.h>

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#include <string>
#include <sstream>


using namespace std;
namespace bpo = boost::program_options;


class JagModel : public DemoInterface
{
public:
    JagModel()
      : DemoInterface( "jag.ex.jagmodel" ),
        _fileName( "cow.osg" ),
        _vertName( "jagmodel.vert" ),
        _geomName( "" ),
        _fragName( "jagmodel.frag" ),
        _drawBound( false ),
        _moveRate( 1. )
    {}
    virtual ~JagModel() {}

    virtual bool parseOptions( bpo::variables_map& vm );

    virtual bool startup( const unsigned int numContexts );
    virtual bool init();
    virtual bool frame( const gmtl::Matrix44d& view, const gmtl::Matrix44d& proj );
    virtual void reshape( const int w, const int h );

    // Return a value to bontrol base gamepad move rate in the scene.
    virtual double getMoveRate() const
    {
        return( _moveRate );
    }

    virtual WinSizeType defaultWinSize() const
    {
        WinSizeType ws;
        ws.push_back( 800 ); ws.push_back( 600 );
        return( ws );
    }

protected:
    std::string _fileName;

    jag::sg::NodePtr _root;

    std::string _vertName, _geomName, _fragName;

    bool _drawBound;
    jag::draw::ProgramPtr _boundProgram;

    double _moveRate;
};


DemoInterface* DemoInterface::create( bpo::options_description& desc )
{
    desc.add_options()
        ( "bound", "Render draw graph Node bounds." )
        ( "file,f", bpo::value< std::string >(), "Model to load. Default: cow.osg" )
        ( "vs", bpo::value< std::string >(), "Specify a vertex shader. Default is jagmodel.vert." )
        ( "gs", bpo::value< std::string >(), "Specify a geometry shader." )
        ( "fs", bpo::value< std::string >(), "Specify a fragment shader. Default is jagmodel.frag." );

    return( new JagModel );
}

bool JagModel::parseOptions( bpo::variables_map& vm )
{
    if( vm.count( "file" ) > 0 )
        _fileName = vm[ "file" ].as< std::string >();

    _drawBound = ( vm.count( "bound" ) > 0 );

    if( vm.count( "vs" ) > 0 )
        _vertName = vm[ "vs" ].as< std::string >();
    if( vm.count( "gs" ) > 0 )
        _geomName = vm[ "gs" ].as< std::string >();
    if( vm.count( "fs" ) > 0 )
        _fragName = vm[ "fs" ].as< std::string >();

    return( true );
}

bool JagModel::startup( const unsigned int numContexts )
{
    DemoInterface::startup( numContexts );

    JAG3D_INFO_STATIC( _logName, "File name: " + _fileName );
    JAG3D_INFO_STATIC( _logName, "Vert shader: " + _vertName );
    JAG3D_INFO_STATIC( _logName, "Geom shader: " + _geomName );
    JAG3D_INFO_STATIC( _logName, "Frag shader: " + _fragName );

    if( _fileName.empty() )
    {
        JAG3D_FATAL_STATIC( _logName, "Specify '--file <fileName>' on command line." );
        return( false );
    }


    // Prepare the draw graph.
    jag::draw::DrawGraphPtr drawGraphTemplate( new jag::draw::DrawGraph() );
    drawGraphTemplate->resize( 1 );
    getCollectionVisitor().setDrawGraphTemplate( drawGraphTemplate );


    // Prepare the scene graph.
    _root = DemoInterface::readSceneGraphNodeUtil( _fileName );
    if( _root == NULL )
        return( false );

    // For gamepad speed control
    _moveRate = _root->getBound()->getRadius();

        

    jag::sg::FrustumCullDistributionVisitor fcdv;
    _root->accept( fcdv );
    jag::sg::SmallFeatureDistributionVisitor sfdv;
    _root->accept( sfdv );

    jag::util::BufferAggregationVisitor bav( _root );


    jag::draw::ShaderPtr vs( DemoInterface::readShaderUtil( _vertName ) );
    jag::draw::ShaderPtr fs( DemoInterface::readShaderUtil( _fragName ) );
    jag::draw::ShaderPtr gs( (jag::draw::Shader*) NULL );
    if( !( _geomName.empty() ) )
        gs = DemoInterface::readShaderUtil( _geomName );


    jag::draw::ProgramPtr prog;
    prog = jag::draw::ProgramPtr( new jag::draw::Program );
    prog->attachShader( vs );
    prog->attachShader( fs );
    if( gs != NULL )
        prog->attachShader( gs );

    jag::draw::CommandMapPtr commands( _root->getOrCreateCommandMap() );
    commands->insert( prog );

    // Set up lighting uniforms
    jag::draw::UniformBlockPtr lights( jag::draw::UniformBlockPtr(
        new jag::draw::UniformBlock( "LightingLight" ) ) );
    gmtl::Vec3f dir( 0.f, 0.f, 1.f );
    gmtl::normalize( dir );
    gmtl::Point4f lightVec( dir[0], dir[1], dir[2], 0. );
    lights->addUniform( jag::draw::UniformPtr(
        new jag::draw::Uniform( "position", lightVec ) ) );
    lights->addUniform( jag::draw::UniformPtr(
        new jag::draw::Uniform( "ambient", gmtl::Point4f( 0.f, 0.f, 0.f, 1.f ) ) ) );
    lights->addUniform( jag::draw::UniformPtr(
        new jag::draw::Uniform( "diffuse", gmtl::Point4f( 1.f, 1.f, 1.f, 1.f ) ) ) );
    lights->addUniform( jag::draw::UniformPtr(
        new jag::draw::Uniform( "specular", gmtl::Point4f( 1.f, 1.f, 1.f, 1.f ) ) ) );

    jag::draw::UniformBlockPtr backMaterials( jag::draw::UniformBlockPtr(
        new jag::draw::UniformBlock( "LightingMaterialBack" ) ) );
    backMaterials->addUniform( jag::draw::UniformPtr(
        new jag::draw::Uniform( "ambient", gmtl::Point4f( .1f, .1f, .1f, 1.f ) ) ) );
    backMaterials->addUniform( jag::draw::UniformPtr(
        new jag::draw::Uniform( "diffuse", gmtl::Point4f( .7f, .7f, .7f, 1.f ) ) ) );
    backMaterials->addUniform( jag::draw::UniformPtr(
        new jag::draw::Uniform( "specular", gmtl::Point4f( .5f, .5f, .5f, 1.f ) ) ) );
    backMaterials->addUniform( jag::draw::UniformPtr(
        new jag::draw::Uniform( "shininess", 16.f ) ) );

    jag::draw::UniformBlockPtr frontMaterials( jag::draw::UniformBlockPtr(
        new jag::draw::UniformBlock( "LightingMaterialFront" ) ) );
    frontMaterials->addUniform( jag::draw::UniformPtr(
        new jag::draw::Uniform( "ambient", gmtl::Point4f( .1f, .1f, .1f, 1.f ) ) ) );
    frontMaterials->addUniform( jag::draw::UniformPtr(
        new jag::draw::Uniform( "diffuse", gmtl::Point4f( .7f, .7f, .7f, 1.f ) ) ) );
    frontMaterials->addUniform( jag::draw::UniformPtr(
        new jag::draw::Uniform( "specular", gmtl::Point4f( .5f, .5f, .5f, 1.f ) ) ) );
    frontMaterials->addUniform( jag::draw::UniformPtr(
        new jag::draw::Uniform( "shininess", 16.f ) ) );

    jag::draw::UniformBlockSetPtr ubsp( jag::draw::UniformBlockSetPtr(
        new jag::draw::UniformBlockSet() ) );
    ubsp->insert( lights );
    ubsp->insert( backMaterials );
    ubsp->insert( frontMaterials );
    commands->insert( ubsp );


    {
        jag::draw::ShaderPtr vs( DemoInterface::readShaderUtil( "bound.vert" ) );
        jag::draw::ShaderPtr gs( DemoInterface::readShaderUtil( "bound.geom" ) );
        jag::draw::ShaderPtr fs( DemoInterface::readShaderUtil( "bound.frag" ) );

        _boundProgram.reset( new jag::draw::Program() );
        _boundProgram->attachShader( vs );
        _boundProgram->attachShader( gs );
        _boundProgram->attachShader( fs );

        _boundProgram->setMaxContexts( numContexts );
    }


    // We have potentially different views per window, so we keep an MxCore
    // per context. Initialize the MxCore objects and create default views.
    const jag::draw::BoundPtr bound( _root->getBound() );
    const gmtl::Point3d pos( bound->getCenter() + gmtl::Vec3d( 0., -1., 0. ) );
    for( unsigned int idx( 0 ); idx<numContexts; ++idx )
    {
        jag::mx::MxCorePtr mxCore( new jag::mx::MxCore() );
        mxCore->setAspect( 1. );
        mxCore->setFovy( 30. );
        mxCore->setPosition( pos );
        mxCore->setOrbitCenterPoint( bound->getCenter() );
        mxCore->lookAtAndFit( bound->asSphere() );
        _mxCore._data.push_back( mxCore );
    }


    // Tell all Jag3D objects how many contexts to expect.
    _root->setMaxContexts( numContexts );

    return( true );
}

bool JagModel::init()
{
    glClearColor( 0.f, 0.f, 0.f, 0.f );

    glEnable( GL_DEPTH_TEST );

    // Auto-log the version string.
    jag::base::getVersionString();

    // Auto-log the OpenGL version string.
    jag::draw::getOpenGLVersionString();

    return( true );
}


#define ENABLE_SORT

bool JagModel::frame( const gmtl::Matrix44d& view, const gmtl::Matrix44d& proj )
{
    if( !getStartupCalled() )
        return( true );

    JAG3D_PROFILE( "frame" );

#ifdef ENABLE_SORT
    jag::draw::Command::CommandTypeVec plist;
    plist.push_back( jag::draw::Command::UniformBlockSet_t );
#endif

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    const jag::draw::jagDrawContextID contextID( jag::draw::ContextSupport::instance()->getActiveContext() );
    jag::draw::DrawInfo& drawInfo( getDrawInfo( contextID ) );

    jag::mx::MxCorePtr mxCore( _mxCore._data[ contextID ] );

    jag::sg::CollectionVisitor& collect( getCollectionVisitor() );
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
            jag::draw::DrawGraphPtr drawGraph( collect.getDrawGraph() );
            BOOST_FOREACH( jag::draw::DrawNodeContainer& nc, *drawGraph )
            {
                std::sort( nc.begin(), nc.end(), jag::draw::DrawNodeCommandSorter( plist ) );
            }
        }
#endif
    }

    {
        JAG3D_PROFILE( "Render" );

        // Execute the draw graph.
        jag::draw::DrawGraphPtr drawGraph( collect.getDrawGraph() );

        // Set view and projection to use for drawing. Create projection using
        // the computed near and far planes.
        double minNear, maxFar;
        collect.getNearFar( minNear, maxFar );
        drawGraph->setViewProj( viewMatrix, mxCore->computeProjection( minNear, maxFar ) );

        drawGraph->execute( drawInfo );
    }
#ifdef JAG3D_ENABLE_PROFILING
    {
        // If profiling, dump out draw graph info.
        jag::util::DrawGraphCountVisitor dgcv;
        dgcv.traverse( *( collect.getDrawGraph() ) );
        dgcv.dump( std::cout );
    }
#endif

    if( _drawBound )
    {
        // Execute the draw graph a second time, but this time
        // render only the draw graph Node bounds.
        jag::draw::DrawGraphPtr drawGraph( collect.getDrawGraph() );

        _boundProgram->execute( drawInfo );
        drawInfo._current.insert( _boundProgram );
        drawInfo._controlFlags |= jag::draw::DrawInfo::DRAW_BOUND;
        drawGraph->execute( drawInfo );
        drawInfo._controlFlags &= ~jag::draw::DrawInfo::DRAW_BOUND;
    }

    glFlush();

    JAG3D_ERROR_CHECK( "jagmodel display()" );

    return( true );
}

void JagModel::reshape( const int w, const int h )
{
    if( !getStartupCalled() )
        return;

    const jag::draw::jagDrawContextID contextID( jag::draw::ContextSupport::instance()->getActiveContext() );
    _mxCore._data[ contextID ]->setAspect( ( double ) w / ( double ) h );
}


/** \defgroup exampleJagModel The jagmodel Example
This is a simple model loader and viewer.

Command line options:
\li --help,-h Help text
\li --version,-v OpenGL context version. Default: 4.0.
\li --nwin Number of windows. Default: 1.
\li --winsize,-w Window width and height. Default: 800 600.
\li --no-ms Disable multisampling
\li --file,-f Model to load.
\li --bound Rendering draw graph Node bound outlines.
\li --vs Specify a vertex shader.
\li --gs Specify a gwomwtry shader.
\li --fs Specify a fragment shader.
*/
