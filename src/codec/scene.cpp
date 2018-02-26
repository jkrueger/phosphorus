#include "codec/scene.hpp"
#include "things/scene.hpp"

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// void add_materials(scene_t& scene, const aiScene* aiScene) {
//   printf("Import materials: %d\n", aiScene->mNumMaterials);

//   for (auto i=0; i<aiScene->mNumMaterials; ++i) {
//     auto materials = aiScene->mMaterials[node->mMeshes[i]];
//   }
// }

void add_meshes(scene_t& scene, const aiScene* aiScene, const aiNode* node) {
  printf("Node: %s\n", node->mName.C_Str());

  for (auto i=0; i<node->mNumMeshes; ++i) { 
    auto mesh = aiScene->mMeshes[node->mMeshes[i]];
    auto mat  = scene.material(mesh->mMaterialIndex);
    auto out  = mesh_t::p(new mesh_t(mat ? mat : scene.material(0)));

    printf("Loading mesh %s: %d, %d\n", mesh->mName.C_Str(), mesh->mNumFaces, mesh->mMaterialIndex);

    out->num_faces    = mesh->mNumFaces;
    out->num_vertices = mesh->mNumVertices;

    for (auto j=0; j<mesh->mNumFaces; ++j) {
      auto face = &mesh->mFaces[j];

      if (face->mNumIndices == 3) {
	for (auto k=0; k<face->mNumIndices; ++k) {
	  mesh_t::faces.push_back(face->mIndices[k]);
	}
      }
      else {
	printf("Only triangles are supported right now: %d\n", face->mNumIndices);
	out->num_faces--;
      }
    }

    if (out->num_faces > 0) {
      for (auto k=0; k<mesh->mNumVertices; ++k) {
	auto& v = mesh->mVertices[k];
	mesh_t::vertices.push_back({v.x, v.y, v.z});
      }

      if (mesh->mNormals) {
	for (auto k=0; k<mesh->mNumVertices; ++k) {
	  auto& n = mesh->mNormals[k];
	  mesh_t::normals.push_back({n.x, n.y, n.z});
	}
      }
      else {
	out->compute_normals();
      }

      scene.add(out);
      printf("id: %d\n", out->id);
    }
    else {
      printf("skipping empty mesh\n");
      delete out;
    }
  }

  for (auto i=0; i<node->mNumChildren; ++i) {
    add_meshes(scene, aiScene, node->mChildren[i]);
  }
}

namespace codec {
  namespace scene {
    void load(const std::string& path, scene_t& scene) {
      printf("Loading scene: %s\n", path.c_str());
      auto import = aiImportFile(
        path.c_str(),
	aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_Triangulate);
      add_meshes(scene, import, import->mRootNode);
    }
  }
}
