set( _libName jagDraw )

if( BUILD_SHARED_LIBS )
    add_definitions( -DJAGDRAW_LIBRARY )
endif()


set( _jagDrawHeaderFiles
    Export.h
    Bound.h
    BoundOwner.h
    BufferObject.h
    Command.h
    CommandMap.h
    CommandMapPtr.h
    CommandNode.h
    CommandNodePtr.h
    Common.h
    ContextSupport.h
    Drawable.h
    DrawCommand.h
    DrawGraph.h
    DrawInfo.h
    DrawNode.h
    DrawNodeContainer.h
    Error.h
    Framebuffer.h
    FramebufferAttachable.h
    Image.h
    ObjectID.h
    PerContextData.h
    PixelStore.h
    PlatformOpenGL.h
    Program.h
    Sampler.h
    Shader.h
    Texture.h
    TransformCallback.h
    types.h
    Uniform.h
    UniformBlock.h
    Version.h
    VertexArrayCommand.h
    VertexArrayObject.h
    VertexAttrib.h
    VertexAttribContainer.h
    TriangleSurfer.h
    Visitor.h
)

set( _jagDrawSourceFiles
    BufferObject.cpp
    CommandNode.cpp
    ContextSupport.cpp
    Drawable.cpp
    DrawGraph.cpp
    DrawInfo.cpp
    DrawNode.cpp
    DrawNodeContainer.cpp
    Error.cpp
    Framebuffer.cpp
    Image.cpp
    PixelStore.cpp
    Program.cpp
    Sampler.cpp
    Shader.cpp
    Texture.cpp
    TransformCallback.cpp
    Uniform.cpp
    UniformBlock.cpp
    Version.cpp
    VertexArrayObject.cpp
)

unset( _jagDrawGl3wHeaderFiles )
unset( _jagDrawGl3wSourceFiles )
if( JAG3D_USE_GL3W )
    set( _jagDrawGl3wHeaderFiles
        ${PROJECT_SOURCE_DIR}/src/jag/draw/gl3w.h
        glExtensions.h
        ContextSupportGl3w.h
    )
    set( _jagDrawGl3wSourceFiles
        ${PROJECT_SOURCE_DIR}/src/jag/draw/gl3w.c
        ContextSupportGl3w.cpp
    )
endif()

_addLibrary( ${_libName}
    ${_jagDrawHeaderFiles}
    ${_jagDrawGl3wHeaderFiles}
    ${_jagDrawSourceFiles}
    ${_jagDrawGl3wSourceFiles}
    JAG_LIBRARIES
        jagBase
)


if( JAG3D_USE_GL3W )
    # Generate gl3w.c and gl3w.h if out of data.
    if( JAG3D_USE_GL3W_AUTOGENERATE AND PYTHONINTERP_FOUND )
        add_custom_command( 
            OUTPUT
                ${PROJECT_SOURCE_DIR}/src/jag/draw/gl3w.c
                ${PROJECT_SOURCE_DIR}/src/jag/draw/gl3w.h
            COMMAND
                ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/src/gl3w/gl3w_gen.py
            MAIN_DEPENDENCY glExtensions.h
            DEPENDS ${GL3_GLCOREARB_HEADER}/GL/glcorearb.h
            COMMENT "Executing Python-based GL3W source code generation:" )
    else()
        message( WARNING "Copying default GL3W source because Python was not found. Please use glcorearb.h from src/gl3w/GL/glcorearb.h to ensure compatibility." )
        configure_file( ${PROJECT_SOURCE_DIR}/src/gl3w/gl3w.c
            ${PROJECT_SOURCE_DIR}/src/jag/draw/gl3w.c
            COPYONLY )
        configure_file( ${PROJECT_SOURCE_DIR}/src/gl3w/gl3w.h
            ${PROJECT_SOURCE_DIR}/src/jag/draw/gl3w.h
            COPYONLY )
    endif()
endif()

install(
    DIRECTORY .
    DESTINATION include/jag/draw
    USE_SOURCE_PERMISSIONS
    FILES_MATCHING 
    PATTERN "*.h"
    PATTERN ".hg" EXCLUDE
    PATTERN ".txt" EXCLUDE
    PATTERN ".cpp" EXCLUDE
    PATTERN ".cxx" EXCLUDE
)
