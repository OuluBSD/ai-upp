#include "Geometry.h"


NAMESPACE_UPP




ModelNode::ModelNode() {
	local_transform = Identity<mat4>();
	modify_count = 0;
}

ModelNode::ModelNode(const ModelNode& n) {
	modify_count = 0;
	*this = n;
}

void ModelNode::Visit(Vis& v) {
	_VIS_(name)
	 VIS_(index)
	 VIS_(parent_node_index)
	 VISN(local_transform);
	
	if (v.IsLoading())
		++modify_count;
}








void Model::Texture::Visit(Vis& v) {
	v VISN(img)
	  VIS_(path);
}

void Model::CubeTexture::Visit(Vis& v) {
	v VISN(img[0])
	  VISN(img[1])
	  VISN(img[2])
	  VISN(img[3])
	  VISN(img[4])
	  VISN(img[5])
	  VIS_(path);
}

void Model::Visit(Vis& v) {
	v VISV(meshes)
	 VISV(nodes)
	 VISM(materials)
	 VISM(textures)
	 VISM(cube_textures)
	 VIS_(path)
	 VIS_(directory);
	if (v.IsLoading()) {
		for (Mesh& m : meshes)
			m.owner = this;
		for (Material& m : materials.GetValues())
			m.owner = this;
	}
}

bool Model::AddTextureFile(int mesh_i, TexType type, String path) {
	if (mesh_i < 0 || mesh_i >= meshes.GetCount())
		return false;
	return AddTextureFile(meshes[mesh_i], type, path);
}

bool Model::AddTextureFile(Mesh& mesh, TexType type, String path){
	if (FileExists(path)) {
        Image src = StreamRaster::LoadFileAny(path);
        return SetTexture(mesh, type, src, path);
    }
	return false;
}

bool Model::SetTexture(Mesh& mesh, TexType type, Image img, String path) {
	if (IsPositive(img.GetSize())) {
		if (mesh.material < 0) {
			Material& mat = AddMaterial();
			mesh.material = mat.id;
		}
		Material& mat = materials.Get(mesh.material);
		ASSERT(mat.id >= 0);
		int tex_id = GetAddTexture(img, path);
        mat.tex_id[type] = tex_id;
        mat.tex_filter[type] = GVar::FILTER_MIPMAP;
        return true;
    }
    return false;
}

bool Model::LoadCubemapFile(Mesh& mesh, TexType type, String path) {
	ASSERT(type >= TEXTYPE_CUBE_DIFFUSE);
	
	if (path.IsEmpty())
		return false;
	Index<String> ext_list; ext_list << "" << ".png" << ".jpg" << ".dds";
	Index<String> dir_list; dir_list << "" << "imgs" << "imgs/skybox";
	String abs_path;
	for(int j = 0; j < ext_list.GetCount(); j++) {
		for(int k = 0; k < dir_list.GetCount(); k++) {
			String s = RealizeShareFile(AppendFileName(dir_list[k], path + ext_list[j]));
			if (FileExists(s)) {
				abs_path = s;
				break;
			}
		}
		if (!abs_path.IsEmpty()) break;
	}

	if (!FileExists(abs_path)) {
		LOG("Model::LoadCubemapFile: error: file does not exist: " << abs_path);
		return false;
	}
	
	int cubetex_id = GetAddCubeTexture(abs_path);
	CubeTexture& cubetex = cube_textures.Get(cubetex_id);

	String dds_ext = ToLower(GetFileExt(abs_path));
	if (dds_ext == ".dds") {
		Vector<Image> faces;
		if (!LoadDdsImages(abs_path, faces) || faces.GetCount() != 6) {
			LOG("Model::LoadCubemapFile: error: failed to load DDS cubemap: " << abs_path);
			return false;
		}
		for (int i = 0; i < 6; i++) {
			cubetex.img[i].Set(faces[i]);
		}
	}
	else {
		// Load all 6 cube faces
		String ext = GetFileExt(abs_path);
		String base = abs_path.Left(abs_path.GetCount() - ext.GetCount());

		for(int i = 0; i < 6; i++) {
			String side_path = (i == 0) ? abs_path : (base + "_" + IntStr(i) + ext);
			if (!FileExists(side_path)) {
				LOG("Model::LoadCubemapFile: error: file does not exist: " << side_path);
				return false;
			}

			Image tmp_img = StreamRaster::LoadFileAny(side_path);
			if (tmp_img.IsEmpty()) {
				LOG("Model::LoadCubemapFile: error: failed to load cube side " << i);
				return false;
			}

			// Convert to ByteImage and swap R/B channels
			cubetex.img[i].Set(tmp_img);
			cubetex.img[i].SwapRedBlue();
		}
	}
	
	if (mesh.material < 0) {
		Material& mat = AddMaterial();
		mesh.material = mat.id;
	}
	Material& mat = materials.Get(mesh.material);
	ASSERT(mat.id >= 0);
    mat.tex_id[type] = cubetex_id;
    mat.tex_filter[type] = GVar::FILTER_MIPMAP;
    return true;
}

