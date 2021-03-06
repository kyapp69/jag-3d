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

#include "assimp2jag.h"

#include <jag/disk/ReaderWriter.h>
#include <jag/disk/ReadWrite.h>
#include <jag/sg/Node.h>
#include <jag/draw/Drawable.h>
#include <jag/draw/VertexAttrib.h>
#include <jag/draw/Texture.h>
#include <jag/draw/Uniform.h>
#include <jag/base/LogMacros.h>

#include <assimp/scene.h>

#include <sstream>


Assimp2Jag::Assimp2Jag( const aiScene* aiScene, const jag::disk::Options* options )
  : _logName( "jag.disk.rw.ai.ai2jag" ),
    _aiScene( *aiScene ),
    _vertexAttribName( "vertex" ),
    _normalAttribName( "normal" ),
    _texCoordAttribName( "texcoord" )
{
    JAG3D_TRACE_STATIC( _logName, "Assimp2Jag::Assimp2Jag" );
}
Assimp2Jag::~Assimp2Jag()
{
}

jag::sg::NodePtr Assimp2Jag::getJagScene()
{
    JAG3D_TRACE_STATIC( _logName, "Assimp2Jag::getJagScene" );

    if( _jagScene == NULL )
    {
        initData();
        _jagScene = traverse( _aiScene.mRootNode );
    }
    return( _jagScene );
}

void Assimp2Jag::initData()
{
    JAG3D_TRACE_STATIC( _logName, "Assimp2Jag::initData" );

    // Convert all aiScene meshes to JAG vertex array and Drawable objects.
    // The _vao and _draw member arrays will later be indexed by the aiNode's
    // mesh index.
    _vao.resize( _aiScene.mNumMeshes );
    _draw.resize( _aiScene.mNumMeshes );
    _materials.resize( _aiScene.mNumMeshes );
    for( unsigned int idx=0; idx < _aiScene.mNumMeshes; idx++ )
    {
        JAG3D_TRACE_STATIC( _logName, "\tmesh" );

        const struct aiMesh* currentMesh( _aiScene.mMeshes[ idx ] );

        jag::draw::VertexArrayObjectPtr& vao( _vao[ idx ] );
        vao.reset( new jag::draw::VertexArrayObject() );

        jag::draw::DrawablePtr& draw( _draw[ idx ] );
        draw.reset( new jag::draw::Drawable() );

        // Save the material index
        _materials[ idx ] = currentMesh->mMaterialIndex;

        // Vertices
        {
            ArrayInfo info( get3fData( currentMesh->mVertices, currentMesh->mNumVertices ) );
            jag::draw::BufferObjectPtr bop( new jag::draw::BufferObject( GL_ARRAY_BUFFER, info._buffer ) );
            vao->addVertexArrayCommand( bop, jag::draw::VertexArrayObject::Vertex );

            jag::draw::VertexAttribPtr attrib( new jag::draw::VertexAttrib(
                _vertexAttribName, info._componentsPerElement, info._type, GL_FALSE, 0, 0 ) );
            vao->addVertexArrayCommand( attrib, jag::draw::VertexArrayObject::Vertex );

            std::ostringstream ostr;
            ostr << info._numElements;
            JAG3D_INFO_STATIC( _logName, "Vertex array size (#verts): " + ostr.str() );
        }

        // Normals
        if( currentMesh->mNormals != NULL )
        {
            ArrayInfo info( get3fData( currentMesh->mNormals, currentMesh->mNumVertices ) );
            jag::draw::BufferObjectPtr bop( new jag::draw::BufferObject( GL_ARRAY_BUFFER, info._buffer ) );
            vao->addVertexArrayCommand( bop, jag::draw::VertexArrayObject::Vertex );

            jag::draw::VertexAttribPtr attrib( new jag::draw::VertexAttrib(
                _normalAttribName, info._componentsPerElement, info._type, GL_FALSE, 0, 0 ) );
            vao->addVertexArrayCommand( attrib, jag::draw::VertexArrayObject::Normal );
        }

        // Tangents
        // Bitangents
        // Colors

        // Texcoords
        for( unsigned int tcIdx=0; tcIdx < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++tcIdx )
        {
            aiVector3D* aitc( currentMesh->mTextureCoords[ tcIdx ] );
            if( aitc == NULL )
                continue;

            {
                std::ostringstream ostr;
                ostr << tcIdx;
                JAG3D_INFO_STATIC( _logName, "\tTexCoords for unit " +
                    std::string( ostr.str() ) );
            }

            ArrayInfo info( get3fData( aitc, currentMesh->mNumVertices ) );
            jag::draw::BufferObjectPtr bop( new jag::draw::BufferObject( GL_ARRAY_BUFFER, info._buffer ) );
            vao->addVertexArrayCommand( bop, jag::draw::VertexArrayObject::TexCoord );

            std::ostringstream ostr;
            ostr << _texCoordAttribName << tcIdx;
            jag::draw::VertexAttribPtr attrib( new jag::draw::VertexAttrib(
                ostr.str(), info._componentsPerElement, info._type, GL_FALSE, 0, 0 ) );
            vao->addVertexArrayCommand( attrib, jag::draw::VertexArrayObject::Normal );
        }

        // Faces
        ArrayInfo info( getUIntFaceData( currentMesh ) );
        jag::draw::BufferObjectPtr bop( new jag::draw::BufferObject( GL_ELEMENT_ARRAY_BUFFER, info._buffer ) );

        GLenum primType( asGLPrimitiveType( currentMesh->mPrimitiveTypes ) );
        jag::draw::DrawElementsPtr drawcom( new jag::draw::DrawElements(
            primType, info._numElements, GL_UNSIGNED_INT, NULL, bop ) );
        draw->addDrawCommand( drawcom );

        {
            std::ostringstream ostr;
            ostr << std::hex << primType;
            JAG3D_INFO_STATIC( _logName, "\tPrimitive type 0x" +
                std::string( ostr.str() ) );
        }

        // Bones
        // Name
        // Animations
    }

    // Convert aiScene meterials
    {
        std::ostringstream ostr;
        ostr << "Num Materials: " << _aiScene.mNumMaterials;
        JAG3D_INFO_STATIC( _logName, ostr.str() );
    }

    _texInfo.resize( _aiScene.mNumMaterials );
    for( unsigned int idx=0; idx < _aiScene.mNumMaterials; idx++ )
    {
        aiMaterial* material( _aiScene.mMaterials[ idx ] );
        TexInfoVec& textures( _texInfo[ idx ] );

        for( unsigned int texType=0; texType < aiTextureType_UNKNOWN; texType++ )
        {
            const unsigned int num( material->GetTextureCount( aiTextureType( texType ) ) );
            if( num > 0 )
            {
                std::ostringstream ostr;
                ostr << "\tFound " << num << " textures of type " << texType;
                JAG3D_INFO_STATIC( _logName, std::string( ostr.str() ) );
            }
        }

        aiString path;
        const unsigned int texCount( material->GetTextureCount( aiTextureType_DIFFUSE ) );
        for( unsigned int nTex=0; nTex < texCount; nTex++ )
        {
            if( material->GetTexture( aiTextureType_DIFFUSE, nTex, &path,
                    NULL, NULL, NULL, NULL ) != aiReturn_SUCCESS )
                continue;

            std::string texFileName( path.data );
            std::ostringstream ostr;
            ostr << nTex;
            JAG3D_INFO_STATIC( _logName, "\tFound texture " +
                std::string( ostr.str() ) + ": " +
                texFileName );

            jag::draw::TexturePtr tex( loadTexture( texFileName ) );
            JAG3D_INFO_STATIC( _logName, "\tReturned texture" );
            if( tex == NULL )
                continue;

            TexInfo texInfo( nTex, tex );
            textures.push_back( texInfo );
        }
    }

    // Convert all embedded aiScene textures to JAG textures
    if( _aiScene.mNumTextures > 0 )
    {
        JAG3D_WARNING_STATIC( _logName, "Unprocessed embedded textures." );
    }
    /*
    for( unsigned int idx=0; idx < _aiScene.mNumTextures; idx++ )
    {
    }
    */
}

