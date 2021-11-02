#pragma once

#include "Models.h"
#include <map>

namespace maaaaap {
class ResourceStorage {
public:
	template<typename T, typename U> void bind(std::shared_ptr<T> t, std::shared_ptr<U> u);
	template<typename T, typename U> void unbind(std::shared_ptr<T> t, std::shared_ptr<U> u);
	template<typename T> void remove(std::shared_ptr<T> t);
	template<typename T> void add(std::shared_ptr<T> t);
	template<typename T> std::set<std::shared_ptr<T>>& getContainer();
	template<typename T> const std::set<std::shared_ptr<T>>& getContainer() const { return const_cast<ResourceStorage*>(this)->getContainer<T>(); }
	
	std::shared_ptr<Source> getSourceFor(std::shared_ptr<WarpingMesh> w);
	std::vector<std::shared_ptr<WarpingMesh>> getWarpsUsing(std::shared_ptr<Source> s);
	std::vector<std::shared_ptr<WarpingMesh>> getWarpsBoundTo(std::shared_ptr<RenderTexture> r);
	std::shared_ptr<RenderTexture> getRenderTextureIncluding(std::shared_ptr<WarpingMesh> w);
	std::shared_ptr<RenderTexture> getRenderTextureReferencedBy(std::shared_ptr<BlendingMesh> b);
	std::vector<std::shared_ptr<BlendingMesh>> getBlendsReferencing(std::shared_ptr<RenderTexture> r);
	std::vector<std::shared_ptr<BlendingMesh>> getBlendsBoundTo(std::shared_ptr<Output> o);
	std::shared_ptr<Output> getOutputFor(std::shared_ptr<BlendingMesh> b);
	
	const std::shared_ptr<Source> getSourceFor(std::shared_ptr<WarpingMesh> w) const { return const_cast<ResourceStorage*>(this)->getSourceFor(w); }
	const std::vector<std::shared_ptr<WarpingMesh>> getWarpsUsing(std::shared_ptr<Source> s) const { return const_cast<ResourceStorage*>(this)->getWarpsUsing(s); }
	const std::vector<std::shared_ptr<WarpingMesh>> getWarpsBoundTo(std::shared_ptr<RenderTexture> r) const { return const_cast<ResourceStorage*>(this)->getWarpsBoundTo(r); }
	const std::shared_ptr<RenderTexture> getRenderTextureIncluding(std::shared_ptr<WarpingMesh> w) const { return const_cast<ResourceStorage*>(this)->getRenderTextureIncluding(w); }
	const std::shared_ptr<RenderTexture> getRenderTextureReferencedBy(std::shared_ptr<BlendingMesh> b) const { return const_cast<ResourceStorage*>(this)->getRenderTextureReferencedBy(b); }
	const std::vector<std::shared_ptr<BlendingMesh>> getBlendsReferencing(std::shared_ptr<RenderTexture> r) const { return const_cast<ResourceStorage*>(this)->getBlendsReferencing(r); }
	const std::vector<std::shared_ptr<BlendingMesh>> getBlendsBoundTo(std::shared_ptr<Output> o) const { return const_cast<ResourceStorage*>(this)->getBlendsBoundTo(o); }
	const std::shared_ptr<Output> getOutputFor(std::shared_ptr<BlendingMesh> b) const { return const_cast<ResourceStorage*>(this)->getOutputFor(b); }
private:
	std::set<std::shared_ptr<Source>> s_;
	std::set<std::shared_ptr<WarpingMesh>> w_;
	std::set<std::shared_ptr<RenderTexture>> r_;
	std::set<std::shared_ptr<BlendingMesh>> b_;
	std::set<std::shared_ptr<Output>> o_;
	
	template<typename T, typename U>
	using weak_map = std::map<std::weak_ptr<T>, std::weak_ptr<U>, std::owner_less<std::weak_ptr<T>>>;
	weak_map<WarpingMesh, Source> ws_;
	weak_map<WarpingMesh, RenderTexture> wr_;
	weak_map<BlendingMesh, RenderTexture> br_;
	weak_map<BlendingMesh, Output> bo_;
	
	template<typename T, typename U> weak_map<T,U>& getMap();
	template<> weak_map<WarpingMesh, Source>& getMap<WarpingMesh, Source>() { return ws_; }
	template<> weak_map<WarpingMesh, RenderTexture>& getMap<WarpingMesh, RenderTexture>() { return wr_; }
	template<> weak_map<BlendingMesh, RenderTexture>& getMap<BlendingMesh, RenderTexture>() { return br_; }
	template<> weak_map<BlendingMesh, Output>& getMap<BlendingMesh, Output>() { return bo_; }
	
	template<typename T, typename U> const weak_map<T,U>& getMap() const { return const_cast<ResourceStorage*>(this)->getMap<T,U>(); }
	
	template<typename T, typename U> std::shared_ptr<U> getRelativeSingle(std::shared_ptr<T> t);
	template<typename T, typename U> std::vector<std::shared_ptr<T>> getRelativeMulti(std::shared_ptr<U> u);

	template<typename T, typename U> const std::shared_ptr<U> getRelativeSingle(std::shared_ptr<T> t) const { return const_cast<ResourceStorage*>(this)->getRelativeSingle<T,U>(t); }
	template<typename T, typename U> const std::vector<std::shared_ptr<T>> getRelativeMulti(std::shared_ptr<U> u) const { return const_cast<ResourceStorage*>(this)->getRelativeMulti<T,U>(u); }
};

template<typename T, typename U>
inline void ResourceStorage::bind(std::shared_ptr<T> t, std::shared_ptr<U> u)
{
	auto &m = getMap<T,U>();
	auto result = m.insert(std::make_pair(t, u));
	if(!result.second) {
		result.first->second = u;
	}
	getContainer<T>().insert(t);
	getContainer<U>().insert(u);
}
template<typename T, typename U>
inline void ResourceStorage::unbind(std::shared_ptr<T> t, std::shared_ptr<U> u)
{
	auto &m = getMap<T,U>();
	auto found = m.find(t);
	if(found != end(m) && found->second.lock() == u) {
		m.erase(found);
	}
}
template<typename T>
inline void ResourceStorage::add(std::shared_ptr<T> t)
{
	getContainer<T>().insert(t);
}
template<typename T>
inline void ResourceStorage::remove(std::shared_ptr<T> t)
{
	getContainer<T>().erase(t);
}
template<> inline std::set<std::shared_ptr<Source>>& ResourceStorage::getContainer() { return s_; }
template<> inline std::set<std::shared_ptr<WarpingMesh>>& ResourceStorage::getContainer() { return w_; }
template<> inline std::set<std::shared_ptr<RenderTexture>>& ResourceStorage::getContainer() { return r_; }
template<> inline std::set<std::shared_ptr<BlendingMesh>>& ResourceStorage::getContainer() { return b_; }
template<> inline std::set<std::shared_ptr<Output>>& ResourceStorage::getContainer() { return o_; }

template<typename T, typename U>
inline std::shared_ptr<U> ResourceStorage::getRelativeSingle(std::shared_ptr<T> t)
{
	auto &m = getMap<T,U>();
	auto found = m.find(t);
	return found != end(m) ? found->second.lock() : nullptr;
}
template<typename T, typename U>
inline std::vector<std::shared_ptr<T>> ResourceStorage::getRelativeMulti(std::shared_ptr<U> u)
{
	std::vector<std::shared_ptr<T>> ret;
	auto &m = getMap<T,U>();
	for(auto &&p : m) {
		if(p.second.lock() == u && !p.first.expired()) {
			ret.push_back(p.first.lock());
		}
	}
	return ret;
}
}