void Model::MakeModel(Shape2DWrapper& shape) {
	ASSERT(shape.shape);
	if (shape.shape) {
		MAKE_STATIC_LOCAL(Vector<tri3>, faces);
		shape.shape->GetFaces(faces);
		
	}
}

void Model::ReverseFaces() {
	for (Mesh& m : meshes)
		m.ReverseFaces();
}

Mesh& Model::AddMesh() {
	Mesh& m = meshes.Add();
	m.owner = this;
	return m;
}

Material& Model::AddMaterial() {
	int id = materials.IsEmpty() ? 0 : materials.Top().id + 1;
	Material& m = materials.Add(id);
	m.owner = this;
	m.id = id;
	return m;
}

Material& Model::RealizeMaterial(Mesh& mesh) {
	if (mesh.material < 0) {
		Material& mat = AddMaterial();
		mesh.material = mat.id;
		return mat;
	}
	else {
		Material& mat = materials.Get(mesh.material);
		return mat;
	}
}

ModelNode& Model::AddNode(String name, NodeIndex parent) {
	ModelNode& n = nodes.Add();
	n.name = name;
	n.index = nodes.GetCount()-1;
	n.parent_node_index = parent;
	return n;
}

Optional<NodeIndex> Model::FindFirstNode(String name, Optional<NodeIndex> const& parent_node_index) {
	// Children are guaranteed to come after their parents, so start looking after the parent index if one is provided.
    //const NodeIndex start_index = parent_node_index ? parent_node_index.value() + 1 : ModelNode::RootIdx;
    for (const ModelNode& node : nodes) {
        if ((!parent_node_index || node.parent_node_index == parent_node_index.value()) &&
            node.name == name) {
            return node.index;
        }
    }

    return {};
}

mat4 Model::GetNodeWorldTransform(NodeIndex node_idx) const {
    const ModelNode& node = GetNode(node_idx);
	
    // Compute the transform recursively.
    const mat4 parent_transform =
		node.index == ModelNode::RootIdx ?
			Identity<mat4>() :
			GetNodeWorldTransform(node.parent_node_index);
	
    return MultiplyMatrix(node.GetTransform(), parent_transform);
}

const ModelNode& Model::GetNode(NodeIndex node_idx) const {
	return nodes[node_idx];
}

void Model::Dump() {
	for(int i = 0; i < meshes.GetCount(); i++) {
		LOG("Mesh " << i << ":");
		meshes[i].Dump(1);
	}
	LOG("Textures:");
	for(int i = 0; i < textures.GetCount(); i++) {
		Size sz = textures[i].img.GetSize();
		LOG("\t" << i << ": " << textures[i].path << ": " << (int)sz.cx << "x" << (int)sz.cy);
	}
}

int Model::AddTexture(const Image& img, String path) {
	int id = textures.IsEmpty() ? 0 : textures.GetKey(textures.GetCount()-1) + 1;
	Texture& t = textures.Add(id);
	t.img.Set(img);
	t.path = path;
	return id;
}