jag::sg::NodePtr Assimp2Jag::traverse( const aiNode* aiNode )
{
    JAG3D_TRACE_STATIC( _logName, "Assimp2Jag::traverse" );

    jag::sg::NodePtr parent( new jag::sg::Node() );

    for( unsigned int idx=0; idx < aiNode->mNumMeshes; idx++ )
    {
        const unsigned int meshIdx( aiNode->mMeshes[ idx ] );

        jag::sg::NodePtr node( new jag::sg::Node() );
        parent->addChild( node );
        jag::draw::CommandMapPtr& commands( node->getOrCreateCommandMap() );

        node->setTransform( asGMTLMatrix( aiNode->mTransformation ) );

        commands->insert( _vao[ meshIdx ] );
        node->addDrawable( _draw[ meshIdx ] );

        TexInfoVec& textures( _texInfo[ _materials[ meshIdx ] ] );

        std::ostringstream ostr;
        ostr << "meshIdx: " << meshIdx << ", mat index: " << _materials[ meshIdx ]
        << ", textures: " << textures.size();
        JAG3D_DEBUG_STATIC( _logName, ostr.str() );

        for( unsigned int tIdx=0; tIdx < textures.size(); tIdx++ )
        {
            JAG3D_INFO_STATIC( _logName, "Processing a texture" );
            TexInfo& texInfo( textures[ tIdx ] );

            jag::draw::TextureSetPtr texSet( new jag::draw::TextureSet() );
            (*texSet)[ GL_TEXTURE0 + texInfo._index ] = texInfo._tex;
            commands->insert( texSet );

            jag::draw::UniformSetPtr usp( new jag::draw::UniformSet() );
            jag::draw::UniformPtr uni( new jag::draw::Uniform(
                "texture0", GL_SAMPLER_2D, texInfo._index ) );
            usp->insert( uni );
            commands->insert( usp );
        }
    }

    for( unsigned int idx=0; idx < aiNode->mNumChildren; idx++ )
        parent->addChild( traverse( aiNode->mChildren[ idx ] ) );

    return( parent );
}


