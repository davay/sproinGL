THINGS=(
    "CMakeCache.txt"
    "cmake_install.cmake"
    "Makefile"
    "CMakeFiles"
    "sproinGL"
)

for thing in "${THINGS[@]}" ; 
do find . -name "$thing" -exec rm -rf {} + ;
done
