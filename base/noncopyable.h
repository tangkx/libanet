#ifndef LIBANET_BASE_NONCOPYABLE_H
#define LIBANET_BASE_NONCOPYABLE_H

namespace libanet {

class noncopyable {
public:
	noncopyable(const noncopyable &) = delete;
	void operator=(const noncopyable &) = delete;

protected:
	noncopyable() = default;
	~noncopyable() = default;
};

} // namespace libanet

#endif