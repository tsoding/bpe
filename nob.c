#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#define NOB_EXPERIMENTAL_DELETE_OLD
#include "./thirdparty/nob.h"

#define BUILD_FOLDER      "build/"
#define SRC_FOLDER        "src/"
#define THIRDPARTY_FOLDER "thirdparty/"

void build_tool(Cmd *cmd, Procs *procs, const char *bin_path, const char *src_path)
{
    cmd_append(cmd, "cc", "-Wall", "-Wextra", "-ggdb", "-I"THIRDPARTY_FOLDER, "-o", bin_path, src_path, SRC_FOLDER"bpe.c");
    da_append(procs, cmd_run_async_and_reset(cmd));
}

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);
    Cmd cmd = {0};
    Procs procs = {0};
    if (!nob_mkdir_if_not_exists(BUILD_FOLDER)) return 1;

    build_tool(&cmd, &procs, BUILD_FOLDER"txt2bpe", SRC_FOLDER"txt2bpe.c");
    build_tool(&cmd, &procs, BUILD_FOLDER"bpe2dot", SRC_FOLDER"bpe2dot.c");
    build_tool(&cmd, &procs, BUILD_FOLDER"bpe_inspect", SRC_FOLDER"bpe_inspect.c");
    if (!procs_wait_and_reset(&procs)) return 1;

    return 0;
}
