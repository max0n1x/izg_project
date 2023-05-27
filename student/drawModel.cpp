/*!
 * @file
 * @brief This file contains functions for model rendering
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 */
#include <student/drawModel.hpp>
#include <student/gpu.hpp>

///\endcond

//void prepareNode(GPUMemory& mem, CommandBuffer& cb, Node const& node, Model const& model,
//                 glm::mat4 const& prubeznaMatice) {
//
//    if (node.mesh >= 0) {
//        Mesh mesh = model.meshes[node.mesh];
//
//        // Set vertex array object attributes
//        mem.vao.indexBuffer = mesh.indices;
//        mem.vao.indexType = mesh.indexType;
//
//        mem.vao.vertexAttrib[0] = mesh.position;
//        mem.prg.vs2fs[0] = AttributeType::VEC3;
//        mem.vao.vertexAttrib[1] = mesh.normal;
//        mem.prg.vs2fs[1] = AttributeType::VEC3;
//        mem.vao.vertexAttrib[2] = mesh.texCoord;
//        mem.prg.vs2fs[2] = AttributeType::VEC2;
//
//        // Create a draw call and write data to GPU memory
//        createDrawCall(mem, cb);
//
//        // Update the model matrix using the provided Node structure
//        glm::mat4 updatedMatrix = prubeznaMatice * node.modelMatrix;
//        updateModelMatrix(mem, node, updatedMatrix);
//    }
//
//    for (size_t i = 0; i < node.children.size(); ++i) {
//        prepareNode(mem, cb, node.children[i], model, prubeznaMatice);
//    }
//}


/**
 * @brief This function prepares model into memory and creates command buffer
 *
 * @param mem gpu memory
 * @param commandBuffer command buffer
 * @param model model structure
 */
//! [drawModel]
void prepareModel(GPUMemory&mem, CommandBuffer&commandBuffer, Model const&model) {
    /// \todo Tato funkce připraví command buffer pro model a nastaví správně pamět grafické karty.<br>
    /// Vaším úkolem je správně projít model a vložit vykreslovací příkazy do commandBufferu.
    /// Zároveň musíte vložit do paměti textury, buffery a uniformní proměnné, které buffer command buffer využívat.
    /// Bližší informace jsou uvedeny na hlavní stránce dokumentace a v testech.

    // Initialize buffers, textures, and programs using the provided Model structure
    // ...

    // Clear command buffer
//    commandBuffer.nofCommands = 0;
//
//    glm::mat4 jednotkovaMatrice = glm::mat4(1.f);
//    for (size_t i = 0; i < model.roots.size(); ++i) {
//            prepareNode(mem, commandBuffer, model.roots[i], model, jednotkovaMatrice);
//    }

}

//! [drawModel]

/**
 * @brief This function represents vertex shader of texture rendering method.
 *
 * @param outVertex output vertex
 * @param inVertex input vertex
 * @param si shader interface
 */
//! [drawModel_vs]
void drawModel_vertexShader(OutVertex&outVertex,InVertex const&inVertex,ShaderInterface const&si){
  /// \todo Tato funkce reprezentujte vertex shader.<br>
  /// Vaším úkolem je správně trasnformovat vrcholy modelu.
  /// Bližší informace jsou uvedeny na hlavní stránce dokumentace.

    auto pos        = inVertex.attributes[0].v3;
    auto normal     = inVertex.attributes[1].v3;
    auto texCoords  = inVertex.attributes[2].v2;

    auto projMatrix     = si.uniforms[0].m4;
    auto modelMatrix    = si.uniforms[1].m4;
    auto inverseMatrix  = si.uniforms[2].m4;

    outVertex.gl_Position      = projMatrix * modelMatrix * glm::vec4(pos,1.f);
    outVertex.attributes[0].v3 = modelMatrix*glm::vec4(pos,1.f);
    outVertex.attributes[1].v3 = inverseMatrix*glm::vec4(normal,0.f);
    outVertex.attributes[2].v2 = texCoords;
}
//! [drawModel_vs]

/**
 * @brief This functionrepresents fragment shader of texture rendering method.
 *
 * @param outFragment output fragment
 * @param inFragment input fragment
 * @param si shader interface
 */
//! [drawModel_fs]
void drawModel_fragmentShader(OutFragment&outFragment,InFragment const&inFragment,ShaderInterface const&si){
  /// \todo Tato funkce reprezentujte fragment shader.<br>
  /// Vaším úkolem je správně obarvit fragmenty a osvětlit je pomocí lambertova osvětlovacího modelu.
  /// Bližší informace jsou uvedeny na hlavní stránce dokumentace.

    auto light          = si.uniforms[3].v3;
    auto diffuseColor   = si.uniforms[5].v4;
    auto tex            = si.uniforms[6].v1;
    auto diffuseTexture = si.uniforms[0].v4;

    auto pos       = inFragment.attributes[0].v3;
    auto nor       = inFragment.attributes[1].v3;
    auto texCoords = inFragment.attributes[2].v2;

    auto N=glm::normalize(nor);

    if (tex > 0){
        diffuseColor = read_texture(si.textures[0], texCoords);
    }


    auto L = glm::normalize(light-pos);
    float dF = glm::max(glm::dot(L,glm::normalize(nor)),0.f);
    float aF = 0.2f;

    glm::vec3 dL = glm::vec3(diffuseColor)*dF;
    glm::vec3 aL = glm::vec3(diffuseColor)*aF;

    outFragment.gl_FragColor = glm::vec4(glm::vec3(aL+dL), diffuseColor.a);
}
//! [drawModel_fs]

