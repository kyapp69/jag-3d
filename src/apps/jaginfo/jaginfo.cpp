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

#include <jagDraw/Common.h>
#include <jagSG/Common.h>
#include <jagUtil/DrawGraphCountVisitor.h>
#include <jagUtil/BufferAggregationVisitor.h>
#include <jagDisk/ReadWrite.h>
#include <jagBase/Log.h>
#include <jagBase/LogMacros.h>

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

#include <string>
#include <sstream>
#include <set>

namespace bpo = boost::program_options;

std::string logstr( "jag.app.info" );



class SceneGraphCountVisitor : public jagSG::VisitorBase
{
public:
    SceneGraphCountVisitor();
    ~SceneGraphCountVisitor();

    void reset();
    void dump( std::ostream& ostr );

    virtual void visit( jagSG::Node& node );

    unsigned int _nodes, _uNodes;
    unsigned int _commands, _uCommands;
    unsigned int _drawables, _uDrawables;

protected:
    std::set< jagSG::NodePtr > _nodeSet;
    std::set< jagDraw::CommandMapPtr > _commandSet;
    std::set< jagDraw::DrawablePtr > _drawableSet;
};

SceneGraphCountVisitor::SceneGraphCountVisitor()
    : jagSG::VisitorBase( "count" )
{
    reset();
}
SceneGraphCountVisitor::~SceneGraphCountVisitor()
{
}

void SceneGraphCountVisitor::reset()
{
    _nodes = _uNodes = 0;
    _commands = _uCommands = 0;
    _drawables = _uDrawables = 0;
    _nodeSet.clear();
    _commandSet.clear();
    _drawableSet.clear();
}
void SceneGraphCountVisitor::dump( std::ostream& ostr )
{
    ostr << "            \tTotal\tUnique" << std::endl;
    ostr << "            \t-----\t------" << std::endl;
    ostr << "      Nodes:\t" << _nodes << "\t" << _uNodes << std::endl;
    ostr << "CommandMaps:\t" << _commands << "\t" << _uCommands << std::endl;
    ostr << "  Drawables:\t" << _drawables << "\t" << _uDrawables << std::endl;
}

void SceneGraphCountVisitor::visit( jagSG::Node& node )
{
    ++_nodes;
    jagSG::NodePtr np( node.shared_from_this() );
    if( _nodeSet.find( np ) == _nodeSet.end() )
    {
        ++_uNodes;
        _nodeSet.insert( np );
    }

    jagDraw::CommandMapPtr cp( node.getCommandMap() );
    if( cp != NULL )
    {
        ++_commands;
        if( _commandSet.find( cp ) == _commandSet.end() )
        {
            ++_uCommands;
            _commandSet.insert( cp );
        }
    }

    for( unsigned int idx=0; idx < node.getNumDrawables(); ++idx )
    {
        ++_drawables;
        const jagDraw::DrawablePtr dp( node.getDrawable( idx ) );
        if( _drawableSet.find( dp ) == _drawableSet.end() )
        {
            ++_uDrawables;
            _drawableSet.insert( dp );
        }
    }

    node.traverse( *this );
}



int main( int argc, char** argv )
{
    jagBase::Log::instance();

    bpo::options_description desc( "Options" );
    desc.add_options()
        ( "help,h", "Help text" )
        ( "file,f", bpo::value< std::string >(), "File to load." )
    ;

    bpo::variables_map vm;
    bpo::store( bpo::parse_command_line( argc, argv, desc ), vm );
    bpo::notify( vm );

    if( vm.count( "help" ) > 0 )
    {
        std::cout << desc << std::endl;
        return( 1 );
    }

    std::string fileName;
    if( vm.count( "file" ) > 0 )
    {
        fileName = vm[ "file" ].as< std::string >();
        JAG3D_INFO_STATIC( logstr, "Loading \""+fileName+"\"" );
    }
    else
    {
        JAG3D_FATAL_STATIC( logstr, "Must specify \"-f <file>\"." );
        std::cerr << desc << std::endl;
        return( 1 );
    }


    boost::any anyRoot( jagDisk::read( fileName ) );
    jagSG::NodePtr root;
    try {
        root = boost::any_cast< jagSG::NodePtr >( anyRoot );
    } catch( boost::bad_any_cast bac ) { bac.what(); }
    if( root == NULL )
    {
        JAG3D_FATAL_STATIC( logstr, "Can't load \"" + fileName + "\"." );
        return( false );
    }


    {
        jagUtil::BufferAggregationVisitor bav( root );
    }

    {
        SceneGraphCountVisitor sgcv;
        root->accept( sgcv );
        sgcv.dump( std::cout );
    }

    jagSG::CollectionVisitor collect;
    collect.setNearFarOps( jagSG::CollectionVisitor::None );
    gmtl::Matrix44d proj;
    gmtl::setOrtho( proj, -DBL_MAX, DBL_MAX, DBL_MAX, -DBL_MAX, DBL_MAX, -DBL_MAX );
    collect.setViewProj( gmtl::MAT_IDENTITY44D, proj );
    root->accept( collect );

    jagDraw::DrawGraphPtr drawGraph( collect.getDrawGraph() );

    {
        jagDraw::DrawablePrep::CommandTypeVec plist;
        plist.push_back( jagDraw::DrawablePrep::UniformBlockSet_t );
        BOOST_FOREACH( jagDraw::NodeContainer& nc, *drawGraph )
        {
            std::sort( nc.begin(), nc.end(), jagDraw::DrawNodeCommandSorter( plist ) );
        }
    }

    {
        jagUtil::DrawGraphCountVisitor dgcv;
        dgcv.traverse( *drawGraph );
        dgcv.dump( std::cout );
    }

    return( 0 );
}


/** \defgroup appJagInfo The jaginfo Application
This application displays information about the scene graph and draw graph
for a loaded model.

Example output:
\code
> jaginfo -f <some-model>
                Total   Unique
                -----   ------
      Nodes:    113     113
CommandMaps:    29      29
  Drawables:    28      28
                Total
                -----
 Containers:    1
      Nodes:    28
  Drawables:    28
  CM deltas:    7
\endcode
The first section of the output shows counts for the scene graph.

The second section shows counts for the draw graph, including CM deltas
(non-empty changes in the CommandMap during the draw traversal, i.e.,
OpenGL state changes).

Command line options:
\li --help,-h Help text
\li --file,-f Model to load.
*/
