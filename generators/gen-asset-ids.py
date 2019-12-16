#!/usr/bin/env python3

import subprocess
import os.path

with open("src/asset_ids.h", "w") as header:
    with open("src/asset_ids.c", "w") as source:
        header.write("#pragma once\n\n")
        source.write('#include "asset_ids.h"\n\n')

        def process_subdir(path, typrefix, subs):
            source.write('const char* {0}_asset_filenames[] = {1}\n' \
                .format(typrefix.lower(), '{'))

            i = 0
            for sub in subs:
                subpath, subprefix = sub
                completed = subprocess.run(
                    ["find", os.path.join(path, subpath), "-type", "f"],
                    capture_output=True)

                asset_filenames = completed.stdout.split()

                for asset_filename in asset_filenames:
                    tile_name = \
                        os \
                            .path \
                            .basename(asset_filename) \
                            .upper() \
                            .decode('ascii') \
                            .split(".")[0] \
                            .replace('-', '_')
                    header.write("#define {0}_ASSET_{1}_{2} {3}\n" \
                        .format(typrefix, subprefix, tile_name, i))

                    local_filename = \
                        os.path.relpath(asset_filename.decode('ascii'), "./data")
                    source.write('    (const char*)"{0}",\n' \
                        .format(local_filename))
                    i += 1

            header.write("#define NUM_{0}_ASSETS {1}\n\n" \
                .format(typrefix, i))
            header.write("extern const char *{0}_asset_filenames[];\n" \
                .format(typrefix.lower()))

            source.write("};\n")

        process_subdir("data/img", "IMG", \
            [
                ("tiles", "TILE"),
                ("sprites", "SPRITE")
            ])
