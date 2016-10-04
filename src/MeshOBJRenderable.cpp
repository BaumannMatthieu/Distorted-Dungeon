#include <fstream>
#include <iostream>

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../include/MeshOBJRenderable.hpp"
#include "../include/ShaderProgram.hpp"

MeshOBJ::MeshOBJ(const std::string& filename) {
    std::vector<unsigned short> vertex_index, normal_index, texcoords_index;

    std::string header;
    std::ifstream file(filename);

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;

    if(file.is_open()) {
        while(file >> header) {
            if(header == "v") {
                glm::vec3 position;
                file >> position.x;
                file >> position.y;
                file >> position.z;

                positions.push_back(position);
            } else if(header == "vt") {
                glm::vec2 texcoord;
                file >> texcoord.x;
                file >> texcoord.y;

                texcoords.push_back(texcoord);
            } else if(header == "vn") {
                glm::vec3 normal;
                file >> normal.x;
                file >> normal.y;
                file >> normal.z;

                normals.push_back(normal);
            } else if(header == "f") {
                unsigned int index_v_1, index_v_2, index_v_3;
                unsigned int index_vt_1, index_vt_2, index_vt_3;
                unsigned int index_vn_1, index_vn_2, index_vn_3;
                file >> index_v_1;
                file.ignore();
                file >> index_vt_1;
                file.ignore();
                file >> index_vn_1;

                file >> index_v_2;
                file.ignore();
                file >> index_vt_2;
                file.ignore();
                file >> index_vn_2;

                file >> index_v_3;
                file.ignore();
                file >> index_vt_3;
                file.ignore();
                file >> index_vn_3;

                m_positions.push_back(positions[index_v_1 - 1]);
                m_positions.push_back(positions[index_v_2 - 1]);
                m_positions.push_back(positions[index_v_3 - 1]);

                m_texcoords.push_back(texcoords[index_vt_1 - 1]);
                m_texcoords.push_back(texcoords[index_vt_2 - 1]);
                m_texcoords.push_back(texcoords[index_vt_3 - 1]);

                m_normals.push_back(normals[index_vn_1 - 1]);
                m_normals.push_back(normals[index_vn_2 - 1]);
                m_normals.push_back(normals[index_vn_3 - 1]);
            }
        }
        file.close();
    }
}

MeshOBJ::~MeshOBJ() {

}

const std::vector<glm::vec3>& MeshOBJ::getPositions() const {
    return m_positions;
}

const std::vector<glm::vec3>& MeshOBJ::getNormals() const {
    return m_normals;
}

const std::vector<glm::vec2>& MeshOBJ::getTexcoords() const {
    return m_texcoords;
}
