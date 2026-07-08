#!/usr/bin/env python
"""
Update the help file translation templates in resources/help/translations
to reflect any recent changes in the source code.  Manually revert any
files with no meaningful content changes, and undo any regressions to the
file's title, author, etc. in the metadata.

Needs to be run in a Python environment in which sphinx and
sphinx_rtd_theme have been installed.  Run from the PortaBase source code's
root directory as "portabase/update_help_translation_templates.py".
"""
import re

from os import scandir
from pathlib import Path
from subprocess import run

HELP_ROOT = str(Path(__file__).parent.parent / "resources" / "help")
POT_DIR = str(Path(HELP_ROOT) / "translations" / "templates")
ID_VERSION_REGEX = re.compile(r"Project-Id-Version: PortaBase [0-9\.]+")
UUID_COMMENT_REGEX = re.compile(r"#:? [0-9a-f]{32}")

print("Running sphinx-build...")
run(["sphinx-build", "-b", "gettext", HELP_ROOT, POT_DIR], check=True)
print("Fixing metadata...")
with scandir(POT_DIR) as it:
    for entry in it:
        pot = Path(entry.path)
        if pot.suffix != ".pot":
            continue
        stem = pot.stem
        print(f"Fixing {stem}")
        content = pot.read_text(encoding="utf8")
        content = content.replace("SOME DESCRIPTIVE TITLE.",
                                  f"Messages from {stem}.txt", 1)
        content = content.replace("Report-Msgid-Bugs-To: \\n",
                                  "Report-Msgid-Bugs-To: https://github.com/jmbowman/portabase/issues\\n",
                                  1)
        content = re.sub(ID_VERSION_REGEX, "Project-Id-Version: PortaBase",
                         content, count=1)
        content = content.replace("LL@li.org", "portabase-i18n@lists.sourceforge.net", 1)
        pot.write_text(content)
        has_real_changes = False
        diff = run(["git", "diff", entry.path], capture_output=True).stdout.decode("utf8")
        for line in diff.split("\n"):
            if not line:
                continue
            if line[0] not in ("+", "-"):
                continue
            if line.startswith("+++") or line.startswith("---"):
                continue
            if "# Copyright (C)" in line:
                continue
            if "POT-Creation-Date:" in line:
                continue
            if not line[1:].strip():
                # adding or removing blank lines doesn't count
                continue
            if line[1:].startswith("#: ../../"):
                # don't make a change just for line number changes
                continue
            if re.match(UUID_COMMENT_REGEX, line[1:]):
                # don't make a change just for UUID value churn
                continue
            has_real_changes = True
            break
        if not has_real_changes:
            run(["git", "checkout", "--", entry.path])
print("Done")
