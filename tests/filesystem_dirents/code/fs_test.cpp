#include <chrono>
#include <cstring>
#include <filesystem>
#include <sstream>
#include <string>

#include "orbis/UserService.h"

#include "fs_test.h"

namespace fs = std::filesystem;

namespace FS_Test
{
    namespace oi = OrbisInternals;

    void Drop(char *buffer, size_t size)
    {
        for (u16 line = 0; line < (size / 32); line++)
        {
            std::stringstream out;
            for (u16 col = 0; col < 32; col++)
            {
                out << std::setw(2) << std::hex << (0xFF & static_cast<unsigned int>(buffer[col + (line * 32)])) << " ";
            }
            Log(out.str());
        }
    }

    void RunTests()
    {
        RegenerateDir("/data/therapist");
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname01", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname02", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname03", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname04", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname05", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname06", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname07", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname08", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname09", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname10", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname11", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname12", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname13", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname14", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname15", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname16", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname17", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname18", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname19", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname10", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname21", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname22", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname23", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname24", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname25", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname26", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname27", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname28", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname29", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname30", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname31", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname32", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname33", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname34", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname35", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname36", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname37", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname38", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname39", O_CREAT | O_WRONLY | O_TRUNC, 0777));
        sceKernelClose(sceKernelOpen("/data/therapist/filewithaverylongname40", O_CREAT | O_WRONLY | O_TRUNC, 0777));

        int status{};
        int counter{};
        s64 idx{};
        constexpr int buffer_size = 1024; // 512 + 256;
        char buffer[buffer_size]{0};

#define FUCKING_LSEEK_OFFSET 1024

        // Log("---------------------");
        // Log("Dump normal directory");
        // Log("---------------------");
        // int fd = sceKernelOpen("/data/therapist", O_DIRECTORY | O_RDONLY, 0777);

        // Log("Directory opened with fd=", fd);

        // Log("LSeek END+0=", sceKernelLseek(fd, 0, 2));
        // Log("LSeek END+100=", sceKernelLseek(fd, 100, 2));
        // Log("LSeek END-100000=", sceKernelLseek(fd, -100000, 2));

        // if (int _tmp = sceKernelLseek(fd, FUCKING_LSEEK_OFFSET, 0); _tmp < 0)
        //     LogError("Lseek failed:", _tmp);
        // counter = 3;
        // idx = 0;
        // do
        // {
        //     memset(buffer, 0xAA, buffer_size);
        //     status = sceKernelRead(fd, buffer, buffer_size);
        //     Log("Read got", status, "bytes, idx=", idx, ",ptr=", sceKernelLseek(fd, 0, 1));
        //     if (status <= 0)
        //         break;
        //     Drop(buffer, buffer_size);
        // } while (status > 0 && counter-- > 0);

        // if (int _tmp = sceKernelLseek(fd, FUCKING_LSEEK_OFFSET, 0); _tmp < 0)
        //     LogError("Lseek failed:", _tmp);
        // counter = 3;
        // idx = 0;
        // do
        // {
        //     memset(buffer, 0xAA, buffer_size);
        //     status = sceKernelGetdirentries(fd, buffer, buffer_size, &idx);
        //     Log("GetDirEntries got", status, "bytes, idx=", idx, ",ptr=", sceKernelLseek(fd, 0, 1));
        //     if (status <= 0)
        //         break;
        //     Drop(buffer, buffer_size);
        // } while (status > 0 && counter-- > 0);

        // sceKernelClose(fd);

        Log("------------------");
        Log("Dump PFS directory");
        Log("------------------");
        int fd = sceKernelOpen("/app0/assets/misc", O_DIRECTORY | O_RDONLY, 0777);

        Log("Directory opened with fd=", fd);

        Log("LSeek END+0=", sceKernelLseek(fd, 0, 2));
        Log("LSeek END+100=", sceKernelLseek(fd, 100, 2));
        Log("LSeek END-100000=", sceKernelLseek(fd, -100000, 2));

        if (int _tmp = sceKernelLseek(fd, FUCKING_LSEEK_OFFSET, 0); _tmp < 0)
            LogError("Lseek failed:", _tmp);
        counter = 3;
        idx = 0;
        do
        {
            memset(buffer, 0xAA, buffer_size);
            status = sceKernelRead(fd, buffer, buffer_size);
            Log("Read got", status, "bytes, idx=", idx, ",ptr=", sceKernelLseek(fd, 0, 1));
            if (status <= 0)
                break;
            Drop(buffer, buffer_size);
        } while (status > 0 && counter-- > 0);

        if (int _tmp = sceKernelLseek(fd, FUCKING_LSEEK_OFFSET, 0); _tmp < 0)
            LogError("Lseek failed:", _tmp);
        counter = 3;
        idx = 0;
        do
        {
            memset(buffer, 0xAA, buffer_size);
            status = sceKernelGetdirentries(fd, buffer, buffer_size, &idx);
            Log("GetDirEntries got", status, "bytes, idx=", idx, ",ptr=", sceKernelLseek(fd, 0, 1));
            if (status <= 0)
                break;
            Drop(buffer, buffer_size);
        } while (status > 0 && counter-- > 0);

        sceKernelClose(fd);
    }

    bool RegenerateDir(const char *path)
    {
        Obliterate(path);
        sceKernelMkdir(path, 0777);
        return true;
    }

}