int Model::AddTextureByteImage(const ByteImage& img, String path) {
	int id = textures.IsEmpty() ? 0 : textures.GetKey(textures.GetCount()-1) + 1;
	Texture& t = textures.Add(id);
	t.img = img;
	t.path = path;
	return id;
}

int Model::GetAddTexture(const Image& img, String path) {
	int i = FindTexture(path);
	if (i >= 0)
		return i;
	if (img.IsEmpty())
		return -1;
	return AddTexture(img, path);
}

int Model::AddCubeTexture(String path) {
	int id = cube_textures.IsEmpty() ? 0 : cube_textures.GetKey(cube_textures.GetCount()-1) + 1;
	CubeTexture& t = cube_textures.Add(id);
	t.path = path;
	return id;
}

int Model::GetAddCubeTexture(String path) {
	int i = FindTexture(path);
	if (i >= 0)
		return i;
	return AddCubeTexture(path);
}

int Model::FindTexture(String path) {
	for(int i = 0; i < textures.GetCount(); i++) {
		Texture& tex = textures[i];
		if (tex.path == path)
			return textures.GetKey(i);
	}
	return -1;
}










ModelLoader::ModelLoader() {
	
}

void ModelLoader::Clear() {
	model.Clear();
}

void ModelLoader::Set(const Model& m) {
	model = new Model(m);
}

void ModelLoader::Attach(Model* m) {
	Clear();
	model = m;
}

void ModelLoader::operator=(const Model & m) {
	Set(m);
}

ModelLoader::operator bool() const {
	return !model.IsEmpty();
}

void ModelLoader::Visit(Vis& vis) {
	bool has_model = !model.IsEmpty();
	vis("has_model", has_model);
	if (vis.IsLoading()) {
		if (has_model && model.IsEmpty())
			model.Create();
	}
	if (model)
		vis("model", *model, VISIT_NODE);
}

Model& ModelLoader::Create() {
	model.Create();
	return *model;
}

Model& ModelLoader::Realize() {
	if (!model.IsEmpty())
		return *model;
	else
		return Create();
}

Ptr<Model> ModelLoader::GetModel() {
	return model ? &*model : 0;
}


bool ModelLoader::LoadModel(String path) {
	model.Clear();
	#ifdef flagASSIMP
	return LoadModelAssimp(path);
	#endif
	return false;
}

#ifdef flagASSIMP
bool ModelLoader::LoadModelAssimp(String path) {
	LOG("ModelLoader::LoadModelAssimp: " << path);
    Assimp::Importer import_;
    const aiScene *scene = import_.ReadFile(path.Begin(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);
	
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        LOG("ERROR: assimp load failed: " << import_.GetErrorString());
        return false;
    }
	
	model = new Model();
    model->path = path;
    model->directory = GetFileDirectory(path);
	
    ProcessMaterials(*model, scene);
    ProcessNode(*model, scene->mRootNode, scene);
    
    return true;
}

void ModelLoader::ProcessMaterials(Model& model, const aiScene *scene) {
	model.materials.Clear();
	for(int i = 0; i < (int)scene->mNumMaterials; i++) {
		const aiMaterial* m = scene->mMaterials[i];
		ASSERT(m);
		Upp::Material& mat = model.materials.Add(i);
		mat.id = i;
		ProcessMaterial(model, mat, m);
	}
	
}

