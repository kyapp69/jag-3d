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

#include <jagDraw/Common.h>
#include <jagDraw/PerContextData.h>
#include <jagSG/Common.h>
#include <jagUtil/Shapes.h>
#include <jagDisk/ReadWrite.h>
#include <jagBase/Profile.h>
#include <jagBase/Version.h>
#include <jagBase/Log.h>
#include <jagBase/LogMacros.h>
#include <jagMx/MxCore.h>

#include <boost/chrono/chrono.hpp>
#include <boost/chrono/time_point.hpp>
#include <boost/chrono/duration.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <gmtl/gmtl.h>

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#include <string>
#include <sstream>


using namespace std;
namespace bpo = boost::program_options;


class Transparency : public DemoInterface
{
public:
    Transparency()
      : DemoInterface( "jag.ex.trans" ),
        _fileName( "cow.osg" ),
        _lastTime()
    {
        setContinuousRedraw();
    }
    virtual ~Transparency() {}

    virtual bool parseOptions( boost::program_options::variables_map& vm );

    virtual bool startup( const unsigned int numContexts );
    virtual bool init();
    virtual bool frame( const gmtl::Matrix44d& view, const gmtl::Matrix44d& proj );
    virtual void reshape( const int w, const int h );

protected:
    gmtl::Matrix44d computeView( jagMx::MxCorePtr mxCore, const double angleRad );

    std::string _fileName;

    jagSG::NodePtr _root;

    boost::chrono::high_resolution_clock::time_point _lastTime;
};


DemoInterface* DemoInterface::create( bpo::options_description& desc )
{
    desc.add_options()
        ( "file,f", bpo::value< std::string >(), "Model to load. Default: cow.osg" );

    return( new Transparency );
}

bool Transparency::parseOptions( bpo::variables_map& vm )
{
    if( vm.count( "file" ) > 0 )
        _fileName = vm[ "file" ].as< std::string >();
    return( true );
}


jagSG::NodePtr createPlanesSubgraph( jagDraw::BoundPtr bound )
{
    jagSG::NodePtr planeRoot = jagSG::NodePtr( new jagSG::Node() );

    // Instruct collection visitor to copy data store references
    // into a specific NodeContainer.
    jagSG::SelectContainerCallbackPtr sccp( new jagSG::SelectContainerCallback( 1 ) );
    planeRoot->getCollectionCallbacks().push_back( sccp );

    // Create container to store the plane vertex / normal / texcoord data.
    jagUtil::VNTCVec data;

    // Create the first plane and its data
    float radius( (float)( bound->getRadius() ) );
    gmtl::Point3f corner( radius, -radius, radius );
    gmtl::Vec3f uVec( 0.f, 0.f, -2.f*radius );
    gmtl::Vec3f vVec( 0.f, 2.f*radius, 0.f );
    jagDraw::DrawablePtr plane0( jagUtil::makePlane(
        data, corner, uVec, vVec ) );
    planeRoot->addDrawable( plane0 );

    // Create second plane and its data.
    corner = gmtl::Point3f( -radius, -radius, -radius );
    uVec = gmtl::Vec3f( 0.f, 0.f, 2.f*radius );
    jagDraw::DrawablePtr plane1( jagUtil::makePlane(
        data, corner, uVec, vVec, 4, 4 ) );
    // Must add as separate child node for proper depth sorting.
    jagSG::NodePtr planeChild( jagSG::NodePtr( new jagSG::Node() ) );
    planeChild->addDrawable( plane1 );
    planeRoot->addChild( planeChild );

    // Put the data in an array buffer object, and add it to
    // a VertexArrayObject.
    jagDraw::CommandMapPtr commands( planeRoot->getCommandMap() );
    if( commands == NULL )
    {
        commands = jagDraw::CommandMapPtr( new jagDraw::CommandMap() );
        planeRoot->setCommandMap( commands );
    }
    commands->insert( jagUtil::createVertexArrayObject( data ) );

    return( planeRoot );
}

