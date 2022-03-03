#include "BlendingUVEditor.h"

void BlendingUVEditor::movePoint(MeshType &mesh, IndexType index, const glm::vec2 &delta)
{
	auto p = mesh[index] += delta;
	mesh[index^2].x = p.x;
	mesh[index^1].y = p.y;
}
