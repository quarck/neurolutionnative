#pragma once

namespace Neurolution
{

	class RuntimeConfig
	{
		int numWorkerThreads{ 4 }; // would be more more flexible in the future.. 
	public:

		int GetNumWorkerThreads() const { return numWorkerThreads; }
	};

}