#include "ResourceStorage.h"

using namespace maaaaap;

std::shared_ptr<Source> ResourceStorage::getSourceFor(std::shared_ptr<WarpingMesh> w)
{
	return  getRelativeSingle<WarpingMesh, Source>(w);
}
std::vector<std::shared_ptr<WarpingMesh>> ResourceStorage::getWarpsUsing(std::shared_ptr<Source> s)
{
	return getRelativeMulti<WarpingMesh, Source>(s);
}
std::vector<std::shared_ptr<WarpingMesh>> ResourceStorage::getWarpsBoundTo(std::shared_ptr<RenderTexture> r)
{
	return getRelativeMulti<WarpingMesh, RenderTexture>(r);
}
std::shared_ptr<RenderTexture> ResourceStorage::getRenderTextureIncluding(std::shared_ptr<WarpingMesh> w)
{
	return getRelativeSingle<WarpingMesh, RenderTexture>(w);
}
std::shared_ptr<RenderTexture> ResourceStorage::getRenderTextureReferencedBy(std::shared_ptr<BlendingMesh> b)
{
	return getRelativeSingle<BlendingMesh, RenderTexture>(b);
}
std::vector<std::shared_ptr<BlendingMesh>> ResourceStorage::getBlendsReferencing(std::shared_ptr<RenderTexture> r)
{
	return getRelativeMulti<BlendingMesh, RenderTexture>(r);
}
std::vector<std::shared_ptr<BlendingMesh>> ResourceStorage::getBlendsBoundTo(std::shared_ptr<Output> o)
{
	return getRelativeMulti<BlendingMesh, Output>(o);
}
std::shared_ptr<Output> ResourceStorage::getOutputFor(std::shared_ptr<BlendingMesh> b)
{
	return getRelativeSingle<BlendingMesh, Output>(b);
}

