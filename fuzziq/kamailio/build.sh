#!/bin/bash
set -euxo pipefail

cd "$SRC/kamailio"

export CC_OPT="$CFLAGS"
export LD_EXTRA_OPTS="$CFLAGS"
export MEMPKG=sys

# Make reruns in the same checkout safe after a previous fuzz build.
sed -i '0,/int main2(/s//int main(/' ./src/main.c

# Build Kamailio normally first so compilation errors are not hidden by `|| true`.
make Q=verbose

# libFuzzer supplies main(). Rename Kamailio's entry point and rebuild only
# main.o before putting the core objects into the fuzzing support archive.
sed -i '0,/int main(/s//int main2(/' ./src/main.c
rm -f ./src/main.o
make -C src Q=verbose main.o

# Kamailio renamed these helpers to the *_str variants.
sed -i \
    -e 's/cmp_uri(/cmp_uri_str(/g' \
    -e 's/cmp_uri_light(/cmp_uri_light_str(/g' \
    -e 's/cmp_aor(/cmp_aor_str(/g' \
    misc/fuzz/fuzz_strutils.c

cd src
rm -rf objects
mkdir -p objects

# Keep every object. Flattening by basename loses objects such as srdb1/db.o
# and srdb2/db.o because one overwrites the other.
while IFS= read -r -d '' object; do
    archive_name="${object#./}"
    archive_name="${archive_name//\//__}"
    cp -- "$object" "objects/$archive_name"
done < <(find . -path ./objects -prune -o -type f -name '*.o' -print0)

"${AR:-ar}" rcs libkamailio.a objects/*.o
cd ..

for fuzzer in misc/fuzz/fuzz_*.c; do
    fuzzer_name="$(basename "$fuzzer" .c)"
    fuzzer_object="$fuzzer_name.o"

    # Use the same CPU family as the x86_64 Kamailio build. Do not pass
    # archives or linker-only flags to the compile-only invocation.
    "$CC" $CFLAGS \
        -DFAST_LOCK -D__CPU_x86_64 \
        -I./src -I./src/core/parser \
        -c "$fuzzer" -o "$fuzzer_object"

    "$CXX" $CXXFLAGS $LIB_FUZZING_ENGINE \
        "$fuzzer_object" ./src/libkamailio.a \
        -ldl -lresolv -lm \
        -o "$OUT/$fuzzer_name"
done
