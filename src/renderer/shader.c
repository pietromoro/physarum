#include "shader.h"
#include "system/logger.h"
#include "system/opengl.h"
#include "system/memory.h"
#include "gl/gl.h"

u32 shader_create(shader_info* info) {
  u8 logInfo[512];
  i32 success;
  
  u32 shaderProgram = glCreateProgram();
  
  for (shader_info* shaderInfo = info; shaderInfo->type; shaderInfo++) {
    u32 shader = glCreateShader(shaderInfo->type);
    glShaderSource(shader, 1, &shaderInfo->source, 0);
    glCompileShader(shader);
    
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(shader, 512, 0, logInfo);
      PH_FATAL("Could not compile shader: %s", logInfo);
      return 0;
    }
    
    glAttachShader(shaderProgram, shader);
    glLinkProgram(shaderProgram);
    
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
      glGetProgramInfoLog(shaderProgram, 512, 0, logInfo);
      PH_FATAL("Compilation failed: %s", logInfo);
      return 0;
    }
    
    glDeleteShader(shader);
  }
  
  return shaderProgram;
}