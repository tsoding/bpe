#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#define NOB_EXPERIMENTAL_DELETE_OLD
#include "./thirdparty/nob.h"

#define BUILD_FOLDER      "build/"
#define SRC_FOLDER        "src/"
#define THIRDPARTY_FOLDER "thirdparty/"

bool build_tool(Cmd *cmd, const char *bin_path, const char *src_path)
{
    cmd_append(cmd, "cc", "-Wall", "-Wextra", "-ggdb", "-I"THIRDPARTY_FOLDER, "-o", bin_path, src_path);
    return cmd_run_sync_and_reset(cmd);
}

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);
    Cmd cmd = {0};
    if (!nob_mkdir_if_not_exists(BUILD_FOLDER)) return 1;
    if (!build_tool(&cmd, BUILD_FOLDER"txt2bpe", SRC_FOLDER"txt2bpe.c")) return 1;
    if (!build_tool(&cmd, BUILD_FOLDER"bpe2dot", SRC_FOLDER"bpe2dot.c")) return 1;

    return 0;
}
