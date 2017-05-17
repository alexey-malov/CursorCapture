#pragma once

namespace freeimagecpp
{

class CFreeImageInit
{
public:
	CFreeImageInit();
	~CFreeImageInit();

	// Debugging facility, don't push code which calls this method.
	void StartListenErrors(const std::function<void(const std::string &)> &handler);
};

}
