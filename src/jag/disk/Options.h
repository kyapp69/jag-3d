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

#ifndef __JAG_DISK_OPTIONS_H__
#define __JAG_DISK_OPTIONS_H__ 1


#include <jag/disk/Export.h>
#include <Poco/Path.h>

#include <boost/any.hpp>
#include <jag/base/ptr.h>
#include <string>
#include <vector>
#include <map>


namespace jag {
namespace disk {


/** \addtogroup PluginSupport Plugin Support
*/
/**@{*/


/** \class Options Options.H <jag/disk/Options.H>
\brief
\details
*/
class JAGDISK_EXPORT Options
{
public:
    typedef enum {
        NO_PATHS                    = 0,
        USE_CURRENT_DIRECTORY       = ( 1 << 0x0 ),
        USE_JAG3D_DATA_PATH_ENV_VAR = ( 1 << 0x1 )
    } InitFlags;

    /** \brief Options class constructor
    \details Constructs an instance of the Options object.
    \param initFlags Specifies an initial list of directories to use when
    searching for data files to load. By default, Options loads its directory
    search path list \c _paths from the value of the JAG3D_DATA_PATH
    environment variable. If this variable is not set, the \c _paths will be
    empty and only the current working directory will be searched.

    To separate multiple directories in JAG3D_DATA_PATH, use the platform-
    specific path separator (";" on Windows, ":" on Linux, etc).
    \envvar JAG3D_DATA_PATH */
    Options( const int initFlags=USE_JAG3D_DATA_PATH_ENV_VAR );
    Options( const Options& rhs );
    ~Options();

    /** \brief Add an individual search path. */
    void addPath( const std::string& path );
    /** \brief Add multiple search paths separated by the platform-specific directory separator. */
    void addPaths( const std::string& paths );
    /** \brief Clear the list of directory search paths. */
    void clearPaths();
    /** \brief Get a list of all directory search paths. */
    const Poco::Path::StringVec& getPaths() const;

    /** \brief Search the directory path list for \c fileName and return the full path. */
    Poco::Path findFile( const std::string& fileName ) const;


    typedef std::map< std::string, boost::any > AnyMap;

    /** \class OptionsEasyInit Options.H <jag/disk/Options.H>
    */
    class OptionsEasyInit
    {
    public:
        OptionsEasyInit( AnyMap& anyMap )
          : _anyMap( anyMap )
        {}
        ~OptionsEasyInit() {}

        OptionsEasyInit& operator()( const std::string& name, const boost::any& value )
        {
            _anyMap[ name ] = value;
            return( *this );
        }

    protected:
        AnyMap& _anyMap;
    };

    OptionsEasyInit addOptions()
    {
        return( OptionsEasyInit( _anyMap ) );
    }
    bool hasOption( const std::string& name ) const
    {
        return( _anyMap.find( name ) != _anyMap.end() );
    }
    const boost::any getOption( const std::string& name ) const
    {
        AnyMap::const_iterator it( _anyMap.find( name ) );
        if( it != _anyMap.end() )
            return( it->second );
        else
            return( boost::any() );
    }

protected:
    Poco::Path::StringVec _paths;

    AnyMap _anyMap;
};

typedef jag::base::ptr< jag::disk::Options >::shared_ptr OptionsPtr;
typedef jag::base::ptr< const jag::disk::Options >::shared_ptr ConstOptionsPtr;
typedef std::vector< OptionsPtr > OptionsVec;


/**@}*/


// namespace jag::disk::
}
}


// __JAG_DISK_OPTIONS_H__
#endif
