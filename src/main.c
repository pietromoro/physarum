#include "defines.h"
#include "system/clock.h"
#include "system/logger.h"
#include "system/window.h"
#include "system/opengl.h"
#include "renderer/shader.h"

#include "windows.h"
#include "gl/gl.h"

#define WINDOW_WIDTH (512)
#define ASPECT_RATIO (16.0f/9.0f)

u32 CreateQuadVAO();
u32 CreateQuadProgram();

i32 CALLBACK WinMain(HINSTANCE instance, HINSTANCE previous, LPSTR cmdLine, i32 showCode) {
  logger_initialize();
  
  clock* clock = clock_create();
  u32 WINDOW_HEIGHT = ((f32)WINDOW_WIDTH / ASPECT_RATIO);
  window* window = window_create("Physarum", WINDOW_WIDTH, WINDOW_HEIGHT);
  window_show(window);
  
  opengl_context(window);
  glEnable(GL_LINE_SMOOTH);
  glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  u32 quadVAO = CreateQuadVAO();
  u32 quadProgram = CreateQuadProgram();
  
  shader_info computeShaderInfo[] = {
    {GL_COMPUTE_SHADER, "#version 430\n"
        "layout (local_size_x = 1, local_size_y = 1) in;\n"
        "layout (rgba32f) uniform image2D imgOut;\n"
        ""
        "uint hash(uint state) {\n"
        "   state ^= 2747636419u;\n"
        "   state *= 2654435769u;\n"
        "   state ^= state >> 16;\n"
        "   state *= 2654435769u;\n"
        "   state ^= state >> 16;\n"
        "   state *= 2654435769u;\n"
        "   return state;\n"
        "}\n"
        ""
        "void main()\n"
        "{\n"
        "   vec4 pixel = vec4(0.0, 1.0, 0.0, 1.0);\n"
        "   ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);\n"
        "   ivec2 dims = imageSize(imgOut);\n"
        ""
        "   int pixelIndex = pixelCoords.y * dims.y + pixelCoords.x;\n"
        "   vec3 val  = vec3(hash(pixelIndex) / 4294967295.0);\n"
        "   pixel = vec4(val, 1.0);\n"
        "   imageStore(imgOut, pixelCoords, pixel);\n"
        "}\0"
    },
    0
  };
  
  u32 computeProgram = shader_create(computeShaderInfo);
  u32 outLoc = glGetUniformLocation(computeProgram, "imgOut");
  
  u32 texture = 0;
  { // create the texture
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, (u32)window->width, (u32)window->height, 0, GL_RGBA, GL_FLOAT, 0);
    glBindImageTexture(outLoc, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
  }
  
  { // query up the workgroups
    int groupSize[3], groupInv;
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &groupSize[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &groupSize[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &groupSize[2]);
    PH_LOG("Max global (total) work group size x: %i, y: %i, z: %i", groupSize[0], groupSize[1], groupSize[2]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &groupSize[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &groupSize[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &groupSize[2]);
    PH_LOG("Max local (in one shader) work group sizes x: %i, y: %i, z: %i", groupSize[0], groupSize[1], groupSize[2]);
    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &groupInv);
    PH_LOG("Max compute shader invocations %i", groupInv);
  }
  
  glViewport(0, 0, window->width, window->height);
  glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
  
  f64 lastTime;
  clock_start(clock);
  clock_update(clock);
  lastTime = clock->elapsed;
  while(!window->close_requested) {
    clock_update(clock);
    f64 currentTime = clock->elapsed;
    f64 delta = currentTime - lastTime;
    
    if (window->size_changed) {
      glViewport(0, 0, window->width, window->height);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, window->width, window->height, 0, GL_RGBA, GL_FLOAT, 0);
      window->size_changed = 0;
    }
    
    glUseProgram(computeProgram);
    glBindImageTexture(outLoc, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glDispatchCompute((u32)window->width, (u32)window->height, 1);
    
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(quadProgram);
    glBindVertexArray(quadVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    opengl_swap_buffers(window);
    window_poll_events(window);
    lastTime = currentTime;
  }
  
  clock_stop(clock);
  window_destroy(window);
  logger_destroy();
  return 0;
}

u32 CreateQuadVAO() {
  u32 vao = 0, vbo = 0;
  f32 verts[] = { 
    -1.0f, -1.0f,    0.0f, 0.0f,
    -1.0f, 1.0f,     0.0f, 1.0f,
    1.0f, -1.0f,     1.0f, 0.0f,
    1.0f, 1.0f,      1.0f, 1.0f
  };
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(f32), verts, GL_STATIC_DRAW);
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glEnableVertexAttribArray(0);
  memory_index stride = 4 * sizeof(f32);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, 0);
  glEnableVertexAttribArray(1);
  memory_index offset = 2 * sizeof(f32);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)offset);
  return vao;
}

u32 CreateQuadProgram() {
  shader_info quadShaders[] = {
    {GL_VERTEX_SHADER, "#version 330 core\n"
        "layout (location = 0) in vec2 pos;\n"
        "layout (location = 1) in vec2 tex;\n"
        "out vec2 st;\n"
        "void main()\n"
        "{\n"
        "   st = tex;\n"
        "   gl_Position = vec4(pos, 0.0, 1.0);\n"
        "}\0"
    },
    {GL_FRAGMENT_SHADER, "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec2 st;\n"
        "uniform sampler2D img;\n"
        "void main()\n"
        "{\n"
        "   FragColor = texture(img, st);\n"
        "}\0"
    },
    0
  };
  return shader_create(quadShaders);
}