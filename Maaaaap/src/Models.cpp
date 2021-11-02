#include "Models.h"

using namespace maaaaap;

ofMesh WarpingMesh::getMesh() const
{
	ofMesh ret = mesh_->getMesh();
	for(auto &coord : ret.getTexCoords()) {
		geom::remapPosition({0,0,1,1}, texcoord_range_, coord);
	}
	return ret;
}

ofMesh BlendingMesh::getMesh() const
{
	return ofxBlendScreen::createMesh(vertex_outer_, vertex_inner_, vertex_frame_, texture_uv_for_frame_);
}