void ModelLoader::ProcessMaterial(Model& model, Upp::Material& m, const aiMaterial *mat) {
	for (int type = 0; type <= aiTextureType_UNKNOWN; type++) {
		int c = mat->GetTextureCount((aiTextureType) type);
		if (c > 1) {
			TODO
		}
		
		for(int i = 0; i < c; i++) {
			TexType textype = TEXTYPE_COUNT;
			switch (type) {
				#define TYPE(x) case aiTextureType_##x: textype = TEXTYPE_##x; break;
				TYPE(NONE)
				TYPE(DIFFUSE)
				TYPE(SPECULAR)
				TYPE(AMBIENT)
				TYPE(EMISSIVE)
				TYPE(HEIGHT)
				TYPE(NORMALS)
				TYPE(SHININESS)
				TYPE(OPACITY)
				TYPE(DISPLACEMENT)
				TYPE(LIGHTMAP)
				TYPE(REFLECTION)
				TYPE(UNKNOWN)
				//TYPE(CUBE_DIFFUSE)
				//TYPE(CUBE_IRRADIANCE)
				#undef TYPE
			}
			if (textype == TEXTYPE_COUNT) continue;
			
	        if (m.tex_id[textype] >= 0) {
	            LOG("warning: ModelLoader: multiple textures per mesh: " << model.path);
	            break;
	        }
	        aiString str;
	        str.Clear();
	        mat->GetTexture((aiTextureType) type, i, &str);
	        
	        String path = AppendFileName(model.directory, str.C_Str());
	        ByteImage img;
	        String ext = ToLower(GetFileExt(path));

	        if (ext == ".tga") {
	            img = TgaReaderBackend::LoadByteImageAny(path);
	        }
	        else if (ext == ".dds") {
	        	Image tmp_img;
	        	if (LoadDdsImage(path, tmp_img)) {
	        		img.Set(tmp_img);
	        	}
	        }
	        else {
	            // Use StreamRaster for PNG/JPG/etc
	            Image tmp_img = StreamRaster::LoadFileAny(path);
	            if (!tmp_img.IsEmpty()) {
	                img.Set(tmp_img);
	            }
	        }

	        if (!img.IsEmpty()) {
	            int tex_id = model.AddTextureByteImage(img, path);
	            m.tex_id[textype] = tex_id;
	            m.tex_filter[textype] = GVar::FILTER_MIPMAP;
	        }
	    }
	}
}

void ModelLoader::ProcessNode(Model& model, aiNode *node, const aiScene *scene) {
	// process all the node's meshes (if any)
    for(unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        ProcessMesh(model, model.meshes.Add(), mesh, scene);
    }
    // then do the same for each of its children
    for(unsigned int i = 0; i < node->mNumChildren; i++) {
        ProcessNode(model, node->mChildren[i], scene);
    }
}

void ModelLoader::ProcessMesh(Model& model, Mesh& out, aiMesh *mesh, const aiScene *scene) {
	out.vertices.SetCount(mesh->mNumVertices);
	
    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex& vertex = out.vertices[i];
        
        // process vertex positions, normals and texture coordinates
        vertex.position[0] = mesh->mVertices[i][0];
		vertex.position[1] = mesh->mVertices[i][1];
		vertex.position[2] = mesh->mVertices[i][2];
		vertex.position[3] = 1.0f;
		
		vertex.normal[0] = mesh->mNormals[i][0];
		vertex.normal[1] = mesh->mNormals[i][1];
		vertex.normal[2] = mesh->mNormals[i][2];
		
		// does the mesh contain texture coordinates?
		
		if(mesh->mTextureCoords[0]) {
		    vertex.tex_coord[0] = mesh->mTextureCoords[0][i][0];
		    vertex.tex_coord[1] = mesh->mTextureCoords[0][i][1];
		}
		else
		    vertex.tex_coord = vec2(0.0f, 0.0f);
		
		ASSERT(vertex.tex_coord[0] >= 0.0f && vertex.tex_coord[0] <= 1.0f);
		ASSERT(vertex.tex_coord[1] >= 0.0f && vertex.tex_coord[1] <= 1.0f);
    }
    
    // process indices
    out.indices.SetCount(0);
    out.indices.Reserve(mesh->mNumFaces*3);
    for(unsigned int i = 0; i < mesh->mNumFaces; i++) {
	    aiFace face = mesh->mFaces[i];
	    for(unsigned int j = 0; j < face.mNumIndices; j++)
	        out.indices.Add(face.mIndices[j]);
	}
	
	// process material
    out.material = mesh->mMaterialIndex;

	out.UpdateBoundingBox();
	
}

#endif


void ModelLoader::operator=(ModelBuilder& mb) {
	model = mb.Detach();
}




END_UPP_NAMESPACE