struct BlendCallback : public jagDraw::DrawNodeContainer::Callback
{
    virtual bool operator()( jagDraw::DrawNodeContainer& nodeContainer,
        jagDraw::DrawInfo& drawInfo )
    {
        glEnable( GL_BLEND );
        nodeContainer.internalExecute( drawInfo );
        glDisable( GL_BLEND );
        return( false );
    }
};


bool Transparency::startup( const unsigned int numContexts )
{
    DemoInterface::startup( numContexts );

    JAG3D_INFO_STATIC( _logName, _fileName );

    if( _fileName.empty() )
    {
        JAG3D_FATAL_STATIC( _logName, "Specify '--file <fileName>' on command line." );
        return( false );
    }


    // Prepare the draw graph.
    jagDraw::DrawGraphPtr drawGraphTemplate( new jagDraw::DrawGraph() );
    drawGraphTemplate->resize( 2 );
    (*drawGraphTemplate)[ 1 ].getCallbacks().push_back(
        jagDraw::DrawNodeContainer::CallbackPtr( new BlendCallback() ) );
    getCollectionVisitor().setDrawGraphTemplate( drawGraphTemplate );


    // Prepare the scene graph.
    _root = jagSG::NodePtr( new jagSG::Node() );

    jagSG::NodePtr model( DemoInterface::readSceneGraphNodeUtil( _fileName ) );
    _root->addChild( model );

    _root->addChild( createPlanesSubgraph( model->getBound() ) );


    jagDraw::ShaderPtr vs( DemoInterface::readShaderUtil( "transparency.vert" ) );
    jagDraw::ShaderPtr fs( DemoInterface::readShaderUtil( "transparency.frag" ) );

    jagDraw::ProgramPtr prog;
    prog = jagDraw::ProgramPtr( new jagDraw::Program );
    prog->attachShader( vs );
    prog->attachShader( fs );

    jagDraw::CommandMapPtr commands( _root->getCommandMap() );
    if( commands == NULL )
    {
        commands = jagDraw::CommandMapPtr( new jagDraw::CommandMap() );
        _root->setCommandMap( commands );
    }
    commands->insert( prog );

    // Test uniform blocks
    jagDraw::UniformBlockPtr ubp( jagDraw::UniformBlockPtr(
        new jagDraw::UniformBlock( "blockTest" ) ) );
    ubp->addUniform( jagDraw::UniformPtr(
        new jagDraw::Uniform( "ambientScene", .2f ) ) );
    ubp->addUniform( jagDraw::UniformPtr(
        new jagDraw::Uniform( "diffuseMat", gmtl::Point3f( 0.f, .7f, 0.9f ) ) ) );
    jagDraw::UniformBlockSetPtr ubsp( jagDraw::UniformBlockSetPtr(
        new jagDraw::UniformBlockSet() ) );
    ubsp->insert( ubp );
    commands->insert( ubsp );

    gmtl::Vec3f lightVec( .5f, .7f, 1.f );
    gmtl::normalize( lightVec );
    jagDraw::UniformSetPtr usp( jagDraw::UniformSetPtr(
        new jagDraw::UniformSet() ) );
    usp->insert( jagDraw::UniformPtr(
        new jagDraw::Uniform( "ecLightDir", lightVec ) ) );
    commands->insert( usp );


    // We keep a different aspect ratio per context (to support different
    // window sizes). Initialize them all to a reasonable default.
    for( unsigned int idx( 0 ); idx<numContexts; ++idx )
    {
        jagMx::MxCorePtr mxCore( new jagMx::MxCore() );
        mxCore->setAspect( 1. );
        mxCore->setFovy( 30. );
        _mxCore._data.push_back( mxCore );
    }


    // Tell all Jag3D objects how many contexts to expect.
    _root->setMaxContexts( numContexts );


    return( true );
}

