#include <cstdint>
#include <link.h>
#include <sys/mman.h>
#include <vector>
#include <string>

#include "log.h"

/* Struct to hold a single entry in /proc/maps/ */
/* Format: 7ac49c2000(start)-7ac4a26000(end) r--p (permissions) 00000000(offset) 00:00 0 (dev) 1245 (inode) /apex/com.android.runtime/bin/linker64 (path) */
struct PROCMAPSINFO {
    uintptr_t start, end, offset;
    uint8_t perms;
    ino_t inode;
    char* dev;
    char* path;
};

std::vector<PROCMAPSINFO> get_modules_by_name(std::string mName, std::string pid)
{
    std::string process_maps_locations_via_proc_id = "/proc/" + pid + "/maps";
    std::string process_maps_locations = "/proc/self/maps";

    std::vector<PROCMAPSINFO> returnVal;

    char buffer[512];
    FILE *fp = fopen(process_maps_locations.c_str(), "re");

    if (fp != nullptr)
    {
        while (fgets(buffer, sizeof(buffer), fp))
        {
            if (strstr(buffer, mName.c_str()))
            {
                PROCMAPSINFO info{};
                char perms[10];
                char path[255];
                char dev[25];

                sscanf(buffer, "%lx-%lx %s %ld %s %ld %s", &info.start, &info.end, perms, &info.offset, dev, &info.inode, path);

                /* Store process permissions in the struct directly via bitwise operations */
                if (strchr(perms, 'r')) info.perms |= PROT_READ;
                if (strchr(perms, 'w')) info.perms |= PROT_WRITE;
                if (strchr(perms, 'x')) info.perms |= PROT_EXEC;
                if (strchr(perms, 'r')) info.perms |= PROT_READ;

                info.dev = dev;
                info.path = path;

                /* LOGI("Line at: %s", buffer); */
                returnVal.push_back(info);
            }
        }
    }
    return returnVal;
}

void remap_lib(std::string name, std::string pid)
{
    std::vector<PROCMAPSINFO> maps = get_modules_by_name(name, pid);

    for (PROCMAPSINFO info : maps)
    {
        void *address = (void *)info.start;
        size_t size = info.end - info.start;

        void *map = mmap(0, size, PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

        if ((info.perms & PROT_READ) == 0) {
            LOGI("Removing memory protection: %s", info.path);
            mprotect(address, size, PROT_READ);
        }

        if (map == nullptr) {
            LOGE("Failed to Allocate Memory: %s", strerror(errno));
            return;
        }

        /* Copy the in-memory data to new virtual location via the memove, allocate and commit it via mremap */
        std::memmove(map, address, size);
        mremap(map, size, size, MREMAP_MAYMOVE | MREMAP_FIXED, info.start);

        /* Re-apply memory protections */
        mprotect((void *)info.start, size, info.perms);

        LOGI("Allocated at address %p with size of %zu", map, size);
    }

        LOGI("Remapping completed!!");
}

bool remap_all(std::vector<std::string> libs, std::string pid)
{
    for (int i = 0; i < libs.size(); i++)
        remap_lib(libs[i], pid);
}