Assimp2Jag::ArrayInfo Assimp2Jag::get3fData( const aiVector3D* data, const unsigned int size ) const
{
    ArrayInfo info;

    info._type = GL_FLOAT;
    info._numElements = size;
    info._componentsPerElement = 3;

    jag::base::Point3fVec out;
    out.resize( size );
    unsigned int idx;
    for( idx=0; idx<size; idx++ )
    {
        const aiVector3D& v( data[ idx ] );
        gmtl::Point3f& p( out[ idx ] );
        p[ 0 ] = v[ 0 ];
        p[ 1 ] = v[ 1 ];
        p[ 2 ] = v[ 2 ];
    }

    jag::base::BufferPtr bp( new jag::base::Buffer( size * sizeof( gmtl::Point3f ), (void*)&( out[0] ) ) );
    info._buffer = bp;
    return( info );
}
Assimp2Jag::ArrayInfo Assimp2Jag::getUIntFaceData( const struct aiMesh* mesh ) const
{
    ArrayInfo info;
    info._type = GL_UNSIGNED_INT;
    info._componentsPerElement = 1;
    {
        unsigned int count( 0 );
        for( unsigned int faceIdx=0; faceIdx < mesh->mNumFaces; ++faceIdx )
        {
            const struct aiFace& face = mesh->mFaces[ faceIdx ];
            count += face.mNumIndices;
        }
        info._numElements = count;
    }

    {
        std::ostringstream ostr;
        ostr << "getUIntFaceData: mNumFaces " << mesh->mNumFaces <<
            ", total indices " << info._numElements;
        JAG3D_INFO_STATIC( _logName, ostr.str() );
    }

    jag::draw::GLuintVec out;
    out.resize( info._numElements );

    for( unsigned int faceIdx=0, count=0;
        faceIdx < mesh->mNumFaces; ++faceIdx )
    {
        const struct aiFace& face = mesh->mFaces[ faceIdx ];
        for( unsigned int idx=0; idx < face.mNumIndices; ++idx )
            out[ count++ ] = face.mIndices[ idx ];
    }

    jag::base::BufferPtr bp( new jag::base::Buffer( info._numElements * sizeof( GLuint ), (void*)&out[0] ) );
    info._buffer = bp;
    return( info );
}
gmtl::Matrix44d Assimp2Jag::asGMTLMatrix( aiMatrix4x4 m )
{
    m.Transpose();
    gmtl::Matrix44d retVal;
    retVal.set( (double)m.a1, (double)m.a2, (double)m.a3, (double)m.a4,
        (double)m.b1, (double)m.b2, (double)m.b3, (double)m.b4,
        (double)m.c1, (double)m.c2, (double)m.c3, (double)m.c4,
        (double)m.d1, (double)m.d2, (double)m.d3, (double)m.d4 );
    return( retVal );
}
GLenum Assimp2Jag::asGLPrimitiveType( const unsigned int aiPrimitiveType )
{
    switch( aiPrimitiveType )
    {
    case aiPrimitiveType_POINT:
        return( GL_POINTS );
        break;
    case aiPrimitiveType_LINE:
        return( GL_LINES );
        break;
    case aiPrimitiveType_TRIANGLE:
        return( GL_TRIANGLES );
        break;
    case aiPrimitiveType_POLYGON:
    default:
        std::cerr << "Assimp2Jag::asGLPrimitiveType: Unsupported type " << std::hex << aiPrimitiveType << std::dec << std::endl;
        return( GL_POINTS );
        break;
    }
}
jag::draw::TexturePtr Assimp2Jag::loadTexture( const std::string& fileName )
{
    jag::draw::ImagePtr image;
    boost::any anyTemp( jag::disk::read( fileName ) );
    try {
        image = boost::any_cast< jag::draw::ImagePtr >( anyTemp );
    }
    catch( boost::bad_any_cast bac )
    {
        JAG3D_ERROR_STATIC( _logName, std::string( bac.what() ) );
        return( jag::draw::TexturePtr( (jag::draw::Texture*)NULL ) );
    }

    jag::draw::TexturePtr tex( new jag::draw::Texture( GL_TEXTURE_2D, image,
        jag::draw::SamplerPtr( new jag::draw::Sampler() ) ) );
    tex->getSampler()->getSamplerState()->_minFilter = GL_LINEAR;

    return( tex );
}