bool Transparency::init()
{
    glClearColor( 1.f, 1.f, 1.f, 0.f );

    glEnable( GL_DEPTH_TEST );

    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    // Auto-log the version string.
    jagBase::getVersionString();

    // Auto-log the OpenGL version string.
    jagDraw::getOpenGLVersionString();

    return( true );
}

// Don't bother until we have something worth sorting.
#define ENABLE_SORT

bool Transparency::frame( const gmtl::Matrix44d& view, const gmtl::Matrix44d& proj )
{
    if( !getStartupCalled() )
        return( true );

    JAG3D_PROFILE( "frame" );

#ifdef ENABLE_SORT
    jagDraw::Command::CommandTypeVec plist;
    plist.push_back( jagDraw::Command::UniformSet_t );
#endif

    boost::chrono::high_resolution_clock::time_point current( boost::chrono::high_resolution_clock::now() );
    const boost::chrono::high_resolution_clock::duration elapsed( current - _lastTime );
    _lastTime = current;
    const double elapsedSecs( elapsed.count() * 0.000000001 );
    const double rotation = elapsedSecs * gmtl::Math::TWO_PI / 4.; // 2*PI rads (360 degrees) every 4 secs.

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    const jagDraw::jagDrawContextID contextID( jagDraw::ContextSupport::instance()->getActiveContext() );
    jagDraw::DrawInfo& drawInfo( getDrawInfo( contextID ) );

    jagMx::MxCorePtr mxCore( _mxCore._data[ contextID ] );

    jagSG::CollectionVisitor& collect( getCollectionVisitor() );
    collect.reset();

    jagDraw::DrawGraphPtr drawGraph;
    {
        JAG3D_PROFILE( "Collection" );

        // Set view and projection to define the collection frustum.
        const gmtl::Matrix44d viewMatrix( computeView( mxCore, rotation ) );
        collect.setViewProj( viewMatrix, mxCore->computeProjection( .1, 500. ) );

        {
            JAG3D_PROFILE( "Collection traverse" );
            // Collect a draw graph.
            _root->accept( collect );
        }
        drawGraph = collect.getDrawGraph();

        // Set view and projection to use for drawing. Create projection using
        // the computed near and far planes.
        double minNear, maxFar;
        collect.getNearFar( minNear, maxFar );
        drawGraph->setViewProj( viewMatrix, mxCore->computeProjection( minNear, maxFar ) );

#ifdef ENABLE_SORT
        {
            JAG3D_PROFILE( "Collection sort" );
            {
                // Sort by commands.
                jagDraw::DrawNodeContainer& nc( (*drawGraph)[ 0 ] );
                std::sort( nc.begin(), nc.end(), jagDraw::DrawNodeCommandSorter( plist ) );
            }
            {
                // Sort by descending eye coord z distance.
                jagDraw::DrawNodeContainer& nc( (*drawGraph)[ 1 ] );
                jagDraw::DrawNodeDistanceSorter distanceSorter;
                std::sort( nc.begin(), nc.end(), distanceSorter );
            }
        }
#endif
    }

    {
        JAG3D_PROFILE( "Render" );

        // Execute the draw graph.
        drawGraph->execute( drawInfo );
    }

    glFlush();

    JAG3D_ERROR_CHECK( "jagmodel display()" );

    return( true );
}

void Transparency::reshape( const int w, const int h )
{
    if( !getStartupCalled() )
        return;

    const jagDraw::jagDrawContextID contextID( jagDraw::ContextSupport::instance()->getActiveContext() );
    _mxCore._data[ contextID ]->setAspect( ( double ) w / ( double ) h );
}

gmtl::Matrix44d Transparency::computeView( jagMx::MxCorePtr mxCore, const double angleRad )
{
    mxCore->rotateOrbit( angleRad, gmtl::Vec3d( 0., 0., 1. ) );
    mxCore->lookAtAndFit( _root->getBound()->asSphere() );
    return( mxCore->getInverseMatrix() );
}
