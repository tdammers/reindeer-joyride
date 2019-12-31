#!/usr/bin/env python3

import subprocess
import os.path

with open("src/asset_ids.h", "w") as header:
    with open("src/asset_ids.c", "w") as source:
        header.write("#pragma once\n\n")
        source.write('#include "asset_ids.h"\n\n')

        def process_subdir(path, typrefix, extensions, subs):
            source.write('const char* {0}_asset_filenames[] = {1}\n' \
                .format(typrefix.lower(), '{'))

            i = 0
            for sub in subs:
                subpath, subprefix = sub
                completed = subprocess.run(
                    ["find",
                        os.path.join(path, subpath),
                        "-type", "f",
                        "-exec", "echo", "{}", ";"],
                    capture_output=True)

                asset_filenames = completed.stdout.split(b'\n')
                asset_filenames.sort()

                for asset_filename in asset_filenames:
                    if not asset_filename:
                        continue
                    matches_extension = False
                    for extension in extensions:
                        if asset_filename.endswith(extension.encode('ascii')):
                            matches_extension = True
                            break;
                    if not matches_extension:
                        continue
                    tile_name = \
                        os \
                            .path \
                            .basename(asset_filename) \
                            .upper() \
                            .decode('ascii') \
                            .split(".")[0] \
                            .replace('-', '_')
                    header.write("#define {0}_ASSET_{1}{2} {3}\n" \
                        .format(
                            typrefix,
                            subprefix + "_" if subprefix else "",
                            tile_name,
                            i))

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

        process_subdir("data/img", "IMG", [".png"], \
            [
                ("tiles", "TILE"),
                ("sprites", "SPRITE"),
                ("backgrounds", "BACKGROUND"),
                ("ui", "UI")
            ])

        process_subdir("data/fonts", "FONT", [".ttf", ".otf"], \
            [
                (".", ""),
            ])
