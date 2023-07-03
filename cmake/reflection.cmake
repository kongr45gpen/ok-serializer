# ---- Find if reflection is supported ----

include(CheckCXXSourceCompiles)

check_cxx_source_compiles(
        "#include <experimental/reflect>
using R = reflexpr(int);
int main() {}
" HAVE_REFLECTION
)