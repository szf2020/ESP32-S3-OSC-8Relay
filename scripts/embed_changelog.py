"""
Pre-build script: embed CHANGELOG.md into include/changelog_data.h
Runs automatically before each PlatformIO build via extra_scripts.
"""
import os

Import("env")  # noqa: F821 — PlatformIO injects this

PROJECT_DIR = env.subst("$PROJECT_DIR")
SRC = os.path.join(PROJECT_DIR, "CHANGELOG.md")
DST = os.path.join(PROJECT_DIR, "include", "changelog_data.h")


def embed_changelog(*args, **kwargs):
    if not os.path.isfile(SRC):
        print("[embed_changelog] CHANGELOG.md not found – skipping")
        return

    with open(SRC, "r", encoding="utf-8") as f:
        content = f.read()

    # Escape backslashes, double-quotes and newlines for a C string literal
    escaped = (
        content
        .replace("\\", "\\\\")
        .replace('"', '\\"')
        .replace("\r\n", "\\n")
        .replace("\n", "\\n")
    )

    header = (
        "// AUTO-GENERATED — do not edit manually.\n"
        "// Source: CHANGELOG.md  (regenerated at every build)\n"
        "#pragma once\n"
        "#include <pgmspace.h>\n"
        "\n"
        'static const char CHANGELOG_DATA[] PROGMEM = "' + escaped + '";\n'
    )

    # Only write when content changed (avoids unnecessary rebuilds)
    existing = None
    if os.path.isfile(DST):
        with open(DST, "r", encoding="utf-8") as f:
            existing = f.read()

    if header != existing:
        with open(DST, "w", encoding="utf-8") as f:
            f.write(header)
        print("[embed_changelog] include/changelog_data.h updated")
    else:
        print("[embed_changelog] include/changelog_data.h unchanged")


env.AddPreAction("buildprog", embed_changelog)
embed_changelog()  # also run immediately so the header exists before compilation
