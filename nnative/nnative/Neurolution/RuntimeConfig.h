#pragma once

namespace Neurolution
{

    class RuntimeConfig
    {
        int numWorkerThreads{ 4 }; // would be more more flexible in the future.. 
    public:

        RuntimeConfig()
        {
            SYSTEM_INFO sysinfo;
            ::GetSystemInfo(&sysinfo);
            numWorkerThreads = sysinfo.dwNumberOfProcessors;
        }

        int GetNumWorkerThreads() const 
        {
            return numWorkerThreads; 
        }
    };

}