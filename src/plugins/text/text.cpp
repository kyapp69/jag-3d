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

#include <jagDisk/PluginManager.h>
#include <jagDisk/ReaderWriter.h>

#include "TextDumpSG.h"
#include "TextDumpDG.h"
#include <jagSG/Node.h>
#include <jagDraw/DrawGraph.h>

#include <jagBase/LogMacros.h>
#include <Poco/ClassLibrary.h>
#include <Poco/Path.h>
#include <Poco/String.h>

#include <fstream>
#include <string>


using namespace jagDisk;


/** \addtogroup PluginSupport Plugin Support
*/
/**@{*/

/** \class TextDump
\brief Dump information about scene graphs and draw graphs to text file.
*/
class TextDump : public ReaderWriter
{
public:
    TextDump()
      : ReaderWriter( "text" )
    {}
    virtual ~TextDump()
    {}

    virtual bool supportsExtension( const std::string& extension )
    {
        const std::string allLower( Poco::toLower( extension ) );
        return( ( extension == "txt" ) ||
            ( extension == "text" )
            );
    }

    virtual bool write( const std::string& fileName, const void* data, const Options* options ) const
    {
        std::ofstream oStr( fileName.c_str() );
        if( !oStr )
        {
            // TBD record error
            return( false );
        }

        const bool result( write( oStr, data, options ) );
        oStr.close();

        return( result );
    }
    virtual bool write( std::ostream& oStr, const void* data, const Options* /*options*/ ) const
    {
        const jagSG::Node* node( static_cast< const jagSG::Node* >( data ) );
        if( node != NULL )
        {
            TextDumpSG tdsg( oStr );
            const_cast< jagSG::Node* >( node )->accept( tdsg );
            return( true );
        }

        JAG3D_NOTICE( "Text dump for jagDraw::DrawGraph: not yet implemented." );
#if 0
        const jagDraw::DrawGraph* drawGraph( dynamic_cast< const jagDraw::DrawGraph* >( data ) );
        if( drawGraph != NULL )
        {
            TextDumpDG tddg( oStr );
            drawGraph->accept( tddg );
            return( true );
        }
#endif

        return( false );
    }

protected:
};

/**@}*/


// Register the ShaderRW class with the PluginManager.
// This macro declares a static object initialized when the plugin is loaded.
REGISTER_READERWRITER(
    new TextDump(),   // Create an instance of ImageRW.
    TextDump,         // Class name -- NOT a string.
    "ReaderWriter",   // Base class name as a string.
    "Dump information about scene graphs and draw graphs to text file."  // Description text.
);


// Poco ClassLibrary manifest registration. Add a POCO_EXPORT_CLASS
// for each ReaderWriter class in the plugin.
POCO_BEGIN_MANIFEST( ReaderWriter )
    POCO_EXPORT_CLASS( TextDump )
POCO_END_MANIFEST
