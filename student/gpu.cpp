/*!
 * @file
 * @brief This file contains implementation of gpu
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 */

#include <student/gpu.hpp>
#include <cmath>

void clear(GPUMemory& mem, ClearCommand cmd) {
    if (cmd.clearColor) {
        float red = cmd.color.r;
        float green = cmd.color.g;
        float blue = cmd.color.b;
        float alpha = cmd.color.a;

        size_t bufferSize = mem.framebuffer.width * mem.framebuffer.height * mem.framebuffer.channels;

        for (size_t i = 0; i < bufferSize; i += 4) {
            mem.framebuffer.color[i] = static_cast<uint8_t>(red * 255.0f);
            mem.framebuffer.color[i + 1] = static_cast<uint8_t>(green * 255.0f);
            mem.framebuffer.color[i + 2] = static_cast<uint8_t>(blue * 255.0f);
            mem.framebuffer.color[i + 3] = static_cast<uint8_t>(alpha * 255.0f);
        }
    }
}

Buffer getBufferData(GPUMemory const&mem, int32_t bufferID) {
    return mem.buffers[bufferID];
}

uint32_t computeVertexID32(VertexArray const &vao, uint32_t shaderInvocation, GPUMemory const&mem)
{
    Buffer indexBuffer = getBufferData(mem, vao.indexBufferID);
    uint32_t* indexes = (uint32_t*)indexBuffer.data;
    return indexes[shaderInvocation];
}

uint16_t computeVertexID16(VertexArray const &vao, uint32_t shaderInvocation, GPUMemory const&mem)
{
    Buffer indexBuffer = getBufferData(mem, vao.indexBufferID);
    uint16_t* indexes = (uint16_t*)(indexBuffer.data);
    return indexes[shaderInvocation];
}

uint8_t computeVertexID8(VertexArray const &vao, uint32_t shaderInvocation, GPUMemory const&mem)
{
    Buffer indexBuffer = getBufferData(mem, vao.indexBufferID);
    uint8_t* indexes = (uint8_t*)(indexBuffer.data);
    return indexes[shaderInvocation];
}

void getVertexID(VertexArray const&vao, InVertex *inVertex, uint32_t v, GPUMemory const&mem)
{
    if (vao.indexBufferID == -1)
    {
        //non-indexed
        inVertex->gl_VertexID = v;
    }
    else
    {   //indexed
        switch (vao.indexType)
        {
            case IndexType::UINT32:
                inVertex->gl_VertexID = computeVertexID32(vao, v, mem);
                break;

            case IndexType::UINT16:
                inVertex->gl_VertexID = computeVertexID16(vao, v, mem);
                break;

            case IndexType::UINT8:
                inVertex->gl_VertexID = computeVertexID8(vao, v, mem);
                break;
        }
    }
}

void setVertexAttributes(VertexAttrib const attrib[maxAttributes], InVertex *inVertex, GPUMemory const&mem)
{
    for (uint32_t i = 0; i < maxAttributes; i++)
    {
        uint32_t ID = inVertex->gl_VertexID;
        uint64_t stride = (attrib[i].stride);
        uint64_t offset = (attrib[i].offset);
        uint8_t *buffer = (uint8_t *) getBufferData(mem, attrib[i].bufferID).data;
        uint8_t *ptr;

        switch (attrib[i].type)
        {
            case AttributeType::EMPTY:
                continue;

            case AttributeType::FLOAT:
                ptr = buffer + offset + stride*ID;
                inVertex->attributes[i].v1 = *((float *)ptr);
                break;

            case AttributeType::VEC2:
                for (uint32_t j = 0; j < 2; j++)
                {
                    ptr = buffer + offset + stride*ID + j*4;
                    inVertex->attributes[i].v2[j] = *((float *)ptr);
                }
                break;

            case AttributeType::VEC3:
                for (uint32_t j = 0; j < 3; j++)
                {
                    ptr = buffer + offset + stride*ID + j*4;
                    inVertex->attributes[i].v3[j] = *((float *)ptr);
                }
                break;

            case AttributeType::VEC4:
                for (uint32_t j = 0; j < 4; j++)
                {
                    ptr = buffer + offset + stride*ID + j*4;
                    inVertex->attributes[i].v4[j] = *((float *)ptr);
                }
                break;
        }
    }
}

void runVertexAssembly(VertexArray const&vao, InVertex *inVertex, uint32_t v, GPUMemory const&mem)
{
    getVertexID(vao, inVertex, v, mem);
    setVertexAttributes(vao.vertexAttrib, inVertex, mem);
}

void draw(GPUMemory& mem, DrawCommand cmd)
{

    Program prg = mem.programs[cmd.programID];

    for (uint32_t v = 0; v < cmd.nofVertices; ++v)
    {
        InVertex inVertex;
        OutVertex outVertex;
        runVertexAssembly(cmd.vao, &inVertex, v, mem);
        ShaderInterface si;
        prg.vertexShader(outVertex, inVertex, si);
    }
}

//! [gpu_execute]
void gpu_execute(GPUMemory&mem,CommandBuffer &cb){

  /// \todo Tato funkce reprezentuje funkcionalitu grafické karty.<br>
  /// Měla by umět zpracovat command buffer, čistit framebuffer a kresli.<br>
  /// mem obsahuje paměť grafické karty.
  /// cb obsahuje command buffer pro zpracování.
  /// Bližší informace jsou uvedeny na hlavní stránce dokumentace.
    for(uint32_t i=0;i<cb.nofCommands;++i) {

        CommandType type = cb.commands[i].type;
        CommandData data = cb.commands[i].data;

        if (type == CommandType::CLEAR) {
            clear(mem, data.clearCommand);
        }

        if (type == CommandType::DRAW ) {
            draw(mem, data.drawCommand);
        }

    }
}
//! [gpu_execute]

/**
 * @brief This function reads color from texture.
 *
 * @param texture texture
 * @param uv uv coordinates
 *
 * @return color 4 floats
 */
glm::vec4 read_texture(Texture const&texture,glm::vec2 uv){
  if(!texture.data)return glm::vec4(0.f);
  auto uv1 = glm::fract(uv);
  auto uv2 = uv1*glm::vec2(texture.width-1,texture.height-1)+0.5f;
  auto pix = glm::uvec2(uv2);
  //auto t   = glm::fract(uv2);
  glm::vec4 color = glm::vec4(0.f,0.f,0.f,1.f);
  for(uint32_t c=0;c<texture.channels;++c)
    color[c] = texture.data[(pix.y*texture.width+pix.x)*texture.channels+c]/255.f;
  return color;
}

