#pragma once


template<typename T, typename Identifier, typename Checker=Identifier>
class Memo
{
public:
	virtual bool isDirty(const Checker &checker) const {
		return checker != identifier_;
	}
	virtual void updateIdentifier(const Checker &checker) {
		identifier_ = checker;
	}
	void reset(const T &t) { cache_ = t; }
	template<typename Create=std::function<T()>>
	T& update(Create create, const Checker &checker) {
		if(is_initial_ || isDirty(checker)) {
			updateIdentifier(checker);
			cache_ = create();
			is_initial_ = false;
		}
		return *this;
	}
	void reset() {
		is_initial_ = true;
	}
	operator T&() { return cache_; }
protected:
	T cache_;
	mutable Identifier identifier_;
	bool is_initial_=true;
};
