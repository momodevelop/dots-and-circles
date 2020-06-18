#ifndef __YUU_GL__
#define __YUU_GL__

// NOTE(Momo): Maybe one day we don't need glad.c and write our
//             our own extentions but meh, I'm lazy for now.
#include "thirdparty/glad/glad.c"

#ifdef DEBUG_OGL
struct GLDebug {
    void (*Logger)(const char* fmt, ...);
};

pure 
void 
GLDebugInit(GLDebug* debugObj, void (*logger)(const char*,...)) {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    
    debugObj->logger = logger;
    
    auto callback = [](GLenum source,
                       GLenum type,
                       GLuint id,
                       GLenum severity,
                       GLsizei length,
                       const GLchar* msg,
                       const void* userParam)
    {
        
        // Ignore NOTIFICATION severity
        if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) 
            return;
        
        (void)length; 
        (void)userParam;
        char* _source;
        char* _type;
        char* _severity;
        switch (source) {
            case GL_DEBUG_SOURCE_API:
            _source = "API";
            break;
            
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            _source = "WINDOW SYSTEM";
            break;
            
            case GL_DEBUG_SOURCE_SHADER_COMPILER:
            _source = "SHADER COMPILER";
            break;
            
            case GL_DEBUG_SOURCE_THIRD_PARTY:
            _source = "THIRD PARTY";
            break;
            
            case GL_DEBUG_SOURCE_APPLICATION:
            _source = "APPLICATION";
            break;
            
            case GL_DEBUG_SOURCE_OTHER:
            _source = "UNKNOWN";
            break;
            
            default:
            _source = "UNKNOWN";
            break;
        }
        
        switch (type) {
            case GL_DEBUG_TYPE_ERROR:
            _type = "ERROR";
            break;
            
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            _type = "DEPRECATED BEHAVIOR";
            break;
            
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            _type = "UDEFINED BEHAVIOR";
            break;
            
            case GL_DEBUG_TYPE_PORTABILITY:
            _type = "PORTABILITY";
            break;
            
            case GL_DEBUG_TYPE_PERFORMANCE:
            _type = "PERFORMANCE";
            break;
            
            case GL_DEBUG_TYPE_OTHER:
            _type = "OTHER";
            break;
            
            case GL_DEBUG_TYPE_MARKER:
            _type = "MARKER";
            break;
            
            default:
            _type = "UNKNOWN";
            break;
        }
        
        switch (severity) {
            case GL_DEBUG_SEVERITY_HIGH:
            _severity = "HIGH";
            break;
            
            case GL_DEBUG_SEVERITY_MEDIUM:
            _severity = "MEDIUM";
            break;
            
            case GL_DEBUG_SEVERITY_LOW:
            _severity = "LOW";
            break;
            
            //case GL_DEBUG_SEVERITY_NOTIFICATION:
            //_severity = "NOTIFICATION";
            //break;
            
            default:
            _severity = "UNKNOWN";
            break;
        }
        
        const GLDebug* db = (const GLDebug*)userParam;
        db->Logger("[OpenGL] %d: %s of %s severity, raised from %s: %s\n",
                   id, _type, _severity, _source, msg);
        
    };
    
    glDebugMessageCallback(callback, debugObj);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);
}
#endif // DEBUG_OGL


pure 
void 
GLAttachShader(GLuint program, GLenum type, const GLchar* code) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &code, NULL);
    glCompileShader(shader);
    glAttachShader(program, shader);
    glDeleteShader(shader);
}




#endif