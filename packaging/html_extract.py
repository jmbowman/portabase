# /// script
# requires-python = ">=3.11"
# dependencies = ["selectolax"]
# ///
"""Extract and normalize an HTML content region before Markdown conversion.

Companion to `html-to-markdown` -- see docs/wiki/converting-html-to-markdown.md
for the full porting recipe and why each step is needed.
"""

import argparse
import re
import sys

from selectolax.parser import HTMLParser

HEADING_TAG_RE = re.compile(r"(</?)h([1-6])\b", re.IGNORECASE)
BOLD_PARAGRAPH_RE = re.compile(
    r"<p>\s*<(b|strong)>(?P<text>.*?)</(?:b|strong)>\s*</p>",
    re.IGNORECASE | re.DOTALL,
)


def shift_headings(html: str, by: int) -> str:
    """Rewrite every h1-h6 tag by `by` levels, clamped to the h1-h6 range.

    Operates on tag names rather than the parsed tree because selectolax nodes
    expose `tag` read-only.

    :param html: HTML fragment to rewrite
    :param by: levels to shift, negative to promote
    :return: the rewritten fragment
    """
    if not by:
        return html
    return HEADING_TAG_RE.sub(
        lambda m: f"{m.group(1)}h{min(6, max(1, int(m.group(2)) + by))}",
        html,
    )


def promote_bold_paragraphs(html: str, level: int) -> str:
    """Convert paragraphs whose entire content is bold into headings.

    Legacy pages often mark up a subheading as `<p><b>Title</b></p>`, which
    converts to emphasis rather than a heading and trips MD036.

    :param html: HTML fragment to rewrite
    :param level: heading level to emit
    :return: the rewritten fragment
    """
    return BOLD_PARAGRAPH_RE.sub(
        lambda m: f"<h{level}>{m.group('text').strip()}</h{level}>",
        html,
    )


def main() -> int:
    """Parse arguments, apply the requested rewrites, and print the result.

    :return: process exit status
    """
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("file", help="input HTML file")
    parser.add_argument(
        "-s", "--select", default="body", help="CSS selector for the content region"
    )
    parser.add_argument(
        "--shift-headings",
        type=int,
        default=0,
        metavar="N",
        help="shift heading levels by N (use -1 when the site banner owns the h1)",
    )
    parser.add_argument(
        "--promote-bold-paragraphs",
        type=int,
        default=0,
        metavar="LEVEL",
        help="convert <p><b>Text</b></p> into a heading at LEVEL",
    )
    args = parser.parse_args()

    with open(args.file, encoding="utf-8") as handle:
        tree = HTMLParser(handle.read())
    node = tree.css_first(args.select)
    if node is None:
        print(f"error: no element matched {args.select!r}", file=sys.stderr)
        return 1

    html = node.html or ""
    if args.promote_bold_paragraphs:
        html = promote_bold_paragraphs(html, args.promote_bold_paragraphs)
    html = shift_headings(html, args.shift_headings)
    print(html, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
