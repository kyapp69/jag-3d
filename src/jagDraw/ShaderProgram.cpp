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

#include <jagDraw/ShaderProgram.h>
#include <jagDraw/PlatformOpenGL.h>
#include <stdio.h>
#include <iostream>


namespace jagDraw {



void ShaderProgram::printInfoLog()
{
    GLsizei bufLen = 0;       
    glGetProgramiv(_id, GL_INFO_LOG_LENGTH, &bufLen );
    if( bufLen > 1 )
    {
        std::cerr << "\n==========  Shader Information Log ============= " << std::endl;
        GLsizei strLen = 0;        // strlen GL actually wrote to buffer
        char* infoLog = new char[bufLen];
        glGetProgramInfoLog( _id, bufLen, &strLen, infoLog );
        if( strLen > 0 )
            std::cerr << infoLog << std::endl;
        std::cerr << "==================================================\n" << std::endl;
        delete [] infoLog;
    }
}

ShaderProgram::ShaderProgram():
    //DrawableAttribute( ShaderProgram_t ),
    m_initialized(false),
    m_linked(false),
    _id(0)
{
    m_stringToNameMap["ii_ModelViewProjectionMatrix"] = ModelViewProjectionMatrix;
    m_stringToNameMap["ii_NormalMatrix"] = NormalMatrix;
    m_nameToLocationMap[ModelViewProjectionMatrix] = -1;
}

ShaderProgram::~ShaderProgram()
{
    if( _id != 0 )
        glDeleteProgram( _id );
}

void ShaderProgram::attachShader( ShaderPtr shader )
{
    m_shaders.push_back( shader );
}

void ShaderProgram::applyNone()
{
    glUseProgram(0);
}

//void ShaderProgram::apply( DrawContext &ctx )
void ShaderProgram::apply( )
{
    if( !m_linked )
        link();

    if( _id == 0 )
        return;

    glUseProgram(_id);
}

GLuint ShaderProgram::getUniformLocation( const std::string &name )
{
    std::map<std::string, LocationTypePair>::iterator f =  m_nameToLocationTypeMap.find( name );
    if( f != m_nameToLocationTypeMap.end() )
        return f->second.loc;
    return -1;
}

void ShaderProgram::setUniformValue( UniformLocationName name, const UniformValue &value )
{
    GLint loc = m_nameToLocationMap[name];
    if( loc != -1 )
        value.apply( loc );
}

void ShaderProgram::setUniformValue( const std::string &name, const UniformValue &value )
{
    GLint loc = getUniformLocation(name);
    if( loc != -1 )
        value.apply(loc);
}


void ShaderProgram::bindAttribLocation( GLuint index, const std::string &name )
{
    if( !m_initialized )
        p_init();

    glBindAttribLocation( _id, index, name.c_str() );
}

GLuint ShaderProgram::getAttribLocation( const std::string &name )
{
    if( !m_initialized )
        p_init();

    return glGetAttribLocation( _id, name.c_str() );
}

void ShaderProgram::setParameter( GLenum pname, GLint value )
{
    if( !m_initialized )
        p_init();

#if 0 // these are potentially not supported if not defined correctly.

    if( pname == GL_GEOMETRY_SHADER ) pname = GL_GEOMETRY_SHADER_ARB;
    else if( pname == GL_GEOMETRY_VERTICES_OUT ) pname = GL_GEOMETRY_VERTICES_OUT_ARB;
    else if( pname == GL_GEOMETRY_INPUT_TYPE ) pname = GL_GEOMETRY_INPUT_TYPE_ARB;
    else if( pname == GL_GEOMETRY_OUTPUT_TYPE ) pname = GL_GEOMETRY_OUTPUT_TYPE_ARB;

#endif

    glProgramParameteri(_id, pname, value );
}

void ShaderProgram::get( GLenum pname, GLint *params )
{
    glGetProgramiv( _id, pname, params );
}

GLint ShaderProgram::getId()
{
    if( !m_initialized )
        p_init();

    return _id;
}

bool ShaderProgram::link()
{
    if( !m_initialized )
        p_init();

    for( std::vector< ShaderPtr >::iterator s = m_shaders.begin(); s != m_shaders.end(); s++ )
    {
        GLint shader = (*s)->getId();
        if( shader != 0 )
            glAttachShader( _id, shader );
    }
    m_shaders.clear();

    GLint status;
    glLinkProgram(_id);
    glGetProgramiv( _id, GL_LINK_STATUS, &status );
    if( status != GL_TRUE )
    {
        printInfoLog();
        glDeleteProgram( _id );
        _id = 0;
    }
    m_linked = (status == GL_TRUE);

    if( m_linked == true )
    {
        GLint n;
        glUseProgram(_id);
        glGetProgramiv( _id, GL_ACTIVE_UNIFORMS, &n );
printf("Active Uniforms:\n" );
        for( GLint i = 0; i < n; i++ )
        {
            GLenum type;
            std::string string;
            getActiveUniform( i, string, type );
            GLint location = glGetUniformLocation( _id, string.c_str() );
printf("......... %d: %s (%d)\n", i, string.c_str(), location );
            m_nameToLocationTypeMap[string] = LocationTypePair( location, type );

            std::map< std::string,  UniformLocationName >::iterator p = m_stringToNameMap.find( string ); 
            if( p != m_stringToNameMap.end() )
            {
                UniformLocationName name = p->second;
                m_nameToLocationMap[name] = location;
            }
        }

        glGetProgramiv( _id, GL_ACTIVE_ATTRIBUTES, &n );
printf("\nActive Attributes: (%d)\n", n );
        for( GLint i = 0; i < n; i++ )
        {
            GLenum type;
            std::string name;

            char namebuff[256];
            GLsizei len;
            GLsizei isize = sizeof(namebuff);
            GLint osize;
            //getActiveAttrib( i, name, type );
            glGetActiveAttrib(  _id, i, isize, &len, &osize, &type, namebuff );
            name = std::string(namebuff );
            GLint loc = glGetAttribLocation( _id, name.c_str() );
printf("......... %d: %s (%d)\n", i, name.c_str(), loc );
            GLint location = glGetUniformLocation( _id, name.c_str() );
            m_nameToLocationTypeMap[name] = LocationTypePair( location, type );
        }
        glUseProgram(0);
    }

    return m_linked;
}

bool ShaderProgram::validate()
{
    if( !m_initialized )
        p_init();

    GLint status;
    glValidateProgram(_id);
    glGetProgramiv( _id, GL_VALIDATE_STATUS, &status );
    if( status != GL_TRUE )
    {
        printInfoLog();
        glDeleteProgram( _id );
        _id = 0;
    }
    return (status == GL_TRUE);
}


void ShaderProgram::getActiveUniform( GLuint index, std::string &name, GLenum &type )
{
    char namebuff[256];
    GLsizei len;
    GLsizei isize = sizeof(namebuff);
    GLint osize;
    glGetActiveUniform( _id, index, isize, &len, &osize, &type, namebuff );
    name = std::string(namebuff );
}


void ShaderProgram::p_init()
{
    if( m_initialized )
        return;

    _id = glCreateProgram();

    m_initialized = true;

}

bool ShaderProgram::p_findLocationTypePair( const std::string &name, ShaderProgram::LocationTypePair &lp )
{
    std::map<std::string, LocationTypePair>::iterator f = m_nameToLocationTypeMap.find(name);

    if( f == m_nameToLocationTypeMap.end() )
        return false;

    lp = f->second;
    return true;
}

void ShaderProgram::fromSourceFiles( const std::string &vertexShaderFile,
                              const std::string &fragmentShaderFile )
{
    jagDraw::ShaderPtr vshader(new jagDraw::Shader(GL_VERTEX_SHADER ));
    vshader->addSourceFile( vertexShaderFile );
    attachShader( vshader );

    jagDraw::ShaderPtr fshader(new jagDraw::Shader(GL_FRAGMENT_SHADER ));
    fshader->addSourceFile( fragmentShaderFile );
    attachShader( fshader );
}

void ShaderProgram::fromSourceStrings( const std::string &vertexShaderString,
                               const std::string &fragmentShaderString )
{
    jagDraw::ShaderPtr vshader(new jagDraw::Shader(GL_VERTEX_SHADER ));
    vshader->addSourceString( vertexShaderString );
    attachShader( vshader );

    jagDraw::ShaderPtr fshader(new jagDraw::Shader(GL_FRAGMENT_SHADER ));
    fshader->addSourceString( fragmentShaderString );
    attachShader( fshader );

}

void ShaderProgram::fromSourceFileList( const SourceList &l )
{
    for( SourceList::const_iterator p = l.begin(); p != l.end(); p++ )
    {
        jagDraw::ShaderPtr shader(new jagDraw::Shader( p->first ));
        shader->addSourceFile( p->second );
        attachShader( shader );
    }
}

void ShaderProgram::fromSourceStringList( const SourceList &l )
{
    for( SourceList::const_iterator p = l.begin(); p != l.end(); p++ )
    {
        jagDraw::ShaderPtr shader(new jagDraw::Shader( p->first ));
        shader->addSourceString( p->second );
        attachShader( shader );
    }
}


// jagDraw
}
