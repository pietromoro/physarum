#include "defines.h"
#include "system/clock.h"
#include "system/memory.h"
#include "system/logger.h"
#include "system/window.h"
#include "system/opengl.h"
#include "renderer/shader.h"

#include "math.h"

#include "windows.h"
#include "gl/gl.h"

#define WINDOW_WIDTH (1000)
#define ASPECT_RATIO (16.0f/9.0f)

#define NUM_AGENTS (250)
#define MOVE_SPEED (0.1f)
#define TURN_SPEED (0.8f)

#define EVAPORATE_SPEED (0.1f)
#define DIFFUSE_SPEED (10.8f)

#define SENSOR_ANGLE (0.4f)
#define SENSOR_SIZE (2)
#define SENSOR_OFFSET (0.4f)

u32 CreateQuadVAO();
u32 CreateQuadProgram();
u32 Hash(u32 state);

typedef struct agent {
  f32 x, y;
  f32 dir;
  u32 pad; // NOTE: padding ?!
} agent;

void ClearAgentsBuffer(window* window, agent* agents, u32 buffer);

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
        "layout (rgba32f) uniform image2D trailMap;\n"
        "\n"
        "#define PI 3.1415926535\n"
        "\n"
        "uniform float deltaTime;\n"
        "uniform float moveSpeed;\n"
        "uniform float turnSpeed;\n"
        "uniform int agentCount;\n"
        "uniform int sensorSize;\n"
        "uniform float sensorAngle;\n"
        "uniform float sensorOffsetDST;\n"
        "\n"
        "struct agent {\n"
        "   vec2 pos;\n"
        "   float dir;\n"
        "};\n"
        "\n"
        "layout(std140, binding = 0) buffer agentsBuffer {\n"
        "   agent agents[];\n"
        "};\n"
        "\n"
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
        "float sense(agent ag, float offset, ivec2 imageDim) {\n"
        "   float angle = ag.dir + offset;\n"
        "   vec2 dir = vec2(cos(angle), sin(angle));\n"
        "   vec2 centerF = ag.pos + dir * sensorOffsetDST;\n"
        "   ivec2 center = ivec2(centerF.x, centerF.y);\n"
        "   float final = 0.0;\n"
        "   for (int offX = -sensorSize; offX <= sensorSize; offX++) {\n"
        "      for (int offY = -sensorSize; offY <= sensorSize; offY++) {\n"
        "         ivec2 pos = center + ivec2(offX, offY);\n"
        "         if (pos.x >= 0 && pos.x < imageDim.x && pos.y >= 0 && pos.y < imageDim.y) {\n"
        "            final += imageLoad(trailMap, pos).x;\n"
        "         }\n"
        "      }\n"
        "   }\n"
        "   return final;\n"
        "}\n"
        "\n"
        "void main()\n"
        "{\n"
        "   ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);\n"
        "   ivec2 imageDim = imageSize(trailMap);\n"
        "   if (pixelCoord.x >= agentCount) { return; }\n"
        ""
        "   agent ag = agents[pixelCoord.x];\n"
        "   uint random = hash(uint(ag.pos.y * imageDim.y + ag.pos.x + hash(pixelCoord.x)));\n"
        ""
        "   vec2 dir = vec2(cos(ag.dir), sin(ag.dir));\n"
        "   vec2 pos = ag.pos + dir * moveSpeed * deltaTime;\n"
        ""
        "   vec3 weights = vec3(sense(ag, 0, imageDim), sense(ag, sensorAngle, imageDim), sense(ag, -sensorAngle, imageDim));\n"
        "   float randomSteerStrength = (random / 4294967295.0);\n"
        ""
        "   if (weights.x > weights.y && weights.x > weights.z) { agents[pixelCoord.x].dir += 0.0; }\n"
        "   else if (weights.x < weights.y && weights.x < weights.z) {\n"
        "      agents[pixelCoord.x].dir += (randomSteerStrength - 0.5) * 2 * turnSpeed * deltaTime;\n"
        "   } else if (weights.z > weights.y) {\n"
        "      agents[pixelCoord.x].dir -= randomSteerStrength * turnSpeed * deltaTime;\n"
        "   } else if (weights.y > weights.z) {\n"
        "      agents[pixelCoord.x].dir += randomSteerStrength * turnSpeed * deltaTime;\n"
        "   }\n"
        ""
        "   if (pos.x < 0 || pos.x >= imageDim.x || pos.y < 0 || pos.y >= imageDim.y) {\n"
        "      pos.x = min(imageDim.x - 0.01, max(0, pos.x));\n"
        "      pos.y = min(imageDim.y - 0.01, max(0, pos.y));\n"
        "      agents[pixelCoord.x].dir = (random / 4294967295.0) * 2 * PI;\n"
        "   }\n"
        ""
        "   agents[pixelCoord.x].pos = pos;\n"
        "   imageStore(trailMap, ivec2(pos.x, pos.y), vec4(1.0));\n"
        "}\0"
    },
    0
  };
  
  u32 computeProgram = shader_create(computeShaderInfo);
  u32 outLoc = glGetUniformLocation(computeProgram, "trailMap");
  u32 deltaTimeLoc = glGetUniformLocation(computeProgram, "deltaTime");
  u32 agentsLoc = 0;
  glUseProgram(computeProgram);
  glUniform1i(glGetUniformLocation(computeProgram, "agentCount"), NUM_AGENTS);
  glUniform1f(glGetUniformLocation(computeProgram, "moveSpeed"), MOVE_SPEED);
  glUniform1f(glGetUniformLocation(computeProgram, "turnSpeed"), MOVE_SPEED);
  glUniform1i(glGetUniformLocation(computeProgram, "sensorSize"), SENSOR_SIZE);
  glUniform1f(glGetUniformLocation(computeProgram, "sensorAngle"), SENSOR_ANGLE);
  glUniform1f(glGetUniformLocation(computeProgram, "sensorOffsetDST"), SENSOR_OFFSET);
  
  shader_info updateMapShaderInfo[] = {
    {GL_COMPUTE_SHADER, "#version 430\n"
        "layout (local_size_x = 1, local_size_y = 1) in;\n"
        "layout (rgba32f) uniform image2D trailMap;\n"
        "\n"
        "uniform float deltaTime;\n"
        "uniform float diffuseSpeed;\n"
        "uniform float evaporateSpeed;\n"
        "\n"
        "void main()\n"
        "{\n"
        "   ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);\n"
        "   ivec2 imageDim = imageSize(trailMap);\n"
        "   if (pixelCoord.x < 0 || pixelCoord.x >= imageDim.x || pixelCoord.y < 0 || pixelCoord.y >= imageDim.y) { return; }\n"
        ""
        "   vec4 original  = imageLoad(trailMap, pixelCoord);\n"
        ""
        "   vec4 finalSum = vec4(0.0);\n"
        "   for (int offX = -1; offX <= 1; offX++) {\n"
        "      for (int offY = -1; offY <= 1; offY++) {\n"
        "         ivec2 samplePoint = pixelCoord + ivec2(offX, offY);\n"
        "         if (samplePoint.x >= 0 && samplePoint.x < imageDim.x && samplePoint.y >= 0 && samplePoint.y < imageDim.y) {\n"
        "            finalSum += imageLoad(trailMap, samplePoint);\n"
        "         }\n"
        "      }\n"
        "   }\n"
        "   vec4 blurred = finalSum / 9.0;"
        "   vec4 diffused = mix(original, blurred, diffuseSpeed * deltaTime);\n"
        "   vec4 newValue = max(vec4(0.0), diffused - evaporateSpeed * deltaTime);\n"
        ""
        "   imageStore(trailMap, pixelCoord, newValue);\n"
        "}\0"
    },
    0
  };
  
  u32 processingShader = shader_create(updateMapShaderInfo);
  u32 processingMapLoc = glGetUniformLocation(processingShader, "trailMap");
  u32 processingDeltaTimeLoc = glGetUniformLocation(processingShader, "deltaTime");
  glUseProgram(processingShader);
  glUniform1f(glGetUniformLocation(processingShader, "evaporateSpeed"), EVAPORATE_SPEED);
  glUniform1f(glGetUniformLocation(processingShader, "diffuseSpeed"), DIFFUSE_SPEED);
  
  u32 imageW = 320; // window->width;
  u32 imageH = 180; // window->height;
  u32 texture = 0;
  { // create the texture
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, imageW, imageH, 0, GL_RGBA, GL_FLOAT, 0);
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
  
  u32 agentsBuffer = 0;
  glUseProgram(computeProgram);
  glGenBuffers(1, &agentsBuffer);
  agent* agents = (agent*)ph_alloc(sizeof(agent) * NUM_AGENTS);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, agentsBuffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(agent) * NUM_AGENTS, 0, GL_DYNAMIC_DRAW);
  ClearAgentsBuffer(window, agents, agentsBuffer);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, agentsLoc, agentsBuffer);
  
  
  glViewport(0, 0, window->width, window->height);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  
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
      //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, window->width, window->height, 0, GL_RGBA, GL_FLOAT, 0);
      //ClearAgentsBuffer(window, agents, agentsBuffer);
      window->size_changed = 0;
    }
    
    glUseProgram(computeProgram);
    glBindImageTexture(outLoc, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glUniform1f(deltaTimeLoc, delta);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, agentsBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, agentsLoc, agentsBuffer);
    glDispatchCompute(imageW, imageH, 1);
    
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
    
    glUseProgram(processingShader);
    glBindImageTexture(processingMapLoc, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glUniform1f(processingDeltaTimeLoc, delta);
    glDispatchCompute(imageW, imageH, 1);
    
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
  
  ph_free(agents);
  clock_stop(clock);
  window_destroy(window);
  logger_destroy();
  return 0;
}

void ClearAgentsBuffer(window* window, agent* agents, u32 buffer) {
  const f32 radius = MINIMUM(window->width, window->height);
  for (u32 i = 0; i < NUM_AGENTS; i++) {
    agent* current = &agents[i];
    const u32 rand = Hash((memory_index)(void*)current * i);
    current->dir = (f32)(rand % 360) * (3.14159265f / 180.0f);
    const f32 r = sqrtf((f32)rand / 4294967295.0) * radius;
    current->x = (f32)window->width / 2.0f; // + cosf(current->dir) * r;
    current->y = (f32)window->height / 2.0f; // + sinf(current->dir) * r;
    current->dir += 3.14159265f;
  }
  
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
  glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(agent) * NUM_AGENTS, agents);
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

u32 Hash(u32 state) {
  state ^= 2747636419;
  state *= 2654435769;
  state ^= state >> 16;
  state *= 2654435769;
  state ^= state >> 16;
  state *= 2654435769;
  return state;
}