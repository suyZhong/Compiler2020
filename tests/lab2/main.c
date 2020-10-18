#include "syntax_tree.h"
extern syntax_tree *parse(const char*);

int main(int argc, char *argv[])
{
    syntax_tree *tree;
    // Call the syntax analyzer.
    if (argc != 2) {
        printf("usage: %s <cminus_file>\n", argv[0]);
    } else {
        tree = parse(argv[1]);
        print_syntax_tree(stdout, tree);
        del_syntax_tree(tree);
    }
    return 0;
}
